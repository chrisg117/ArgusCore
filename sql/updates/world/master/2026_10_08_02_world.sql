-- Broken Shore scenario (map 1460), stage "The Highlord", Alliance side.
-- Highlord Tirion Fordring hangs in the air over the fel pool that Krosus rises from. Wowhead has no map pin for
-- him, so he is placed over Krosus's pins at the height of the ledge that King Varian's pins are on, facing that
-- ledge, which is where players arrive. The position is approximate.

-- The group is spawned by the instance script when the stage starts
DELETE FROM `spawn_group_template` WHERE `groupId`=14606;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14606, 'Broken Shore Scenario - The Highlord (Alliance)', 4);

DELETE FROM `creature` WHERE `guid`=14600600;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600600, 91951, 1460, 7534, 8120, '12', 0, 0, 0, -1, 0, 0, 1495.60, 1782.40, 40.00, 1.4822, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- Highlord Tirion Fordring

-- CREATURE_STATIC_FLAG_FLOATING: he stays in the air instead of falling into the pool
DELETE FROM `creature_static_flags_override` WHERE `SpawnId`=14600600;
INSERT INTO `creature_static_flags_override` (`SpawnId`, `DifficultyId`, `StaticFlags1`) VALUES
(14600600, 12, 0x20000000);

DELETE FROM `spawn_group` WHERE `groupId`=14606;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14606, 0, 14600600);
