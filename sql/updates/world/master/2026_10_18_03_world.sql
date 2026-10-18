-- Nythendra: root wall on the way to Il'gynoth, closed until she is defeated
DELETE FROM `gameobject` WHERE `guid` = 1520002;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(1520002, 254987, 1520, 0, 0, '14', 0, 0, 0, -1, 1906.6488, 1261.2933, 369.5014, 5.1808, 0, 0, 0.523712, -0.851895, 7200, 255, 1, '', NULL, 0);

-- Not selectable, never despawns (48 = NOT_SELECTABLE | NODESPAWN)
DELETE FROM `gameobject_template_addon` WHERE `entry` = 254987;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`, `mingold`, `maxgold`, `artkit0`, `artkit1`, `artkit2`, `artkit3`, `artkit4`, `WorldEffectID`, `AIAnimKitID`) VALUES
(254987, 114, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0);
