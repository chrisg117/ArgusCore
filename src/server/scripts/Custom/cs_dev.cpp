/*
 * Local dev tooling (chore(local), not for upstream): console commands an agent uses over
 * SOAP to act as an online character, plus the in-game "dev trace" the ArgusDev addon calls.
 *   dev as <char> <command>                                          (console only)
 *   dev select <char> creature <spawnId> | nearest <entry> [range] | clear   (console only)
 *   dev mark <text>                                                  (console only)
 *   dev trace <kind> <text>                                          (in game only)
 */

#include "ScriptMgr.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "Creature.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RBAC.h"
#include "WorldSession.h"
#include "dev_trace.h"

using namespace Trinity::ChatCommands;

namespace
{
    // Runs commands as a player's session and passes every printed line on to another handler.
    class ForwardingHandler : public ChatHandler
    {
    public:
        ForwardingHandler(WorldSession* session, ChatHandler* out) : ChatHandler(session), _out(out) { }

        void SendSysMessage(std::string_view str, bool /*escapeCharacters*/) override
        {
            size_t start = 0;
            while (true)
            {
                size_t end = str.find('\n', start);
                _out->SendSysMessage(str.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start));
                ++lines;
                if (end == std::string_view::npos)
                    break;
                start = end + 1;
            }
        }

        int lines = 0;

    private:
        ChatHandler* _out;
    };

    bool Fail(ChatHandler* handler, std::string const& message)
    {
        handler->SendSysMessage(message);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // Console::Yes only allows the console; these must also refuse in-game callers.
    bool ConsoleOnly(ChatHandler* handler, char const* command)
    {
        if (!handler->GetSession())
            return true;
        return Fail(handler, std::string(command) + " is console only");
    }

    Player* FindOnline(ChatHandler* handler, std::string name)
    {
        if (normalizePlayerName(name))
            if (Player* player = ObjectAccessor::FindPlayerByName(name))
                return player;
        Fail(handler, name + " is not online");
        return nullptr;
    }

    std::string_view TrimLeft(std::string_view text)
    {
        while (!text.empty() && text.front() == ' ')
            text.remove_prefix(1);
        return text;
    }
}

class dev_commandscript : public CommandScript
{
public:
    dev_commandscript() : CommandScript("dev_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable devCommandTable =
        {
            { "as",     HandleDevAsCommand,     rbac::RBAC_PERM_COMMAND_DEBUG, Console::Yes },
            { "select", HandleDevSelectCommand, rbac::RBAC_PERM_COMMAND_DEBUG, Console::Yes },
            { "mark",   HandleDevMarkCommand,   rbac::RBAC_PERM_COMMAND_DEBUG, Console::Yes },
            { "trace",  HandleDevTraceCommand,  rbac::RBAC_PERM_COMMAND_DEBUG, Console::No },
        };
        static ChatCommandTable commandTable =
        {
            { "dev", devCommandTable },
        };
        return commandTable;
    }

    static bool HandleDevAsCommand(ChatHandler* handler, std::string name, Tail command)
    {
        if (!ConsoleOnly(handler, "dev as"))
            return false;

        std::string_view cmd = TrimLeft(command);
        if (cmd.substr(0, 3) == "dev" && (cmd.size() == 3 || cmd[3] == ' '))
            return Fail(handler, "nested dev commands are refused");

        Player* player = FindOnline(handler, name);
        if (!player)
            return false;

        ForwardingHandler forward(player->GetSession(), handler);
        bool parsed = forward._ParseCommands(cmd);
        bool ok = parsed && !forward.HasSentErrorMessage();
        handler->SendSysMessage(ok ? "-> ok" : "-> failed");
        DevTrace::Write("agent", player->GetName(), "cmd",
            { DevTrace::Str("command", cmd), DevTrace::Bool("ok", ok), DevTrace::Num("lines", forward.lines) });
        if (!ok)
            handler->SetSentErrorMessage(true);
        return ok;
    }

    static bool HandleDevSelectCommand(ChatHandler* handler, std::string name, std::string mode, Optional<uint64> arg, Optional<float> range)
    {
        if (!ConsoleOnly(handler, "dev select"))
            return false;

        Player* player = FindOnline(handler, name);
        if (!player)
            return false;

        if (mode == "clear")
        {
            player->SetSelection(ObjectGuid::Empty);
            handler->SendSysMessage(player->GetName() + " -> nothing");
            return true;
        }

        Creature* target = nullptr;
        if (mode == "creature" && arg)
            target = player->GetMap()->GetCreatureBySpawnId(*arg);
        else if (mode == "nearest" && arg)
            target = player->FindNearestCreature(uint32(*arg), range.value_or(50.0f), true);
        else
            return Fail(handler, "usage: dev select <char> creature <spawnId> | nearest <entry> [range] | clear");

        if (!target)
            return Fail(handler, "not found");

        player->SetSelection(target->GetGUID());
        handler->SendSysMessage(player->GetName() + " -> " + target->GetName() + " " + target->GetGUID().ToString());
        return true;
    }

    static bool HandleDevMarkCommand(ChatHandler* handler, Tail text)
    {
        if (!ConsoleOnly(handler, "dev mark"))
            return false;

        DevTrace::Write("agent", "", "mark", { DevTrace::Str("text", text) });
        handler->SendSysMessage("marked");
        return true;
    }

    // Called by the ArgusDev addon over the addon channel; replies nothing so it stays quiet.
    static bool HandleDevTraceCommand(ChatHandler* handler, std::string kind, Tail text)
    {
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!player)
            return false;

        DevTrace::Write("client", player->GetName(), kind, { DevTrace::Str("text", text) });
        return true;
    }
};

void AddSC_dev_commandscript()
{
    new dev_commandscript();
}
