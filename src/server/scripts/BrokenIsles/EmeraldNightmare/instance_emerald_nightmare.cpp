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
#include "InstanceScript.h"
#include "ScriptMgr.h"
#include "emerald_nightmare.h"

// Nythendra: the room is a circle fitted through three points on its edge
// (measured in game, 2026-10-17). The check is 3D, so the centre needs the
// floor height. The radius is widened past the fitted 55 yd to 90 yd so it
// reaches the entrance, where the root wall stands (1842.2, 1387.2).
BossBoundaryData const boundaries =
{
    { DATA_NYTHENDRA, new CircleBoundary(Position(1881.5f, 1311.0f, 369.5f), 90.0f) },
    // Il'gynoth: the four room-edge readings fit no single circle (triples of
    // them give 65-87 yd), so the centre is their centroid and the radius reaches
    // the farthest reading (88.4 yd) plus a margin. The check is 3D, so the
    // centre carries the floor height.
    { DATA_ILGYNOTH, new CircleBoundary(Position(-12837.2f, 12314.2f, -248.0f), 95.0f) }
};

// A root wall grows across the entrance while Nythendra is fought; the one
// on the way to Il'gynoth stands until she is dead.
DoorData const doorData[] =
{
    { GO_NYTHENDRA_ROOT_WALL,       DATA_NYTHENDRA, EncounterDoorBehavior::OpenWhenNotInProgress },
    { GO_NYTHENDRA_EXIT_ROOT_WALL,  DATA_NYTHENDRA, EncounterDoorBehavior::OpenWhenDone          },
    { 0,                            0,              EncounterDoorBehavior::OpenWhenNotInProgress }  // END
};

// The four Dragons of Nightmare are left out: how they are tracked is decided
// with that encounter's script.
ObjectData const creatureData[] =
{
    { BOSS_NYTHENDRA,           DATA_NYTHENDRA          },
    { BOSS_ILGYNOTH,            DATA_ILGYNOTH           },
    { BOSS_ELERETHE_RENFERAL,   DATA_ELERETHE_RENFERAL  },
    { BOSS_URSOC,               DATA_URSOC              },
    { BOSS_CENARIUS,            DATA_CENARIUS           },
    { BOSS_XAVIUS,              DATA_XAVIUS             },
    { NPC_EYE_OF_ILGYNOTH,      DATA_EYE_OF_ILGYNOTH    },
    { 0,                        0                       }  // END
};

ObjectData const gameObjectData[] =
{
    { GO_EYE_OF_ILGYNOTH_FORCEFIELD,    DATA_ILGYNOTH_FORCEFIELD    },
    { 0,                                0                           }  // END
};

DungeonEncounterData const encounters[] =
{
    { DATA_NYTHENDRA,               {{ 1853 }} },
    { DATA_ILGYNOTH,                {{ 1873 }} },
    { DATA_ELERETHE_RENFERAL,       {{ 1876 }} },
    { DATA_URSOC,                   {{ 1841 }} },
    { DATA_DRAGONS_OF_NIGHTMARE,    {{ 1854 }} },
    { DATA_CENARIUS,                {{ 1877 }} },
    { DATA_XAVIUS,                  {{ 1864 }} },
};

class instance_emerald_nightmare : public InstanceMapScript
{
    public:
        instance_emerald_nightmare() : InstanceMapScript(ENScriptName, 1520) { }

        struct instance_emerald_nightmare_InstanceMapScript: public InstanceScript
        {
            instance_emerald_nightmare_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
                SetBossNumber(EncounterCount);
                LoadObjectData(creatureData, gameObjectData);
                LoadDoorData(doorData);
                LoadBossBoundaries(boundaries);
                LoadDungeonEncounterData(encounters);
            }

            void OnGameObjectCreate(GameObject* go) override
            {
                InstanceScript::OnGameObjectCreate(go);

                // Once Il'gynoth is dead the hollow stays open.
                if (go->GetEntry() == GO_EYE_OF_ILGYNOTH_FORCEFIELD && GetBossState(DATA_ILGYNOTH) == DONE)
                    HandleGameObject(ObjectGuid::Empty, true, go);
            }
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const override
        {
            return new instance_emerald_nightmare_InstanceMapScript(map);
        }
};

void AddSC_instance_emerald_nightmare()
{
    new instance_emerald_nightmare();
}
