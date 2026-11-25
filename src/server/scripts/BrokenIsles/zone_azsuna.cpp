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

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "CellImpl.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "PathGenerator.h"
#include "QuestDef.h"
#include "SpellScript.h"
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
#include "WaypointManager.h"

#include <array>
#include <cmath>
#include <unordered_map>

enum AzsunaQuests
{
    QUEST_DEMON_SOULS           = 37653,
    QUEST_SCYTHE_OF_SOULS       = 37660,
    QUEST_SAVING_STELLAGOSA     = 37450,
    QUEST_DARK_REVELATIONS      = 37449,
    QUEST_DEATH_OF_THE_ELDEST   = 37853,
    QUEST_AGAPANTHUS            = 37991,
    QUEST_THEIR_DYING_BREATHS   = 42271,

    OBJECTIVE_FIRST_SOUL_FREED  = 277152,
    OBJECTIVE_FIRST_SOUL_DONE   = 277153,
    OBJECTIVE_SECOND_SOUL_FREED = 277156,
    OBJECTIVE_SECOND_SOUL_DONE  = 277157,
    OBJECTIVE_GEM_ENTERED       = 277158,
    OBJECTIVE_AREVNAAL          = 277159,
    OBJECTIVE_NIGHTGLAIVE       = 277301,
    OBJECTIVE_LEYWORM_CORES     = 277011,
    OBJECTIVE_WHELPLINGS_REVIVED = 284015
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
    NPC_ILLIDARI_LEADER_CREDIT  = 112175,
    NPC_SENEGOS_POOL_CREDIT     = 90315,
    NPC_AGAPANTHUS              = 90543
};

enum AzsunaSpells
{
    SPELL_SOUL_GEM_TRANSFORM    = 178752,
    SPELL_SCYTHE_BEAM           = 178939,
    SPELL_TOSS_CRYSTAL_MISSILE  = 179913,
    SPELL_TOSS_CRYSTALS_BAR     = 179916
};

static constexpr float SoulGemRadius = 15.0f;
static constexpr float QuestPollRadius = 30.0f;
static constexpr Milliseconds QuestPollInterval = 1s;
static constexpr Milliseconds SoulCompelTime = 5s;
static constexpr float SenegosPoolRadius = 20.0f;
static constexpr uint32 ITEM_GRIEVOUSLY_WOUNDED_WHELPLING = 122292;
static constexpr Milliseconds WhelplingReviveTime = 2s;
static constexpr Seconds WhelplingRespawnTime = 30s;

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

// 89975 - Senegos (The Death of the Eldest): with the six cores in bags at his pool, the player gets the override bar of
// Toss Crystals (179916, OverrideSpellData 573 -> 179915), and loses it on stepping away or once the pool counts
struct npc_death_of_the_eldest_senegos : public AzsunaQuestPollAI
{
    npc_death_of_the_eldest_senegos(Creature* creature) : AzsunaQuestPollAI(creature) { }

    void CheckPlayer(Player* player) override
    {
        bool wanted = player->GetQuestStatus(QUEST_DEATH_OF_THE_ELDEST) == QUEST_STATUS_INCOMPLETE
            && me->IsWithinDist(player, SenegosPoolRadius)
            && player->IsQuestObjectiveComplete(QUEST_DEATH_OF_THE_ELDEST, OBJECTIVE_LEYWORM_CORES)
            && player->GetReqKillOrCastCurrentCount(QUEST_DEATH_OF_THE_ELDEST, NPC_SENEGOS_POOL_CREDIT) == 0;
        if (wanted && !player->HasAura(SPELL_TOSS_CRYSTALS_BAR))
            player->CastSpell(player, SPELL_TOSS_CRYSTALS_BAR, true);
        else if (!wanted && player->HasAura(SPELL_TOSS_CRYSTALS_BAR))
            player->RemoveAurasDueToSpell(SPELL_TOSS_CRYSTALS_BAR);
    }
};

// 90543 - Agapanthus: "Investigate the visitors" is reaching her
struct npc_agapanthus : public AzsunaQuestPollAI
{
    npc_agapanthus(Creature* creature) : AzsunaQuestPollAI(creature) { }

    void CheckPlayer(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_AGAPANTHUS) == QUEST_STATUS_INCOMPLETE)
            player->KilledMonsterCredit(NPC_AGAPANTHUS);
    }
};

// 90880 - Mana-Drained Whelpling (Their Dying Breaths): the click casts Resuscitating (180713), which credits. A revived
// whelpling leaves for a while, so one whelpling can't be revived four times. The last one is the Grievously Wounded
// Whelpling the player carries back to Senegos: nothing in the data hands that item out.
struct npc_their_dying_breaths_whelpling : public ScriptedAI
{
    npc_their_dying_breaths_whelpling(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void OnSpellClick(Unit* clicker, bool spellClickHandled) override
    {
        Player* player = clicker->ToPlayer();
        if (!player || !spellClickHandled)
            return;

        ObjectGuid playerGuid = player->GetGUID();
        int32 revivedBefore = player->GetQuestObjectiveData(QUEST_THEIR_DYING_BREATHS, OBJECTIVE_WHELPLINGS_REVIVED);
        _scheduler.Schedule(WhelplingReviveTime, [this, playerGuid, revivedBefore](TaskContext)
        {
            Player* reviver = ObjectAccessor::GetPlayer(*me, playerGuid);
            if (!reviver || reviver->GetQuestObjectiveData(QUEST_THEIR_DYING_BREATHS, OBJECTIVE_WHELPLINGS_REVIVED) <= revivedBefore)
                return;

            if (reviver->IsQuestObjectiveComplete(QUEST_THEIR_DYING_BREATHS, OBJECTIVE_WHELPLINGS_REVIVED)
                && !reviver->HasItemCount(ITEM_GRIEVOUSLY_WOUNDED_WHELPLING))
                reviver->AddItem(ITEM_GRIEVOUSLY_WOUNDED_WHELPLING, 1);

            me->DespawnOrUnsummon(0ms, WhelplingRespawnTime);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

enum FarondisEscort
{
    QUEST_TRAILING_THE_TIDESTONE    = 37497,
    QUEST_THE_WALK_OF_SHAME         = 37467,

    NPC_FARONDIS_ESCORT_RUINS       = 88869,    // Summon Farondis 177755: the gate to the Ruins of Nar'thalas
    NPC_FARONDIS_ESCORT_EDGE        = 88889,    // Summon Farondis 177816: the ruins around the edge of town
    NPC_TRAILING_ESCORT_CREDIT      = 90077,    // Trailing the Crescent Escort Credit B
    NPC_DROWNED_GARDENS_CREDIT      = 88746,

    PATH_FARONDIS_TO_RUINS          = 1220001,
    PATH_FARONDIS_EDGE_OF_TOWN      = 1220002
};

static constexpr float FarondisWaitRadius = 20.0f;     // walks on once his player is this close
static constexpr float FarondisCreditRadius = 30.0f;   // every player on the quest this close is credited on arrival
static constexpr float FarondisLostRadius = 80.0f;
static constexpr Seconds FarondisLostTime = 60s;
static constexpr float FarondisDetourFactor = 1.5f;     // a mesh route longer than this times the straight line is a detour

struct FarondisWalk
{
    uint32 Escort;
    uint32 Quest;
    uint32 Path;
    uint32 Credit;
};

static constexpr std::array<FarondisWalk, 2> FarondisWalks =
{{
    { NPC_FARONDIS_ESCORT_RUINS, QUEST_TRAILING_THE_TIDESTONE, PATH_FARONDIS_TO_RUINS,     NPC_TRAILING_ESCORT_CREDIT },
    { NPC_FARONDIS_ESCORT_EDGE,  QUEST_THE_WALK_OF_SHAME,      PATH_FARONDIS_EDGE_OF_TOWN, NPC_DROWNED_GARDENS_CREDIT }
}};

static FarondisWalk const* GetFarondisWalk(uint32 escortEntry)
{
    for (FarondisWalk const& walk : FarondisWalks)
        if (walk.Escort == escortEntry)
            return &walk;
    return nullptr;
}

// 88869, 88889 - Prince Farondis (Trailing the Tidestone, The Walk of Shame): a personal summon of the player who asked
// for the walk. Walks the path node by node, waiting at each node until that player is close, and credits every player
// on the quest near him at the end. Leaves when his player drops the quest, logs out, or stays far away for a minute.
struct npc_farondis_escort : public ScriptedAI
{
    npc_farondis_escort(Creature* creature) : ScriptedAI(creature), _walk(GetFarondisWalk(creature->GetEntry())) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetWalk(true);
    }

    void IsSummonedBy(WorldObject* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player || !_walk)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGuid = player->GetGUID();

        // One walk per player: a second request replaces the first
        std::list<Creature*> others;
        me->GetCreatureListWithEntryInGrid(others, me->GetEntry(), FarondisLostRadius * 2);
        for (Creature* other : others)
            if (other != me && other->GetPrivateObjectOwner() == _playerGuid)
                other->DespawnOrUnsummon();

        _scheduler.Schedule(2s, [this](TaskContext)
        {
            _waiting = true;    // the first node is reached as soon as the player stands by
        });
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE || id != _nextNode)
            return;

        WaypointPath const* path = sWaypointMgr->GetPath(_walk->Path);
        if (!path || ++_nextNode >= path->Nodes.size())
        {
            Arrive();
            return;
        }

        _waiting = true;
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

        if (_arrived || !_playerGuid)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid);
        if (!player || player->GetQuestStatus(_walk->Quest) != QUEST_STATUS_INCOMPLETE)
        {
            me->DespawnOrUnsummon();
            return;
        }

        if (!me->IsWithinDist(player, FarondisLostRadius))
        {
            _lostFor += QuestPollInterval;
            if (_lostFor >= FarondisLostTime)
                me->DespawnOrUnsummon();
            return;
        }

        _lostFor = 0ms;

        if (_waiting && me->IsWithinDist(player, FarondisWaitRadius))
            MoveToNextNode();
    }

private:
    void MoveToNextNode()
    {
        WaypointPath const* path = sWaypointMgr->GetPath(_walk->Path);
        if (!path || _nextNode >= path->Nodes.size())
        {
            Arrive();
            return;
        }

        _waiting = false;
        WaypointNode const& node = path->Nodes[_nextNode];
        me->GetMotionMaster()->MovePoint(_nextNode, node.X, node.Y, node.Z, UseMeshPath(node));
    }

    // The nodes are a hand-laid road a few yards apart. Where the navmesh has no complete route between two of
    // them (the steps below the Crumbled Palace gate carry no mesh: the path finder sent him 90 yd around for a
    // 17 yd hop), or only a long way round, he walks the straight line as a player would.
    bool UseMeshPath(WaypointNode const& node) const
    {
        PathGenerator mesh(me);
        mesh.CalculatePath(node.X, node.Y, node.Z);
        if (mesh.GetPathType() != PATHFIND_NORMAL)
            return false;

        return mesh.GetPathLength() <= me->GetExactDist(node.X, node.Y, node.Z) * FarondisDetourFactor;
    }

    void Arrive()
    {
        if (_arrived)
            return;

        _arrived = true;
        _waiting = false;

        std::list<Player*> players;
        auto inRange = [this](Player* player) { return me->IsWithinDist(player, FarondisCreditRadius); };
        Trinity::PlayerListSearcher searcher(PhasingHandler::GetAlwaysVisiblePhaseShift(), players, inRange);
        Cell::VisitWorldObjects(me, searcher, FarondisCreditRadius);
        for (FarondisWalk const& walk : FarondisWalks)
        {
            if (walk.Escort != me->GetEntry())
                continue;
            for (Player* player : players)
                if (player->GetQuestStatus(walk.Quest) == QUEST_STATUS_INCOMPLETE)
                    player->KilledMonsterCredit(walk.Credit);
        }

        me->DespawnOrUnsummon(5s);
    }

    FarondisWalk const* _walk;
    TaskScheduler _scheduler;
    ObjectGuid _playerGuid;
    Milliseconds _pollLeft = QuestPollInterval;
    Milliseconds _lostFor = 0ms;
    uint32 _nextNode = 0;
    bool _waiting = false;
    bool _arrived = false;
};

// 88115, 88867 - Prince Farondis (gate, ruins): the static Farondis phases out as the walk is accepted (phases 4270,
// 4290) and the walking one takes his place. Retail casts Summon Farondis 177755 / 177816; the walking copy is summoned
// directly here so that its summoner is the player.
struct npc_farondis_tidestone_giver : public ScriptedAI
{
    npc_farondis_tidestone_giver(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        uint32 escortEntry = 0;
        switch (quest->GetQuestId())
        {
            case QUEST_TRAILING_THE_TIDESTONE: escortEntry = NPC_FARONDIS_ESCORT_RUINS; break;
            case QUEST_THE_WALK_OF_SHAME:      escortEntry = NPC_FARONDIS_ESCORT_EDGE; break;
            default: return;
        }

        player->SummonCreature(escortEntry, me->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, player->GetGUID());
    }
};

enum NarthalasAcademy
{
    QUEST_WANDING_101               = 42370,
    QUEST_STUDY_HALL                = 42371,
    QUEST_POP_QUIZ                  = 37729,

    NPC_WAND_TARGET                 = 107279,

    SPELL_WAND_SHOT                 = 179174,   // the missile from the player's wand to the target (dummy, visual)

    // Pop Quiz: Advanced Rune Drawing
    SPELL_RUNE_DRAWING_ARCANE       = 179151,   // Rune Drawing 101: Rune A, plays scene 935
    SPELL_RUNE_DRAWING_FIRE         = 179152,   // Rune B, scene 936
    SPELL_RUNE_DRAWING_FROST        = 179153,   // Rune C, scene 937
    SCENE_RUNE_ARCANE               = 935,
    SCENE_RUNE_FIRE                 = 936,
    SCENE_RUNE_FROST                = 937,
    NPC_RUNE_CREDIT_ARCANE          = 89655,
    NPC_RUNE_CREDIT_FIRE            = 89656,
    NPC_RUNE_CREDIT_FROST           = 89657,
    AT_PROPERTIES_RUNE_ARCANE       = 1220006,
    AT_PROPERTIES_RUNE_FIRE         = 1220007,
    AT_PROPERTIES_RUNE_FROST        = 1220008,

    // Study Hall: Combat Research
    SCENE_INTO_THE_BOOK_1           = 1279,
    SCENE_INTO_THE_BOOK_2           = 1280,
    SCENE_INTO_THE_BOOK_3           = 1281
};

static constexpr float WandMinRange = 5.0f;     // "And no point-blank shots! That's cheating."
static constexpr float WandMaxRange = 40.0f;
static constexpr float WandCone = float(M_PI) / 3.0f;   // Wand Practice's 60 degree cone

// 179915 - Toss Crystals (The Death of the Eldest): the override bar's button. Effect 0 removes the bar (179916); the
// crystals fly at Senegos (179913, its target set by conditions) and the pool counts. Effect 1's periodic spell
// (179917) is not in the client's data, so the aura it would tick is not applied.
class spell_death_of_the_eldest_toss_crystals : public SpellScript
{
    void PreventEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void HandleToss()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_DEATH_OF_THE_ELDEST) != QUEST_STATUS_INCOMPLETE
            || !player->IsQuestObjectiveComplete(QUEST_DEATH_OF_THE_ELDEST, OBJECTIVE_LEYWORM_CORES))
            return;

        player->CastSpell(player, SPELL_TOSS_CRYSTAL_MISSILE, true);
        player->KilledMonsterCredit(NPC_SENEGOS_POOL_CREDIT);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_death_of_the_eldest_toss_crystals::PreventEffect, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
        AfterCast += SpellCastFn(spell_death_of_the_eldest_toss_crystals::HandleToss);
    }
};

// 212782 - Wand Practice: the button the override bar of Wanding 101 gives (OverrideSpellData 711). Its area trigger
// (7042) has no properties and its triggered spell (218570) is not in the client's data, so the wand shot is played
// here: the nearest Wand Target in front of the caster, from 5 to 40 yards, is hit and counts.
class spell_narthalas_wand_practice : public SpellScript
{
    void PreventEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void HandleShot()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_WANDING_101) != QUEST_STATUS_INCOMPLETE)
            return;

        std::list<Creature*> targets;
        player->GetCreatureListWithEntryInGrid(targets, NPC_WAND_TARGET, WandMaxRange);
        Creature* best = nullptr;
        for (Creature* target : targets)
        {
            if (!target->IsAlive() || !player->CanSeeOrDetect(target) || player->IsWithinDist(target, WandMinRange)
                || !player->HasInArc(WandCone, target) || !player->IsWithinLOSInMap(target))
                continue;
            if (!best || player->GetExactDist2dSq(target) < player->GetExactDist2dSq(best))
                best = target;
        }

        if (!best)
            return;

        player->CastSpell(best, SPELL_WAND_SHOT, true);
        player->KilledMonsterCredit(NPC_WAND_TARGET, best->GetGUID());
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_narthalas_wand_practice::PreventEffect, EFFECT_0, SPELL_EFFECT_CREATE_AREATRIGGER);
        OnEffectHitTarget += SpellEffectFn(spell_narthalas_wand_practice::PreventEffect, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        AfterCast += SpellCastFn(spell_narthalas_wand_practice::HandleShot);
    }
};

struct NarthalasRune
{
    uint32 AreaTriggerProperties;
    uint32 Spell;
    uint32 Scene;
    uint32 Credit;
};

static constexpr std::array<NarthalasRune, 3> NarthalasRunes =
{{
    { AT_PROPERTIES_RUNE_ARCANE, SPELL_RUNE_DRAWING_ARCANE, SCENE_RUNE_ARCANE, NPC_RUNE_CREDIT_ARCANE },
    { AT_PROPERTIES_RUNE_FIRE,   SPELL_RUNE_DRAWING_FIRE,   SCENE_RUNE_FIRE,   NPC_RUNE_CREDIT_FIRE },
    { AT_PROPERTIES_RUNE_FROST,  SPELL_RUNE_DRAWING_FROST,  SCENE_RUNE_FROST,  NPC_RUNE_CREDIT_FROST }
}};

static bool HasRuneCredit(Player* player, uint32 credit)
{
    Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_POP_QUIZ);
    if (!quest)
        return true;
    for (QuestObjective const& objective : quest->GetObjectives())
        if (objective.Type == QUEST_OBJECTIVE_MONSTER && uint32(objective.ObjectID) == credit)
            return player->IsQuestObjectiveComplete(QUEST_POP_QUIZ, objective.ID);
    return true;
}

// Server-side area triggers of ours on the academy floor, one per rune outline ("I will provide outlines for you. You can
// simply use your feet to trace the runes."): stepping on one starts its tracing scene. Nothing in the client's data
// casts the three scene auras.
struct at_narthalas_rune_outline : AreaTriggerAI
{
    at_narthalas_rune_outline(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_POP_QUIZ) != QUEST_STATUS_INCOMPLETE)
            return;

        for (NarthalasRune const& rune : NarthalasRunes)
            if (rune.AreaTriggerProperties == at->GetCreateProperties()->Id.Id)
            {
                if (!HasRuneCredit(player, rune.Credit) && !player->HasAura(rune.Spell))
                    player->CastSpell(player, rune.Spell, true);
                return;
            }
    }
};

// Scenes 935 to 937 (Azsuna - Academy - Runes A to C): the client's scene traces the rune under the player's feet and
// sends the server event "Credit" once every circle of the rune is filled.
class scene_narthalas_rune_drawing : public SceneScript
{
public:
    scene_narthalas_rune_drawing() : SceneScript("scene_narthalas_rune_drawing") { }

    void OnSceneTriggerEvent(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* sceneTemplate, std::string const& triggerName) override
    {
        if (triggerName != "Credit")
            return;

        for (NarthalasRune const& rune : NarthalasRunes)
            if (rune.Scene == sceneTemplate->SceneId && player->GetQuestStatus(QUEST_POP_QUIZ) == QUEST_STATUS_INCOMPLETE)
                player->KilledMonsterCredit(rune.Credit);
    }

    void OnSceneComplete(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* sceneTemplate) override
    {
        EndRune(player, sceneTemplate);
    }

    void OnSceneCancel(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* sceneTemplate) override
    {
        EndRune(player, sceneTemplate);
    }

private:
    // The scene's aura would otherwise stay, and a rune left half-drawn could not be started again
    static void EndRune(Player* player, SceneTemplate const* sceneTemplate)
    {
        for (NarthalasRune const& rune : NarthalasRunes)
            if (rune.Scene == sceneTemplate->SceneId)
                player->RemoveAurasDueToSpell(rune.Spell);
    }
};

struct StudyHallBook
{
    uint32 Item;            // the book from the shelf
    uint32 ItemObjective;
    uint32 BookCredit;      // Book Credit 01..03: kill credit for the podium and the scene "Into the Book"
    uint32 BookObjective;
    uint32 Scene;
    uint32 Drawing;         // the challenge inside the book
    uint32 DrawingObjective;
};

static constexpr std::array<StudyHallBook, 3> StudyHallBooks =
{{
    { 137423, 284157, 212912, 284158, SCENE_INTO_THE_BOOK_1, 107301, 284150 },  // Encyclopedia Azsunica (K-M), Kobold Drawing
    { 137422, 284159, 212913, 284160, SCENE_INTO_THE_BOOK_2, 107300, 284149 },  // Courtship Rituals of the Skrog, Sea Skrog Drawing
    { 137426, 284161, 212914, 284162, SCENE_INTO_THE_BOOK_3, 107299, 284151 }   // Draconic Compendium, Volume III, Senegos Drawing
}};

static constexpr Seconds StudyHallDrawingTime = 3min;

// 250362 - Sythorne's Podium: its goober spell (212924) is not in the client's data. The book the player brought next
// goes on the podium: Book Credit 01..03 counts it and plays the scene that takes the player into the book.
struct go_sythorne_podium : public GameObjectAI
{
    go_sythorne_podium(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_STUDY_HALL) != QUEST_STATUS_INCOMPLETE)
            return true;

        for (StudyHallBook const& book : StudyHallBooks)
        {
            if (!player->IsQuestObjectiveComplete(QUEST_STUDY_HALL, book.BookObjective))
            {
                if (player->IsQuestObjectiveComplete(QUEST_STUDY_HALL, book.ItemObjective))
                    player->CastSpell(player, book.BookCredit, true);
                return true;
            }

            if (!player->IsQuestObjectiveComplete(QUEST_STUDY_HALL, book.DrawingObjective))
                return true;    // this book's challenge first: the objectives are sequenced
        }
        return true;
    }
};

// Scenes 1279 to 1281 (Into the Book 01 to 03): the scene sends "phase" as the camera comes out of the book, and the
// book's challenge, the drawing, stands before the player.
class scene_study_hall_into_the_book : public SceneScript
{
public:
    scene_study_hall_into_the_book() : SceneScript("scene_study_hall_into_the_book") { }

    void OnSceneTriggerEvent(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* sceneTemplate, std::string const& triggerName) override
    {
        if (triggerName != "phase" || player->GetQuestStatus(QUEST_STUDY_HALL) != QUEST_STATUS_INCOMPLETE)
            return;

        for (StudyHallBook const& book : StudyHallBooks)
        {
            if (book.Scene != sceneTemplate->SceneId || player->IsQuestObjectiveComplete(QUEST_STUDY_HALL, book.DrawingObjective))
                continue;

            Position spot = player->GetNearPosition(6.0f, 0.0f);
            if (Creature* drawing = player->SummonCreature(book.Drawing, spot, TEMPSUMMON_TIMED_DESPAWN, StudyHallDrawingTime, 0, 0, player->GetGUID()))
                drawing->SetFacingToObject(player);
        }
    }
};

enum FaronaarCompanion
{
    QUEST_FEL_MACHINATIONS              = 37656,

    // Kor'vas walks with the player through Fel Machinations and Saving Stellagosa and fights beside her (the user's
    // ruling from the quest videos). The DB2's spell for this, 178860 "Faronaar: Summon Kor'vas Bloodthorn", summons 90474,
    // a generic female demon hunter that is also the quest ender at the camp; we summon an unspawned Kor'vas template
    // instead, so the follower carries her name and display (66159). 90624 is the one that scales 98..110 like Azsuna.
    NPC_FARONAAR_KORVAS                 = 90624,
    SPELL_FARONAAR_SUMMON_KORVAS        = 178860,   // kept as the summon's created-by spell only

    ZONE_AZSUNA                         = 7334,

    OBJECTIVE_LYKILLS_KEY               = 277456,
    OBJECTIVE_FIRST_CHAIN_UNLOCKED      = 283274,
    NPC_FIRST_CHAIN_UNLOCKED_CREDIT     = 105635,

    SPELL_LYKILL_FEL_IMPRISONMENT       = 183020,   // 2 s cast: roots and burns the target for 6 s; interruptible
    SPELL_LYKILL_FEL_CLEAVE             = 184248,   // instant 12 yd frontal cone

    // BroadcastText (Kor'vas's lines are the female text)
    TEXT_KORVAS_FIND_THE_KEY            = 92337,    // "Quickly, $p, we have to find the key to those demonic locks..."
    TEXT_KORVAS_GET_THE_KEY             = 92339,    // "Get the key. Let's get over there and free that drake."
    TEXT_KORVAS_SOON_FREE               = 93025,    // "You will soon be free, drake."
    TEXT_KORVAS_COME_BACK               = 92980,    // "Stellagosa, no... come back. Damn!"
    TEXT_LYKILL_AGGRO                   = 97407,    // "There is plenty of room in my cage for your souls."
    TEXT_LYKILL_DEATH                   = 93273     // "I WILL return!"
};

static constexpr Milliseconds KorvasCheckInterval = 1s;
static constexpr float KorvasFollowDist = 2.5f;
static constexpr float KorvasFollowAngle = float(M_PI) * 0.75f;   // behind the player's left shoulder
static constexpr float KorvasLeashDist = 40.0f;                    // in combat: a fight further than this from her player is dropped
static constexpr float KorvasSprintDist = 12.0f;                   // out of combat and further than this: she runs faster to catch up
static constexpr float KorvasSprintRate = 2.5f;                    // run speed rate while catching up (Fel Rush, glides)
static constexpr uint32 KorvasStuckChecks = 2;                     // checks (1 s each) behind without getting closer
static constexpr float KorvasTeleportDist = 90.0f;                 // further than this (a flight path, a long glide): she appears beside her player
static constexpr float KorvasGiverHandoffDist = 30.0f;              // a giver this close is where the follower appears
static constexpr ObjectGuid::LowType FaronaarKorvasGiverSpawns[] = { 651475, 12201032 };   // Vanquish Point, Faronaar

static constexpr Milliseconds LykillImprisonmentFirst = 8s;
static constexpr Milliseconds LykillImprisonmentEvery = 18s;
static constexpr Milliseconds LykillCleaveFirst = 5s;
static constexpr Milliseconds LykillCleaveEvery = 11s;

// In Azsuna from Fel Machinations until Dark Revelations is taken, the turn-in of the first quest included. On retail both quests are turned in to the Kor'vas
// following you, and Dark Revelations is taken from her (WoW-Pro 7.3.5 Azsuna), so she stays through both turn-ins and
// leaves once the next quest is in the log. The camp's 90474 (12201003) stays the fallback ender and starter.
static bool IsFaronaarKorvasWanted(Player const* player)
{
    if (player->GetZoneId() != ZONE_AZSUNA)
        return false;

    QuestStatus felMachinations = player->GetQuestStatus(QUEST_FEL_MACHINATIONS);
    if (felMachinations == QUEST_STATUS_INCOMPLETE || felMachinations == QUEST_STATUS_COMPLETE)
        return true;

    // between the two quests she offers Saving Stellagosa herself
    QuestStatus savingStellagosa = player->GetQuestStatus(QUEST_SAVING_STELLAGOSA);
    if (felMachinations == QUEST_STATUS_REWARDED && savingStellagosa == QUEST_STATUS_NONE)
        return true;

    if (savingStellagosa == QUEST_STATUS_INCOMPLETE || savingStellagosa == QUEST_STATUS_COMPLETE)
        return true;

    return savingStellagosa == QUEST_STATUS_REWARDED && player->GetQuestStatus(QUEST_DARK_REVELATIONS) == QUEST_STATUS_NONE;
}

// Every Faronaar line is logged, so a run's Server.log shows which ones fired (the chat itself is client-side only)
static void FaronaarSay(Creature* speaker, uint32 textId, WorldObject const* target = nullptr)
{
    TC_LOG_INFO("scripts", "Faronaar: {} ({}) says broadcast text {} to {}", speaker->GetName(), speaker->GetEntry(), textId,
        target ? target->GetName() : std::string("nobody"));
    speaker->Say(textId, target);
}

// 90624 - Kor'vas Bloodthorn (Fel Machinations, Saving Stellagosa): personal companion. She follows her player, attacks
// what her player fights, cannot die, and leaves when neither quest needs her (the AI checks every second, so a relog,
// a teleport out of Azsuna or an abandoned quest all end her).
// each player's follower Kor'vas, by player: a grid search (250 yd) misses her right after her player jumps far (a
// flight, a teleport), before her catch-up, and a second one would be summoned
static std::unordered_map<ObjectGuid, ObjectGuid> FaronaarKorvasByPlayer;

struct npc_faronaar_korvas : public ScriptedAI
{
    npc_faronaar_korvas(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
            return;

        _player = player->GetGUID();
        FaronaarKorvasByPlayer[_player] = me->GetGUID();
        _keyLineSaid = player->IsQuestObjectiveComplete(QUEST_SAVING_STELLAGOSA, OBJECTIVE_LYKILLS_KEY);
        // said once, when the chains open; a Kor'vas summoned after that (login, return to Azsuna) stays quiet
        _comeBackSaid = player->GetQuestStatus(QUEST_SAVING_STELLAGOSA) == QUEST_STATUS_COMPLETE
            || player->GetQuestStatus(QUEST_SAVING_STELLAGOSA) == QUEST_STATUS_REWARDED;
        // she is her player's alone (a private object): out of every phase, so she stays visible to that player in the
        // Faronaar areas where phase 4356, which the summon copied from the player at Vanquish Point, is off
        PhasingHandler::ResetPhaseShift(me);
        me->UpdateObjectVisibility();
        // active: her grid keeps updating when her player is far ahead (a flight, a long glide), so the catch-up runs
        me->setActive(true);
        me->SetFaction(player->GetFaction());
        me->SetReactState(REACT_DEFENSIVE);
        Follow(player);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (damage >= me->GetHealth())
            damage = me->GetHealth() - 1;
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        // never back to the summon spot: back to her player
        me->CombatStop(true);
        if (Player* player = ObjectAccessor::GetPlayer(*me, _player))
            Follow(player);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_player.IsEmpty())
            return;

        if (_checkLeft > Milliseconds(diff))
            _checkLeft -= Milliseconds(diff);
        else
        {
            _checkLeft = KorvasCheckInterval;
            if (!CheckPlayer())
                return;
        }

        UpdateVictim();
    }

    bool IsLeaving() const { return _player.IsEmpty(); }

private:
    // false once she is leaving
    bool CheckPlayer()
    {
        Player* player = ObjectAccessor::GetPlayer(*me, _player);
        if (!player || !player->IsInWorld() || player->GetMapId() != me->GetMapId() || !IsFaronaarKorvasWanted(player))
        {
            _player.Clear();
            me->DespawnOrUnsummon(3s);
            return false;
        }

        if (!_keyLineSaid && player->IsQuestObjectiveComplete(QUEST_SAVING_STELLAGOSA, OBJECTIVE_LYKILLS_KEY))
        {
            _keyLineSaid = true;
            FaronaarSay(me, TEXT_KORVAS_GET_THE_KEY, player);
        }

        // the chains are open and Stellagosa flies off; Kor'vas stays for the turn-in
        if (!_comeBackSaid && player->GetQuestStatus(QUEST_SAVING_STELLAGOSA) == QUEST_STATUS_COMPLETE)
        {
            _comeBackSaid = true;
            FaronaarSay(me, TEXT_KORVAS_COME_BACK, player);
        }

        // a fight that has pulled her (or her target) away from her player is dropped; the catch-up below then brings
        // her back
        if (me->IsInCombat())
        {
            Unit* victim = me->GetVictim();
            if (!me->IsWithinDist(player, KorvasLeashDist) || (victim && !victim->IsWithinDist(player, KorvasLeashDist)))
            {
                me->GetThreatManager().ClearFixate();
                EnterEvadeMode(EvadeReason::Other);
            }
        }

        // out of combat she keeps up by running: a sprint after a Fel Rush or a glide, a teleport only when the player is
        // far out of reach
        if (!me->IsInCombat())
        {
            // behind and not getting closer (no path up a ledge, the follow gave up at the end of a partial path) for
            // KorvasStuckChecks checks: the same catch-up as a player out of reach
            float dist = me->GetDistance(player);
            if (dist > KorvasSprintDist && dist > _lastDist - 1.0f)
                ++_stuckChecks;
            else
                _stuckChecks = 0;
            _lastDist = dist;

            if (!me->IsWithinDist(player, KorvasTeleportDist) || _stuckChecks >= KorvasStuckChecks)
            {
                _stuckChecks = 0;
                SetSprinting(false);
                me->NearTeleportTo(player->GetNearPosition(3.0f, float(M_PI)));
                Follow(player);
                return true;
            }

            // a fight that ended on the other side (her target evaded and reset) leaves her chase behind: back to her player
            if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
                Follow(player);

            SetSprinting(!me->IsWithinDist(player, KorvasSprintDist));
        }
        else
            SetSprinting(false);

        // what her player fights, she fights. She is fixated on it, so the threat she holds on something else (the
        // first mob that hit her) can't pull her back to that one through UpdateVictim.
        Unit* target = player->GetVictim();
        if (!target || !me->IsValidAttackTarget(target))
            target = player->getAttackerForHelper();
        if (target && me->IsValidAttackTarget(target) && target->IsWithinDist(player, KorvasLeashDist))
        {
            if (target != me->GetVictim())
                AttackStart(target);
            if (me->GetThreatManager().GetFixateTarget() != target)
                me->GetThreatManager().FixateTarget(target);
        }
        else if (me->GetThreatManager().GetFixateTarget())
            me->GetThreatManager().ClearFixate();

        return true;
    }

    void Follow(Player* player)
    {
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveFollow(player, KorvasFollowDist, KorvasFollowAngle);
    }

    void SetSprinting(bool sprinting)
    {
        if (sprinting == _sprinting)
            return;

        _sprinting = sprinting;
        me->SetSpeedRate(MOVE_RUN, sprinting ? KorvasSprintRate : me->GetCreatureTemplate()->speed_run);
    }

    ObjectGuid _player;
    Milliseconds _checkLeft = KorvasCheckInterval;
    bool _keyLineSaid = false;
    bool _comeBackSaid = false;
    bool _sprinting = false;
    float _lastDist = 0.0f;
    uint32 _stuckChecks = 0;
};

static Creature* FindFaronaarKorvas(Player* player)
{
    auto itr = FaronaarKorvasByPlayer.find(player->GetGUID());
    if (itr == FaronaarKorvasByPlayer.end())
        return nullptr;

    Creature* korvas = ObjectAccessor::GetCreature(*player, itr->second);
    // one that is already leaving (its quest just ended) is not her: a quest accepted in that second gets a new one
    npc_faronaar_korvas const* ai = korvas ? dynamic_cast<npc_faronaar_korvas const*>(korvas->AI()) : nullptr;
    if (!korvas || !korvas->IsAlive() || !ai || ai->IsLeaving())
    {
        FaronaarKorvasByPlayer.erase(itr);
        return nullptr;
    }

    return korvas;
}

static Creature* EnsureFaronaarKorvas(Player* player)
{
    if (!IsFaronaarKorvasWanted(player))
        return nullptr;

    if (Creature* korvas = FindFaronaarKorvas(player))
        return korvas;

    // Standing at one of the static Kor'vas (90255) spawns, the follower takes her place for this player (smooth phasing),
    // so the giver turns and walks along. The static ones are also phased out for this player while the follower is
    // wanted (phase 4356 conditions); the spawn-id store ignores phases, since at quest accept the phase may already
    // have changed.
    for (ObjectGuid::LowType spawnId : FaronaarKorvasGiverSpawns)
    {
        auto bounds = player->GetMap()->GetCreatureBySpawnIdStore().equal_range(spawnId);
        for (auto itr = bounds.first; itr != bounds.second; ++itr)
            if (itr->second->IsAlive() && itr->second->IsWithinDist(player, KorvasGiverHandoffDist))
                return itr->second->SummonPersonalReplacement(NPC_FARONAAR_KORVAS, itr->second->GetPosition(), player,
                    TEMPSUMMON_MANUAL_DESPAWN, 0s, SPELL_FARONAAR_SUMMON_KORVAS);
    }

    return player->SummonCreature(NPC_FARONAAR_KORVAS, player->GetNearPosition(3.0f, float(M_PI)), TEMPSUMMON_MANUAL_DESPAWN, 0s,
        0, SPELL_FARONAAR_SUMMON_KORVAS, player->GetGUID());
}

// Summons Kor'vas when either quest starts or is restored by a login or a return to Azsuna; she removes herself
class player_faronaar_korvas : public PlayerScript
{
public:
    player_faronaar_korvas() : PlayerScript("player_faronaar_korvas") { }

    void OnUpdateZone(Player* player, uint32 /*newZone*/, uint32 /*newArea*/) override
    {
        EnsureFaronaarKorvas(player);
    }

    void OnQuestStatusChange(Player* player, uint32 questId) override
    {
        if (questId != QUEST_FEL_MACHINATIONS && questId != QUEST_SAVING_STELLAGOSA)
            return;

        Creature* korvas = EnsureFaronaarKorvas(player);
        if (korvas && questId == QUEST_SAVING_STELLAGOSA && player->GetQuestStatus(QUEST_SAVING_STELLAGOSA) == QUEST_STATUS_INCOMPLETE
            && !player->IsQuestObjectiveComplete(QUEST_SAVING_STELLAGOSA, OBJECTIVE_LYKILLS_KEY))
            FaronaarSay(korvas, TEXT_KORVAS_FIND_THE_KEY, player);
    }
};

enum FelMachinationsFreed
{
    OBJECTIVE_DEMON_HUNTERS_FREED       = 277252,
    NPC_FEL_MACHINATIONS_HOSTAGE        = 90487,    // the caged one: "Demon Hunters freed" is its kill credit

    // BroadcastText, by rescue number (Warcraft Wiki, Fel Machinations notes); the demon hunters' lines carry both
    // voices, Kor'vas's are her female text
    TEXT_KORVAS_WHAT_HAPPENED           = 92248,    // "Illidari, what happened here?"
    TEXT_KORVAS_ALL_TORTURED            = 92243,    // "They have all been tortured."
    TEXT_FREED_DIDNT_GIVE_IN            = 92938,    // "I didn't give in."
    TEXT_FREED_DID_THINGS               = 92937,    // "They did... things... to me."
    TEXT_FREED_UNIMAGINABLE_POWER       = 92939,    // "The demons promised us unimaginable power."
    TEXT_FREED_NIGHTGLAIVE_WENT         = 92942     // "Nightglaive went southwest with a warden named Cordana Felsong."
};

static constexpr Milliseconds FreedFirstLine = 1s;      // after the use's credit has counted
static constexpr Milliseconds FreedReply = 3s;          // the demon hunter answers Kor'vas
static constexpr Milliseconds FreedDespawn = 8s;        // from the summon, as the SAI it replaces

// 90532, 90534, 90535, 90536 - Demon Hunter (Fel Machinations): the freed hostage (spell_summon_replace puts it in the
// caged one's place for its freer). Rescues 1 and 2 are an exchange with the follower Kor'vas, 3 and 4 a line of its
// own, 5 and 6 are silent; it leaves after 8 s.
struct npc_fel_machinations_freed_demon_hunter : public ScriptedAI
{
    npc_fel_machinations_freed_demon_hunter(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon(FreedDespawn);
            return;
        }

        ObjectGuid playerGuid = player->GetGUID();
        me->SetFacingToObject(player);
        me->DespawnOrUnsummon(FreedDespawn);

        // the rescue counts when the freed one is out: a credit at the use itself would complete the quest on the sixth
        // cage before the cage's spell, whose quest check then stops the sixth rescue
        if (player->GetQuestStatus(QUEST_FEL_MACHINATIONS) == QUEST_STATUS_INCOMPLETE)
            player->KilledMonsterCredit(NPC_FEL_MACHINATIONS_HOSTAGE);

        _scheduler.Schedule(FreedFirstLine, [this, playerGuid](TaskContext context)
        {
            Player* player = ObjectAccessor::GetPlayer(*me, playerGuid);
            if (!player)
                return;

            int32 freed = player->GetQuestObjectiveData(QUEST_FEL_MACHINATIONS, OBJECTIVE_DEMON_HUNTERS_FREED);
            uint32 korvasLine = 0;
            uint32 ownLine = 0;
            switch (freed)
            {
                case 1: korvasLine = TEXT_KORVAS_WHAT_HAPPENED; ownLine = TEXT_FREED_DIDNT_GIVE_IN; break;
                case 2: korvasLine = TEXT_KORVAS_ALL_TORTURED;  ownLine = TEXT_FREED_DID_THINGS; break;
                case 3: ownLine = TEXT_FREED_UNIMAGINABLE_POWER; break;
                case 4: ownLine = TEXT_FREED_NIGHTGLAIVE_WENT; break;
                default: return;
            }

            Creature* korvas = korvasLine ? FindFaronaarKorvas(player) : nullptr;
            if (!korvas)
            {
                FaronaarSay(me, ownLine, player);
                return;
            }

            FaronaarSay(korvas, korvasLine, me);
            context.Schedule(FreedReply, [this, playerGuid, ownLine](TaskContext /*context*/)
            {
                if (Player* player = ObjectAccessor::GetPlayer(*me, playerGuid))
                    FaronaarSay(me, ownLine, player);
            });
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 86535 - Overseer Lykill (Saving Stellagosa): the jailer who carries Lykill's Key
struct npc_overseer_lykill : public ScriptedAI
{
    npc_overseer_lykill(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _scheduler.CancelAll();
    }

    void JustEngagedWith(Unit* who) override
    {
        FaronaarSay(me, TEXT_LYKILL_AGGRO, who);

        _scheduler
            .Schedule(LykillCleaveFirst, [this](TaskContext context)
            {
                DoCastVictim(SPELL_LYKILL_FEL_CLEAVE);
                context.Repeat(LykillCleaveEvery);
            })
            .Schedule(LykillImprisonmentFirst, [this](TaskContext context)
            {
                // a player, never Kor'vas
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 30.0f, true))
                    DoCast(target, SPELL_LYKILL_FEL_IMPRISONMENT);
                context.Repeat(LykillImprisonmentEvery);
            });
    }

    void JustDied(Unit* /*killer*/) override
    {
        FaronaarSay(me, TEXT_LYKILL_DEATH);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 239455 - Fel Lock (Saving Stellagosa): the first lock opened counts the hidden "First chain unlocked" (105635), and
// Kor'vas speaks to the drake
struct go_saving_stellagosa_fel_lock : public GameObjectAI
{
    go_saving_stellagosa_fel_lock(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_SAVING_STELLAGOSA) != QUEST_STATUS_INCOMPLETE
            || !player->IsQuestObjectiveComplete(QUEST_SAVING_STELLAGOSA, OBJECTIVE_LYKILLS_KEY)
            || player->IsQuestObjectiveComplete(QUEST_SAVING_STELLAGOSA, OBJECTIVE_FIRST_CHAIN_UNLOCKED))
            return false;

        TC_LOG_INFO("scripts", "Faronaar: {} opens the first Fel Lock: credit {}", player->GetName(), uint32(NPC_FIRST_CHAIN_UNLOCKED_CREDIT));
        player->KilledMonsterCredit(NPC_FIRST_CHAIN_UNLOCKED_CREDIT);
        if (Creature* korvas = FindFaronaarKorvas(player))
            FaronaarSay(korvas, TEXT_KORVAS_SOON_FREE, player);

        return false;   // the lock opens as usual
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
    RegisterCreatureAI(npc_death_of_the_eldest_senegos);
    RegisterCreatureAI(npc_agapanthus);
    RegisterCreatureAI(npc_their_dying_breaths_whelpling);
    RegisterCreatureAI(npc_farondis_escort);
    RegisterCreatureAI(npc_farondis_tidestone_giver);
    RegisterSpellScript(spell_death_of_the_eldest_toss_crystals);
    RegisterSpellScript(spell_narthalas_wand_practice);
    RegisterAreaTriggerAI(at_narthalas_rune_outline);
    new scene_narthalas_rune_drawing();
    RegisterGameObjectAI(go_sythorne_podium);
    new scene_study_hall_into_the_book();
    RegisterCreatureAI(npc_faronaar_korvas);
    RegisterCreatureAI(npc_fel_machinations_freed_demon_hunter);
    new player_faronaar_korvas();
    RegisterCreatureAI(npc_overseer_lykill);
    RegisterGameObjectAI(go_saving_stellagosa_fel_lock);
}
