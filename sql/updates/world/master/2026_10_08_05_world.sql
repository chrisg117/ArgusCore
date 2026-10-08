-- Broken Shore scenario (map 1460), stage "Krosus", Alliance side.
-- Krosus rises from the fel pool below the ledge that players fight him from, and faces it. He stands at the
-- centre of his Alliance-side Wowhead map pins converted to map 1460, which is approximate.
-- His height is not the pool's floor: the origin of his model is at his waist, with as much of him below it as
-- above it (43 yards each way), so he is placed at the surface of the fel, which the map has at 31.9. That
-- leaves him waist-deep in it, 14 yards out from the ledge and 6 yards below it, well inside his reach of 30.

-- The group is spawned by the instance script when the stage starts
DELETE FROM `spawn_group_template` WHERE `groupId`=14607;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14607, 'Broken Shore Scenario - Krosus (Alliance)', 4);

DELETE FROM `creature` WHERE `guid`=14600700;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600700, 90544, 1460, 7534, 8120, '12', 0, 0, 0, -1, 0, 0, 1495.60, 1782.40, 31.90, 1.4822, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- Krosus

-- CREATURE_STATIC_FLAG_SESSILE and CREATURE_STATIC_FLAG_FLOATING: he fights from where he stands instead of
-- going to his target, and stays at the surface instead of sinking to the floor of the pool
DELETE FROM `creature_static_flags_override` WHERE `SpawnId`=14600700;
INSERT INTO `creature_static_flags_override` (`SpawnId`, `DifficultyId`, `StaticFlags1`) VALUES
(14600700, 12, 0x20000100);

DELETE FROM `spawn_group` WHERE `groupId`=14607;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14607, 0, 14600700);
