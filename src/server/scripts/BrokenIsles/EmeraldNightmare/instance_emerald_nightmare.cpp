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
#include <array>
#include <vector>

// Elerethe Renferal: fitted 2026-10-20 to a trail walked as a GM (one closed loop per
// platform and per bridge web, 3 yd margin; fitter: ~/emerald-nightmare/fit_boundary.py).
// A union of seven pieces, each a polygon (vertices relative to its origin) cut to a z band:
// the band is the loop's lowest z - 10 up to its highest z + 15, and + 25 on the bridges so
// the flight over a web (20 yd up) stays inside. The five loops are
// convex hulls of the walked points (the loops overlapped themselves). Two more pieces are
// 20 yd wide strips along walked paths: the alternate way P2 to P3 and the bridge P3 to P1
// (three readings each, ends extended 5 yd into the platforms, z band min - 10 .. max + 15).
namespace
{
    struct BoundaryPiece
    {
        float originX, originY;
        float minZ, maxZ;
        std::vector<Position> vertices;
    };
}

static AreaBoundary const* MakeElereteBoundary()
{
    static std::array<BoundaryPiece, 7> const pieces =
    {{
        // P1, 21 vertices
        { 11405.62f, 11389.69f, -100.21f, -66.70f,
          { Position(-59.44f, -3.99f), Position(-59.18f, -12.96f), Position(-51.45f, -26.41f),
            Position(-46.62f, -32.91f), Position(-5.03f, -35.08f), Position(1.74f, -35.30f),
            Position(8.85f, -35.22f), Position(15.11f, -34.96f), Position(23.71f, -32.04f),
            Position(28.98f, -28.15f), Position(36.04f, -22.58f), Position(58.48f, 4.40f),
            Position(62.35f, 9.59f), Position(65.73f, 17.74f), Position(17.51f, 46.84f),
            Position(11.08f, 48.09f), Position(5.05f, 48.86f), Position(-3.31f, 48.66f),
            Position(-31.33f, 29.35f), Position(-36.81f, 25.03f), Position(-41.44f, 21.04f) } },
        // P2, 22 vertices
        { 11375.87f, 11227.80f, -111.99f, -82.34f,
          { Position(-50.58f, 5.36f), Position(-49.53f, -4.73f), Position(-38.15f, -28.45f),
            Position(-35.12f, -34.76f), Position(-31.80f, -41.66f), Position(-12.05f, -57.94f),
            Position(1.95f, -49.78f), Position(11.69f, -44.08f), Position(33.51f, -27.17f),
            Position(36.88f, -20.15f), Position(40.47f, -12.65f), Position(44.79f, 8.11f),
            Position(45.76f, 14.22f), Position(45.87f, 30.48f), Position(41.48f, 36.49f),
            Position(32.87f, 41.44f), Position(27.02f, 44.45f), Position(13.10f, 45.75f),
            Position(-30.58f, 32.01f), Position(-35.10f, 28.54f), Position(-42.83f, 22.26f),
            Position(-49.66f, 12.28f) } },
        // P3, 19 vertices
        { 11532.66f, 11289.11f, -103.87f, -61.63f,
          { Position(-46.29f, 1.68f), Position(-43.72f, -34.06f), Position(-41.30f, -38.47f),
            Position(-33.33f, -45.03f), Position(-24.12f, -50.38f), Position(-17.24f, -53.35f),
            Position(-2.89f, -56.37f), Position(5.60f, -56.10f), Position(11.39f, -51.60f),
            Position(53.49f, -3.08f), Position(58.06f, 4.88f), Position(56.21f, 11.87f),
            Position(44.43f, 40.66f), Position(38.92f, 46.97f), Position(26.79f, 53.54f),
            Position(18.80f, 55.74f), Position(-27.09f, 62.47f), Position(-35.88f, 59.11f),
            Position(-41.82f, 51.51f) } },
        // B12, 11 vertices
        { 11362.71f, 11308.93f, -112.93f, -65.86f,
          { Position(-21.63f, -46.95f), Position(-19.30f, -54.43f), Position(-11.77f, -56.70f),
            Position(-5.00f, -54.78f), Position(15.70f, -7.51f), Position(21.38f, 5.52f),
            Position(22.92f, 11.17f), Position(7.87f, 48.85f), Position(2.88f, 50.65f),
            Position(-2.92f, 51.73f), Position(-10.11f, 52.46f) } },
        // B23, 8 vertices
        { 11454.98f, 11237.07f, -110.70f, -65.97f,
          { Position(-40.31f, 5.11f), Position(-40.30f, -7.01f), Position(-9.77f, -15.68f),
            Position(-2.52f, -17.42f), Position(5.97f, -17.24f), Position(10.61f, -13.12f),
            Position(36.40f, 24.36f), Position(39.93f, 41.00f) } },
        // S23, 6 vertices
        { 11451.25f, 11262.16f, -109.65f, -78.00f,
          { Position(-39.77f, 0.98f), Position(-5.36f, 8.64f), Position(38.23f, 19.57f),
            Position(43.09f, 0.17f), Position(-0.75f, -10.82f), Position(-35.43f, -18.54f) } },
        // S31, 6 vertices
        { 11478.71f, 11365.47f, -104.10f, -72.30f,
          { Position(11.98f, -31.18f), Position(-5.03f, -8.51f), Position(-30.33f, 20.90f),
            Position(-15.16f, 33.94f), Position(10.57f, 4.03f), Position(27.98f, -19.17f) } }
    }};

    AreaBoundary const* result = nullptr;
    for (BoundaryPiece const& piece : pieces)
    {
        AreaBoundary const* part = new BoundaryIntersectionBoundary(
            new PolygonBoundary(Position(piece.originX, piece.originY), std::vector<Position>(piece.vertices)),
            new ZRangeBoundary(piece.minZ, piece.maxZ));
        result = result ? new BoundaryUnionBoundary(result, part) : part;
    }

    return result;
}

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
    { DATA_ILGYNOTH, new CircleBoundary(Position(-12837.2f, 12314.2f, -248.0f), 95.0f) },
    { DATA_ELERETHE_RENFERAL, MakeElereteBoundary() }
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
