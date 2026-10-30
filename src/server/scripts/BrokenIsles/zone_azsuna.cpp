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

#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "TemporarySummon.h"
#include "Vehicle.h"

#include <array>

enum AzsunaQuests
{
    QUEST_DEMON_SOULS           = 37653,
    QUEST_SCYTHE_OF_SOULS       = 37660,
    QUEST_SAVING_STELLAGOSA     = 37450,
    QUEST_DARK_REVELATIONS      = 37449,

    OBJECTIVE_FIRST_SOUL_FREED  = 277152,
    OBJECTIVE_FIRST_SOUL_DONE   = 277153,
    OBJECTIVE_SECOND_SOUL_FREED = 277156,
    OBJECTIVE_SECOND_SOUL_DONE  = 277157,
    OBJECTIVE_GEM_ENTERED       = 277158,
    OBJECTIVE_AREVNAAL          = 277159,
    OBJECTIVE_NIGHTGLAIVE       = 277301
};

enum AzsunaCreatures
{
    NPC_SOUL_TRAPPED_CREDIT     = 90298,
    NPC_ALLARI_MOVED_CREDIT     = 90401,
    NPC_ALLARI_SPEAK            = 90403,
    NPC_FIRST_SOUL              = 90402,
    NPC_SECOND_SOUL             = 89276,
    NPC_AREVNAAL                = 89673,
    NPC_ALLARI                  = 89398,
    NPC_STELLAGOSA_CHAINED      = 90546,
    NPC_STELLAGOSA_RIDE         = 90982,
    NPC_ILLIDARI_LEADER_CREDIT  = 112175
};

enum AzsunaSpells
{
    SPELL_SOUL_GEM_TRANSFORM    = 178752,
    SPELL_SCYTHE_BEAM           = 178939
};

static constexpr float SoulGemRadius = 15.0f;
static constexpr float QuestPollRadius = 30.0f;
static constexpr Milliseconds QuestPollInterval = 1s;
static constexpr Milliseconds SoulCompelTime = 5s;

// The quest POIs of The Scythe of Souls
static constexpr Position AllariAtTheGems = { -156.0f, 6906.0f, 13.5f, 0.0f };
static constexpr Position FirstSoulSpot = { -159.0f, 6897.0f, 14.1f, 0.0f };
static constexpr Position SecondSoulSpot = { -164.0f, 6914.0f, 11.9f, 0.0f };
static constexpr Position ArevnaalSpot = { -162.0f, 6903.0f, 12.9f, 0.0f };

// Runs a check every second on the players near a quest NPC
struct AzsunaQuestPollAI : public ScriptedAI
{
    AzsunaQuestPollAI(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (_pollLeft > Milliseconds(diff))
        {
            _pollLeft -= Milliseconds(diff);
            return;
        }

        _pollLeft = QuestPollInterval;

        // A quest NPC can be phased away from the very player it waits for: Allari is, while The Scythe of Souls runs
        std::list<Player*> players;
        auto inRange = [this](Player* player) { return me->IsWithinDist(player, QuestPollRadius); };
        Trinity::PlayerListSearcher searcher(PhasingHandler::GetAlwaysVisiblePhaseShift(), players, inRange);
        Cell::VisitWorldObjects(me, searcher, QuestPollRadius);
        for (Player* player : players)
            CheckPlayer(player);
    }

    virtual void CheckPlayer(Player* player) = 0;

protected:
    TaskScheduler _scheduler;

private:
    Milliseconds _pollLeft = QuestPollInterval;
};

// 86963 - Soul Gem (Demon Souls): "Place a soul gem to trap the souls of dead demons within 15 yards" (178787)
struct npc_demon_souls_soul_gem : public ScriptedAI
{
    npc_demon_souls_soul_gem(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(WorldObject* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
            return;

        DoCastSelf(SPELL_SOUL_GEM_TRANSFORM, true);

        if (player->GetQuestStatus(QUEST_DEMON_SOULS) != QUEST_STATUS_INCOMPLETE)
            return;

        std::list<Creature*> corpses;
        auto isDemonCorpse = [this](Creature* creature)
        {
            return !creature->IsAlive() && creature->GetCreatureType() == CREATURE_TYPE_DEMON
                && me->IsWithinDist(creature, SoulGemRadius);
        };
        Trinity::CreatureListSearcher searcher(me->GetPhaseShift(), corpses, isDemonCorpse);
        Cell::VisitGridObjects(me, searcher, SoulGemRadius);

        for (Creature* corpse : corpses)
        {
            player->KilledMonsterCredit(NPC_SOUL_TRAPPED_CREDIT);
            corpse->DespawnOrUnsummon();
        }
    }
};

// 89398 - Allari the Souleater: brings the player's own Allari to the gems, and takes them back once Arev'naal has talked
struct npc_illidari_stand_allari : public AzsunaQuestPollAI
{
    npc_illidari_stand_allari(Creature* creature) : AzsunaQuestPollAI(creature) { }

    void CheckPlayer(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_SCYTHE_OF_SOULS) != QUEST_STATUS_INCOMPLETE)
            return;

        if (player->IsQuestObjectiveComplete(QUEST_SCYTHE_OF_SOULS, OBJECTIVE_AREVNAAL))
        {
            player->KilledMonsterCredit(NPC_ALLARI);
            return;
        }

        std::list<Creature*> allaris;
        player->GetCreatureListWithEntryInGrid(allaris, NPC_ALLARI_SPEAK, 200.0f);
        for (Creature* allari : allaris)
            if (allari->GetPrivateObjectOwner() == player->GetGUID())
                return;

        player->KilledMonsterCredit(NPC_ALLARI_MOVED_CREDIT);
        player->SummonCreature(NPC_ALLARI_SPEAK, AllariAtTheGems, TEMPSUMMON_TIMED_DESPAWN, 30min, 0, 0, player->GetGUID());
    }
};

// 90403 - Allari the Souleater (The Scythe of Souls): compels each soul the player releases, and calls up Arev'naal
struct npc_scythe_of_souls_allari : public ScriptedAI
{
    npc_scythe_of_souls_allari(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        player->KilledMonsterCredit(NPC_ALLARI_SPEAK);
        CloseGossipMenuFor(player);
        return true;
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (_pollLeft > Milliseconds(diff))
        {
            _pollLeft -= Milliseconds(diff);
            return;
        }

        _pollLeft = QuestPollInterval;

        Player* owner = ObjectAccessor::GetPlayer(*me, me->GetPrivateObjectOwner());
        if (!owner || owner->GetQuestStatus(QUEST_SCYTHE_OF_SOULS) != QUEST_STATUS_INCOMPLETE
            || owner->IsQuestObjectiveComplete(QUEST_SCYTHE_OF_SOULS, OBJECTIVE_AREVNAAL))
        {
            me->DespawnOrUnsummon();
            return;
        }

        if (!_firstSoul && owner->IsQuestObjectiveComplete(QUEST_SCYTHE_OF_SOULS, OBJECTIVE_FIRST_SOUL_FREED)
            && !owner->IsQuestObjectiveComplete(QUEST_SCYTHE_OF_SOULS, OBJECTIVE_FIRST_SOUL_DONE))
        {
            _firstSoul = true;
            CompelSoul(owner, NPC_FIRST_SOUL, FirstSoulSpot);
        }

        if (!_secondSoul && owner->IsQuestObjectiveComplete(QUEST_SCYTHE_OF_SOULS, OBJECTIVE_SECOND_SOUL_FREED)
            && !owner->IsQuestObjectiveComplete(QUEST_SCYTHE_OF_SOULS, OBJECTIVE_SECOND_SOUL_DONE))
        {
            _secondSoul = true;
            CompelSoul(owner, NPC_SECOND_SOUL, SecondSoulSpot);
        }

        if (!_arevnaal && owner->IsQuestObjectiveComplete(QUEST_SCYTHE_OF_SOULS, OBJECTIVE_GEM_ENTERED))
        {
            _arevnaal = true;
            me->SummonCreature(NPC_AREVNAAL, ArevnaalSpot, TEMPSUMMON_TIMED_DESPAWN, 30min, 0, 0, owner->GetGUID());
        }
    }

private:
    void CompelSoul(Player* owner, uint32 soulEntry, Position const& spot)
    {
        Creature* soul = me->SummonCreature(soulEntry, spot, TEMPSUMMON_TIMED_DESPAWN, SoulCompelTime + 2s, 0, 0, owner->GetGUID());
        if (!soul)
            return;

        DoCast(soul, SPELL_SCYTHE_BEAM, true);

        ObjectGuid ownerGuid = owner->GetGUID();
        _scheduler.Schedule(SoulCompelTime, [this, ownerGuid, soulEntry](TaskContext)
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, ownerGuid))
                player->KilledMonsterCredit(soulEntry);
        });
    }

    TaskScheduler _scheduler;
    Milliseconds _pollLeft = QuestPollInterval;
    bool _firstSoul = false;
    bool _secondSoul = false;
    bool _arevnaal = false;
};

// 89673 - Questioner Arev'naal (The Scythe of Souls): inside the gem
struct npc_scythe_of_souls_arevnaal : public ScriptedAI
{
    npc_scythe_of_souls_arevnaal(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        player->KilledMonsterCredit(NPC_AREVNAAL);
        CloseGossipMenuFor(player);
        me->DespawnOrUnsummon(2s);
        return true;
    }
};

// 90546 - Stellagosa (Saving Stellagosa): "Find Stellagosa"
struct npc_saving_stellagosa_stellagosa : public AzsunaQuestPollAI
{
    npc_saving_stellagosa_stellagosa(Creature* creature) : AzsunaQuestPollAI(creature) { }

    void CheckPlayer(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_SAVING_STELLAGOSA) == QUEST_STATUS_INCOMPLETE)
            player->KilledMonsterCredit(NPC_STELLAGOSA_CHAINED);
    }
};

// 90621 - Nightglaive the Traitor (Dark Revelations): Stellagosa comes for her killer
struct npc_dark_revelations_nightglaive : public ScriptedAI
{
    npc_dark_revelations_nightglaive(Creature* creature) : ScriptedAI(creature) { }

    void JustDied(Unit* killer) override
    {
        Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (player && player->GetQuestStatus(QUEST_DARK_REVELATIONS) == QUEST_STATUS_INCOMPLETE)
            player->SummonCreature(NPC_STELLAGOSA_RIDE, player->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 5min, 0, 0, player->GetGUID());
    }
};

static constexpr std::array<Position, 3> StellagosaFlightToIllidariStand =
{{
    { -480.0f, 7420.0f, 110.0f },
    { -280.0f, 7190.0f, 90.0f },
    { -112.0f, 6972.0f, 17.0f }
}};

static constexpr float StellagosaRideSpeed = 28.0f;

enum StellagosaRide
{
    POINT_STELLAGOSA_ILLIDARI_STAND = 1
};

// 90982 - Stellagosa (Dark Revelations): 180768 on spell click boards the player and counts as the ride; flies to Kayn
struct npc_dark_revelations_stellagosa : public ScriptedAI
{
    npc_dark_revelations_stellagosa(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!apply || !passenger->IsPlayer() || _flying)
            return;

        _flying = true;
        me->SetDisableGravity(true);
        me->GetMotionMaster()->LaunchMoveSpline([](Movement::MoveSplineInit& init)
        {
            Movement::PointsArray path;
            for (Position const& point : StellagosaFlightToIllidariStand)
                path.emplace_back(point.GetPositionX(), point.GetPositionY(), point.GetPositionZ());
            init.MovebyPath(path);
            init.SetFly();
            init.SetVelocity(StellagosaRideSpeed);
        }, POINT_STELLAGOSA_ILLIDARI_STAND);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE || id != POINT_STELLAGOSA_ILLIDARI_STAND)
            return;

        if (Vehicle* vehicle = me->GetVehicleKit())
            vehicle->RemoveAllPassengers();
        me->DespawnOrUnsummon(2s);
    }

private:
    bool _flying = false;
};

// 89362 - Kayn Sunfury: "Meet back at Illidari Stand"
struct npc_illidari_stand_kayn : public AzsunaQuestPollAI
{
    npc_illidari_stand_kayn(Creature* creature) : AzsunaQuestPollAI(creature) { }

    void CheckPlayer(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_DARK_REVELATIONS) == QUEST_STATUS_INCOMPLETE
            && player->IsQuestObjectiveComplete(QUEST_DARK_REVELATIONS, OBJECTIVE_NIGHTGLAIVE))
            player->KilledMonsterCredit(NPC_ILLIDARI_LEADER_CREDIT);
    }
};

void AddSC_zone_azsuna()
{
    RegisterCreatureAI(npc_demon_souls_soul_gem);
    RegisterCreatureAI(npc_illidari_stand_allari);
    RegisterCreatureAI(npc_scythe_of_souls_allari);
    RegisterCreatureAI(npc_scythe_of_souls_arevnaal);
    RegisterCreatureAI(npc_saving_stellagosa_stellagosa);
    RegisterCreatureAI(npc_dark_revelations_nightglaive);
    RegisterCreatureAI(npc_dark_revelations_stellagosa);
    RegisterCreatureAI(npc_illidari_stand_kayn);
}
