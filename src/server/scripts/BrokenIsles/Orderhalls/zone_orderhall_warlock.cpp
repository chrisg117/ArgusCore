/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "QuestDef.h"
#include "ScriptedCreature.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"

enum TheSixthData
{
    QUEST_THE_SIXTH                     = 40716,

    NPC_RISSTYNS_PORTAL                 = 103505,
    NPC_RITSSYN_THE_SIXTH               = 103506,

    SAY_RITSSYN_GREETING                = 0, // 106111 "A moment of your time, warlock..."
    SAY_RITSSYN_JOIN_US                 = 1, // 104179 "Join us at the Circle of Wills..."

    POINT_RITSSYN_PORTAL                = 1
};

// 103505 - Demonic Portal: summoned in Dalaran by 204858 (from the 7.0 Order Campaign aura 224240) for a warlock who
// has not taken The Sixth. Ritssyn steps out of it (204860, cast by the warlock so that he is the warlock's summon).
// The portal stays for its 5 minutes: a click casts 204857 Risstyn's Portal (npc_spellclick_spells), which takes the
// warlock to the Circle of Wills (Wowhead's acquisition video, 0:00-0:32).
struct npc_risstyns_portal : public ScriptedAI
{
    npc_risstyns_portal(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player || player->GetClass() != CLASS_WARLOCK || player->GetQuestStatus(QUEST_THE_SIXTH) == QUEST_STATUS_REWARDED)
        {
            me->DespawnOrUnsummon();
            return;
        }

        // Ritssyn only comes out for a warlock who has not taken the quest yet; one who has (and comes back) just gets
        // the portal
        if (player->GetQuestStatus(QUEST_THE_SIXTH) != QUEST_STATUS_NONE)
            return;

        // 204860 Summon Risstyn is the data's way (DEST_NEARBY_ENTRY the portal), but it is a self-range spell: this
        // core searches nearby entries within the spell's max range (0), so it never finds the portal 10 yd away and
        // summons nothing (2026-11-08, Claudeloc). Ritssyn is summoned here instead, as the warlock's summon, a step
        // out of the portal towards the warlock.
        ObjectGuid playerGuid = player->GetGUID();
        _scheduler.Schedule(1500ms, [this, playerGuid](TaskContext /*context*/)
        {
            Player* player = ObjectAccessor::GetPlayer(*me, playerGuid);
            if (!player)
                return;
            Position pos = me->GetPosition();
            float angle = me->GetAbsoluteAngle(player);
            me->MovePosition(pos, 1.5f, angle - me->GetOrientation());
            pos.SetOrientation(angle);
            player->SummonCreature(NPC_RITSSYN_THE_SIXTH, pos, TEMPSUMMON_TIMED_DESPAWN, 5min);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 103506 - Ritssyn Flamescowl: the copy that steps out of the portal, greets the warlock and offers The Sixth (fork
// data: the quest has no starter in the data). Once it is taken he points to his portal and steps back through it.
struct npc_ritssyn_flamescowl_the_sixth : public ScriptedAI
{
    npc_ritssyn_flamescowl_the_sixth(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* summoner) override
    {
        _summonerGuid = summoner->GetGUID();
        if (Player* player = summoner->ToPlayer())
            me->SetFacingToObject(player);
        _scheduler.Schedule(1s, [this](TaskContext /*context*/)
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGuid))
                Talk(SAY_RITSSYN_GREETING, player);
        });
    }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() != QUEST_THE_SIXTH)
            return;

        Talk(SAY_RITSSYN_JOIN_US, player);
        _scheduler.Schedule(5s, [this](TaskContext /*context*/)
        {
            if (Creature* portal = FindOwnPortal())
                me->GetMotionMaster()->MovePoint(POINT_RITSSYN_PORTAL, portal->GetPosition());
            else
                me->DespawnOrUnsummon();
        });
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_RITSSYN_PORTAL)
            me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    // Only this warlock's own portal: another warlock's may stand nearby
    Creature* FindOwnPortal() const
    {
        std::list<Creature*> portals;
        me->GetCreatureListWithEntryInGrid(portals, NPC_RISSTYNS_PORTAL, 30.0f);
        for (Creature* portal : portals)
            if (portal->IsSummon() && portal->ToTempSummon()->GetSummonerGUID() == _summonerGuid)
                return portal;
        return nullptr;
    }

    TaskScheduler _scheduler;
    ObjectGuid _summonerGuid;
};

void AddSC_orderhall_warlock()
{
    RegisterCreatureAI(npc_risstyns_portal);
    RegisterCreatureAI(npc_ritssyn_flamescowl_the_sixth);
}
