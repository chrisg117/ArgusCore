-- Broken Shore scenario (map 1460), stage "Find Varian", Alliance side.
-- King Varian Wrynn stands at the Wowhead map pin he shares with his knights, tinkerers, mages and priests,
-- beside the Alliance "Portal" graveyard, converted to map 1460 like the spawns of the earlier stages and
-- facing the hill that players come over. The position is approximate.

-- The group is spawned by the instance script when the stage starts
DELETE FROM `spawn_group_template` WHERE `groupId`=14603;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14603, 'Broken Shore Scenario - Find Varian (Alliance)', 4);

DELETE FROM `creature` WHERE `guid`=14600300;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600300, 90713, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1123.40, 2487.00, 39.10, 3.9650, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- King Varian Wrynn

DELETE FROM `spawn_group` WHERE `groupId`=14603;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14603, 0, 14600300);
