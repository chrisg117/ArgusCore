-- Broken Shore scenario (map 1460), stage "Stop Gul'dan", Alliance side.
-- Gul'dan stands on the path before the Tomb of Sargeras. Wowhead's map pins for him are on another map, so his
-- position was taken in game, at the spot where he stands in a video of the retail scenario. He faces the
-- platform that the Alliance fights on and cannot be attacked.
-- Mo'arg Spinebreaker, whose death ends the stage, stands on that platform at the centre of its Wowhead map pins
-- converted to map 1460, facing the graveyard that players come from. Its position is approximate.

-- The group is spawned by the instance script when the stage starts
DELETE FROM `spawn_group_template` WHERE `groupId`=14608;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14608, 'Broken Shore Scenario - Stop Gul''dan (Alliance)', 4);

-- Gul'dan's unit_flags: UNIT_FLAG_IMMUNE_TO_PC and UNIT_FLAG_IMMUNE_TO_NPC
DELETE FROM `creature` WHERE `guid` IN (14600800, 14600801);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600800, 94276, 1460, 7534, 7624, '12', 0, 0, 0, -1, 0, 0, 1658.38, 1660.22, 78.79, 2.2178, 7200, 0, 0, 100, 0, NULL, 0x300, NULL, NULL, 0), -- Gul'dan
(14600801, 105205, 1460, 7534, 7624, '12', 0, 0, 0, -1, 0, 0, 1625.20, 1703.50, 77.52, 2.8546, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- Mo'arg Spinebreaker

DELETE FROM `spawn_group` WHERE `groupId`=14608;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14608, 0, 14600800),
(14608, 0, 14600801);
