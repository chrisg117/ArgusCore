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

#include "AreaBoundary.h"
#include "Creature.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Map.h"
#include "Player.h"
#include "QuestDef.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "TemporarySummon.h"
#include "eye_of_azshara.h"
#include <algorithm>

// One boundary per boss, added by each boss's task. The bosses stand in the open, so each is a circle
// around the spawn (the check is 3D, so the centre carries the ground height).
BossBoundaryData const boundaries =
{
    // Warlord Parjesh: calibrate the radius (no walls; the path inland runs through his spot).
    { DATA_WARLORD_PARJESH, new CircleBoundary(Position(-3682.65f, 4417.02f, 32.44f), 60.0f) },
    // Lady Hatecoil: the Sand Dune box (x -3479..-3402, y 4552..4620) plus 25 yd. Calibrate.
    { DATA_LADY_HATECOIL, new RectangleBoundary(-3504.0f, -3377.0f, 4527.0f, 4645.0f) },
    // King Deepbeard: the box his 35 Call the Seas stalkers cover (x -3491..-3409, y 4119..4214) plus 10 yd;
    // the two Water Doors sit on its south and east edges. Calibrate.
    { DATA_KING_DEEPBEARD, new RectangleBoundary(-3501.0f, -3399.0f, 4109.0f, 4224.0f) },
    // Serpentrix: around his spawn, wide enough for the six Sinkholes he re-emerges at (47-80 yd out) and a
    // group in reach of them. Calibrate.
    { DATA_SERPENTRIX, new CircleBoundary(Position(-3250.7f, 4429.4f, -2.8f), 110.0f) }
};

ObjectData const creatureData[] =
{
    { BOSS_WARLORD_PARJESH,     DATA_WARLORD_PARJESH    },
    { BOSS_LADY_HATECOIL,       DATA_LADY_HATECOIL      },
    { BOSS_KING_DEEPBEARD,      DATA_KING_DEEPBEARD     },
    { BOSS_SERPENTRIX,          DATA_SERPENTRIX         },
    { BOSS_WRATH_OF_AZSHARA,    DATA_WRATH_OF_AZSHARA   },
    { 0,                        0                       }  // END
};

ObjectData const gameObjectData[] =
{
    { GO_WRATH_BUBBLE,          DATA_WRATH_BUBBLE       },
    { 0,                        0                       }  // END
};

// The two Water Doors stand on the edge of King Deepbeard's arena (by position; the
// scaffold's in-game check confirms they bound it). Resting state open.
DoorData const doorData[] =
{
    { GO_DEEPBEARD_DOOR_1,  DATA_KING_DEEPBEARD,    EncounterDoorBehavior::OpenWhenNotInProgress },
    { GO_DEEPBEARD_DOOR_2,  DATA_KING_DEEPBEARD,    EncounterDoorBehavior::OpenWhenNotInProgress },
    { 0,                    0,                      EncounterDoorBehavior::OpenWhenNotInProgress }  // END
};

DungeonEncounterData const encounters[] =
{
    { DATA_WARLORD_PARJESH,     {{ 1810 }} },
    { DATA_LADY_HATECOIL,       {{ 1811 }} },
    { DATA_KING_DEEPBEARD,      {{ 1812 }} },
    { DATA_SERPENTRIX,          {{ 1813 }} },
    { DATA_WRATH_OF_AZSHARA,    {{ 1814 }} }
};

namespace
{
    uint32 constexpr QUEST_WRATH_OF_AZSHARA = 38286;
    uint32 constexpr NPC_EYE_OF_AZSHARA_ENTERED_CREDIT = 106847;
    uint32 constexpr NPC_TIDESTONE_OF_GOLGANNETH = 106780;
    Position const TidestonePosition = { -3472.50f, 4366.40f, 2.69f, 0.0f };

    uint32 constexpr ArcanistCount = 4;
    uint32 constexpr RitualCasterCount = 4;
    uint32 constexpr ArcaneShieldingMaxStacks = 3;  // SpellAuraOptions of 197868

    bool IsRitualCaster(uint32 entry)
    {
        switch (entry)
        {
            case NPC_MYSTIC_SSAVEH:
            case NPC_RITUALIST_LESHA:
            case NPC_CHANNELER_VARISZ:
            case NPC_BINDER_ASHIOI:
                return true;
            default:
                return false;
        }
    }
}

class instance_eye_of_azshara : public InstanceMapScript
{
    public:
        instance_eye_of_azshara() : InstanceMapScript(EOAScriptName, 1456) { }

        struct instance_eye_of_azshara_InstanceMapScript : public InstanceScript
        {
            instance_eye_of_azshara_InstanceMapScript(InstanceMap* map) : InstanceScript(map),
                _arcanistsDead(*this, "ArcanistsDead", 0),
                _ritualCastersDead(*this, "RitualCastersDead", 0)
            {
                SetHeaders(DataHeader);
                SetBossNumber(EncounterCount);
                LoadObjectData(creatureData, gameObjectData);
                LoadDoorData(doorData);
                LoadBossBoundaries(boundaries);
                LoadDungeonEncounterData(encounters);
            }

            void OnCreatureCreate(Creature* creature) override
            {
                InstanceScript::OnCreatureCreate(creature);

                switch (creature->GetEntry())
                {
                    case BOSS_LADY_HATECOIL:
                        UpdateArcaneShielding(creature);
                        break;
                    case BOSS_WRATH_OF_AZSHARA:
                        SetWrathAttackable(creature, IsWrathUnlocked());
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* go) override
            {
                InstanceScript::OnGameObjectCreate(go);

                // The Bubble stands while the ritual holds.
                if (go->GetEntry() == GO_WRATH_BUBBLE && IsWrathUnlocked())
                    go->DespawnOrUnsummon();
            }

            // The Arcanists and ritual casters are Enemy Forces (template H1): they die to
            // players and are never despawned by a script, so a death is the only count.
            void OnUnitDeath(Unit* unit) override
            {
                Creature* creature = unit->ToCreature();
                if (!creature)
                    return;

                if (creature->GetEntry() == NPC_HATECOIL_ARCANIST)
                {
                    if (_arcanistsDead < ArcanistCount)
                        _arcanistsDead = _arcanistsDead + 1;
                    if (Creature* hatecoil = GetCreature(DATA_LADY_HATECOIL))
                        UpdateArcaneShielding(hatecoil);
                }
                else if (IsRitualCaster(creature->GetEntry()))
                {
                    if (_ritualCastersDead < RitualCasterCount)
                        _ritualCastersDead = _ritualCastersDead + 1;
                    CheckWrath();
                }
            }

            bool SetBossState(uint32 type, EncounterState state) override
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                if (state == DONE && type < DATA_WRATH_OF_AZSHARA)
                    CheckWrath();

                if (state == DONE && type == DATA_WRATH_OF_AZSHARA)
                    SummonTidestone();

                return true;
            }

            // Quest 38286 "Eye of Azshara: Wrath of Azshara": its first objective is the credit 106847 ("Eye of
            // Azshara entered"); the quest ends at the Tidestone of Golganneth, who appears once the Wrath is dead.
            void OnPlayerEnter(Player* player) override
            {
                if (player->GetQuestStatus(QUEST_WRATH_OF_AZSHARA) == QUEST_STATUS_INCOMPLETE)
                    player->KilledMonsterCredit(NPC_EYE_OF_AZSHARA_ENTERED_CREDIT);

                // Intro (RP task): Parjesh's "Hmph. Intruders." (101992) when the first player lands. Gates nothing.
                if (!_introLanded && !player->IsGameMaster() && GetBossState(DATA_WARLORD_PARJESH) != DONE)
                {
                    _introLanded = true;
                    if (Creature* parjesh = GetCreature(DATA_WARLORD_PARJESH))
                        if (parjesh->IsAIEnabled())
                            parjesh->AI()->Talk(10);
                }

                if (GetBossState(DATA_WRATH_OF_AZSHARA) == DONE)
                    SummonTidestone();
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_ARCANISTS:
                        return _arcanistsDead;
                    case DATA_RITUAL:
                        return _ritualCastersDead;
                    default:
                        return 0;
                }
            }

            void SetData(uint32 type, uint32 /*data*/) override
            {
                switch (type)
                {
                    case DATA_ARCANISTS:
                        // Lady Hatecoil asks for her shield again when she respawns after a wipe (the
                        // creature addon gives back one stack); the count itself only changes on a death.
                        if (Creature* hatecoil = GetCreature(DATA_LADY_HATECOIL))
                            UpdateArcaneShielding(hatecoil);
                        break;
                    case DATA_KEYSTONE_START:
                        // Template H3, the spec's "Keystone start" list: the Mythic+ conversion calls this after
                        // its respawn. Parjesh's landing line is skipped, the two counters start at 0 (Hatecoil
                        // shielded, the Wrath sealed), and no door needs opening.
                        _introLanded = true;
                        _arcanistsDead = 0;
                        _ritualCastersDead = 0;
                        if (Creature* hatecoil = GetCreature(DATA_LADY_HATECOIL))
                            UpdateArcaneShielding(hatecoil);
                        if (Creature* wrath = GetCreature(DATA_WRATH_OF_AZSHARA))
                            SetWrathAttackable(wrath, IsWrathUnlocked());
                        break;
                    default:
                        break;
                }
            }

            void AfterDataLoad() override
            {
                // A saved instance whose ritual was broken has no Bubble to come back to.
                CheckWrath();
            }

        private:
            // 197868 lowers damage taken by 90% per stack, up to 3 stacks; each living Arcanist
            // holds one up. It goes with the last of them.
            void UpdateArcaneShielding(Creature* hatecoil)
            {
                uint32 const alive = ArcanistCount - std::min<uint32>(_arcanistsDead, ArcanistCount);
                uint32 const stacks = std::min(alive, ArcaneShieldingMaxStacks);
                if (!stacks)
                {
                    hatecoil->RemoveAurasDueToSpell(SPELL_ARCANE_SHIELDING);
                    return;
                }

                hatecoil->SetAuraStack(SPELL_ARCANE_SHIELDING, hatecoil, stacks);
            }

            bool IsWrathUnlocked() const
            {
                for (uint32 boss = DATA_WARLORD_PARJESH; boss < DATA_WRATH_OF_AZSHARA; ++boss)
                    if (GetBossState(boss) != DONE)
                        return false;

                return _ritualCastersDead >= RitualCasterCount;
            }

            void SetWrathAttackable(Creature* wrath, bool attackable)
            {
                // Visible and selectable, but no one can attack her.
                if (attackable)
                    wrath->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                else
                    wrath->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                wrath->SetImmuneToPC(!attackable);
                wrath->SetReactState(attackable ? REACT_AGGRESSIVE : REACT_PASSIVE);
            }

            void CheckWrath()
            {
                if (!IsWrathUnlocked())
                    return;

                if (Creature* wrath = GetCreature(DATA_WRATH_OF_AZSHARA))
                    if (wrath->IsAlive() && wrath->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
                    {
                        SetWrathAttackable(wrath, true);
                        // "THE STORM AWAKENS." (99950, her text group 0)
                        if (wrath->IsAIEnabled())
                            wrath->AI()->Talk(0);
                    }

                if (GameObject* bubble = GetGameObject(DATA_WRATH_BUBBLE))
                    bubble->DespawnOrUnsummon();
            }

            void SummonTidestone()
            {
                if (!_tidestone.IsEmpty() && instance->GetCreature(_tidestone))
                    return;

                // The quest's turn-in point of interest (-3474, 4369) beside the Wrath's spot, moved 3 yd to stand
                // 4.5 yd clear of the nearest wall (navplace on the 1456 mesh, 2026-11-26).
                if (TempSummon* tidestone = instance->SummonCreature(NPC_TIDESTONE_OF_GOLGANNETH, TidestonePosition))
                    _tidestone = tidestone->GetGUID();
            }

            ObjectGuid _tidestone;
            bool _introLanded = false;
            PersistentInstanceScriptValue<uint32> _arcanistsDead;
            PersistentInstanceScriptValue<uint32> _ritualCastersDead;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const override
        {
            return new instance_eye_of_azshara_InstanceMapScript(map);
        }
};

void AddSC_instance_eye_of_azshara()
{
    new instance_eye_of_azshara();
}
