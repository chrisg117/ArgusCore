-- The temple's mission table: the Command Map (102589), which the client's data has for the priest's order
-- hall, opens the class hall missions. Its looks are invisible ones and the temple has no table of its own to
-- stand on, so it is a place in the main circle that is clicked; where, and which way it faces, was chosen by
-- hand in the game.
UPDATE `creature_template` SET `npcflag`=137438953472 WHERE `entry`=102589;

DELETE FROM `creature` WHERE `guid`=15120004;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15120004, 102589, 1512, 0, 0, '0', 0, 0, 0, -1, 0, 0, 1334.86, 1344.27, 177.138, 3.01315, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Command Map

-- Saa'ra floats over the Sanctuary of the Light rather than the main circle, lower than before and turned
-- a quarter, as placed by hand in the game
UPDATE `creature` SET `position_x`=1355.86, `position_y`=1221.69, `position_z`=183.6, `orientation`=4.71 WHERE `guid`=15120003;
