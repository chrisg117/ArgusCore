-- The temple's mission table. Its model, the priest's "mission manager" (7af_priest_missionmanager01.m2, the
-- client's display 40446), has no object in the database, so it is given one; it stands in the main circle,
-- where and facing as placed by hand in the game. Over it, the Command Map (102589), which the client's data
-- has for the priest's order hall, opens the class hall missions: it is not seen, only clicked, as in a
-- screenshot of the table on Wowhead. Of its two looks it keeps the one that can be pointed at.
DELETE FROM `gameobject_template` WHERE `entry`=1512001;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `VerifiedBuild`) VALUES
(1512001, 5, 40446, 'Command Map', 1, 0);

-- flags: GO_FLAG_NOT_SELECTABLE and GO_FLAG_NODESPAWN, so that pointing at the table finds the Command Map
DELETE FROM `gameobject_template_addon` WHERE `entry`=1512001;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`) VALUES
(1512001, 0, 0x30);

DELETE FROM `gameobject` WHERE `guid`=15120003;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15120003, 1512001, 1512, 0, 0, '0', 0, 0, 0, -1, 1334.86, 1344.27, 177.138, 3.01315, 0, 0, 0.997941, 0.064138, 300, 255, 1, '', NULL, 0); -- Command Map

UPDATE `creature_template` SET `npcflag`=137438953472 WHERE `entry`=102589;

DELETE FROM `creature` WHERE `guid`=15120004;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15120004, 102589, 1512, 0, 0, '0', 0, 0, 0, -1, 64062, 0, 1334.86, 1344.27, 177.138, 3.01315, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Command Map

-- Saa'ra floats over the Sanctuary of the Light rather than the main circle, lower than before and turned
-- a quarter, as placed by hand in the game
UPDATE `creature` SET `position_x`=1355.86, `position_y`=1221.69, `position_z`=183.6, `orientation`=4.71 WHERE `guid`=15120003;
