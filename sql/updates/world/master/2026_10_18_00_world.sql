-- Nythendra: root wall across the entrance, closed while the encounter is in progress
DELETE FROM `gameobject` WHERE `guid` = 1520001;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(1520001, 254985, 1520, 0, 0, '14', 0, 0, 0, -1, 1842.1722, 1387.2362, 369.9020, 2.0471, 0, 0, 0.853950, 0.520356, 7200, 255, 0, '', NULL, 0);
