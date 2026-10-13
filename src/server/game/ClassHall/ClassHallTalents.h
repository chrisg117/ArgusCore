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

#ifndef ClassHallTalents_h__
#define ClassHallTalents_h__

#include "Define.h"
#include "DatabaseEnvFwd.h"
#include "ObjectGuid.h"
#include <ctime>
#include <map>

class Player;
struct GarrTalentEntry;

namespace WorldPackets
{
    namespace Garrison
    {
        struct GarrisonInfo;
    }
}

// The talents (hall upgrades) a character has chosen in the research tree of its class hall.
class TC_GAME_API ClassHallTalents
{
public:
    explicit ClassHallTalents(Player* owner);

    void LoadFromDB(PreparedQueryResult talents);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

private:
    struct Talent
    {
        GarrTalentEntry const* Entry = nullptr;
        time_t ResearchStartTime = 0;
        uint32 Flags = 0;
    };

    Player* _owner;
    std::map<uint32 /*garrTalentId*/, Talent> _talents;
};

#endif // ClassHallTalents_h__
