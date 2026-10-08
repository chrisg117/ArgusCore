-- Broken Shore scenario (map 1460), stage "Defeat the Commander", Alliance side.
-- Dread Commander Arganoth stands at the average of his Wowhead map pins, converted to map 1460 like the
-- spawns of "Storm the Beach", facing the Alliance landing point. The position is approximate.

-- The group is spawned by the instance script when the stage starts
DELETE FROM `spawn_group_template` WHERE `groupId`=14602;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14602, 'Broken Shore Scenario - Defeat the Commander (Alliance)', 4);

DELETE FROM `creature` WHERE `guid`=14600200;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600200, 90705, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 506.10, 2080.80, 0.83, 3.2164, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- Dread Commander Arganoth

DELETE FROM `spawn_group` WHERE `groupId`=14602;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14602, 0, 14600200);
