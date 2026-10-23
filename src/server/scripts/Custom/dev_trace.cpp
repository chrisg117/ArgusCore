/*
 * Local dev tooling (chore(local), not for upstream): the devtrace.jsonl writer and the
 * player events it records.
 */

#include "dev_trace.h"
#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Util.h"
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <mutex>

namespace
{
    std::mutex traceLock;
    FILE* traceFile = nullptr;
    std::string tracePath;
    bool traceFailed = false;

    // Opened on first use, in append mode; a failure is logged once and tracing stays off.
    // If the file was deleted or moved away, it is reopened so lines don't vanish into it.
    FILE* OpenTrace()
    {
        if (traceFile && !std::filesystem::exists(tracePath))
        {
            fclose(traceFile);
            traceFile = nullptr;
        }
        if (traceFile || traceFailed)
            return traceFile;

        std::string dir = sConfigMgr->GetStringDefault("LogsDir", "");
        if (!dir.empty() && dir.back() != '/' && dir.back() != '\\')
            dir.push_back('/');
        std::string path = dir + "devtrace.jsonl";
        tracePath = path;

        traceFile = fopen(path.c_str(), "a");
        if (!traceFile)
        {
            traceFailed = true;
            TC_LOG_ERROR("scripts", "DevTrace: cannot open {}", path);
        }
        return traceFile;
    }

    std::string Now()
    {
        auto now = std::chrono::system_clock::now();
        time_t seconds = std::chrono::system_clock::to_time_t(now);
        int ms = int(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000);
        tm parts = TimeBreakdown(seconds);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            parts.tm_year + 1900, parts.tm_mon + 1, parts.tm_mday, parts.tm_hour, parts.tm_min, parts.tm_sec, ms);
        return buffer;
    }
}

std::string DevTrace::Escape(std::string_view text)
{
    std::string out;
    out.reserve(text.size());
    for (char c : text)
    {
        switch (c)
        {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                {
                    char buffer[8];
                    snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned char>(c));
                    out += buffer;
                }
                else
                    out += c;
                break;
        }
    }
    return out;
}

std::string DevTrace::Str(char const* key, std::string_view value)
{
    return "\"" + std::string(key) + "\":\"" + Escape(value) + "\"";
}

std::string DevTrace::Num(char const* key, int64 value)
{
    return "\"" + std::string(key) + "\":" + std::to_string(value);
}

std::string DevTrace::Bool(char const* key, bool value)
{
    return "\"" + std::string(key) + "\":" + (value ? "true" : "false");
}

void DevTrace::Write(std::string_view src, std::string_view charName, std::string_view kind, std::initializer_list<std::string> fields)
{
    std::string line = "{" + Str("t", Now()) + "," + Str("src", src) + "," + Str("char", charName) + "," + Str("kind", kind);
    for (std::string const& field : fields)
        line += "," + field;
    line += "}\n";

    std::lock_guard<std::mutex> guard(traceLock);
    if (FILE* file = OpenTrace())
    {
        fputs(line.c_str(), file);
        fflush(file);
    }
}

class dev_trace_player : public PlayerScript
{
public:
    dev_trace_player() : PlayerScript("dev_trace_player", {
        PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LOGOUT, PLAYERHOOK_ON_UPDATE_ZONE, PLAYERHOOK_ON_QUEST_STATUS_CHANGE,
        PLAYERHOOK_ON_SPELL_CAST, PLAYERHOOK_ON_MOVIE_COMPLETE, PLAYERHOOK_ON_PLAYER_KILLED_BY_CREATURE,
        PLAYERHOOK_ON_PVP_KILL, PLAYERHOOK_ON_PLAYER_REPOP }) { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        DevTrace::Write("server", player->GetName(), "login");
    }

    void OnLogout(Player* player) override
    {
        DevTrace::Write("server", player->GetName(), "logout");
    }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 newArea) override
    {
        DevTrace::Write("server", player->GetName(), "zone", { DevTrace::Num("zone", newZone), DevTrace::Num("area", newArea) });
    }

    void OnQuestStatusChange(Player* player, uint32 questId) override
    {
        DevTrace::Write("server", player->GetName(), "quest",
            { DevTrace::Num("quest", questId), DevTrace::Num("status", player->GetQuestStatus(questId)) });
    }

    void OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        DevTrace::Write("server", player->GetName(), "cast", { DevTrace::Num("spell", spell->GetSpellInfo()->Id) });
    }

    void OnMovieComplete(Player* player, uint32 movieId) override
    {
        DevTrace::Write("server", player->GetName(), "movie", { DevTrace::Num("movie", movieId) });
    }

    void OnPlayerKilledByCreature(Creature* /*killer*/, Player* killed) override
    {
        DevTrace::Write("server", killed->GetName(), "death");
    }

    void OnPVPKill(Player* /*killer*/, Player* killed) override
    {
        DevTrace::Write("server", killed->GetName(), "death");
    }

    void OnPlayerRepop(Player* player) override
    {
        DevTrace::Write("server", player->GetName(), "repop");
    }
};

void AddSC_dev_trace()
{
    new dev_trace_player();
}
