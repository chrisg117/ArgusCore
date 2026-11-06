-- DK campaign: The Call of Icecrown, scenario 901 step 1811 (Task 3c). The map draws no pad at the Spire's Scourge
-- Teleporter (user, pass 5), so the Legion copy of the transporter model stands there: 251471, type 5 (generic), decoration
-- only, at 631's size (1.19); at_icc_scenario_spire_teleporter (2026_11_06_01) does the work. Position and facing are 631's Frozen Throne
-- transporter (GO 202223, guid 150338), the same spot on 1480, where the areatrigger is centred
DELETE FROM `gameobject` WHERE `guid`=14800006;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14800006, 251471, 1480, 0, 0, '12', 0, 0, 0, -1, 4356.93, 2769.41, 355.955, -2.35619, 0, 0, -0.9238795, 0.3826834, 86400, 255, 1, '', NULL, 0);
