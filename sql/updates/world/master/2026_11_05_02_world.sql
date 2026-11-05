-- DK campaign: The Call of Icecrown, scenario 901 step 1810, the gargoyle gate before Baelgun (Task 3b-2a).
-- Design: dk-frost-task3b2-design.md. From the user's retail recording (5:40 to 6:25): two Stoneskin Gatekeepers lift off
-- their pressure plates as a death knight comes near (emote 99548) and the portcullis behind them slams shut. They are
-- Hardened (192188) and damage cannot kill them; pulled back onto its own plate, each dies, and both dead opens the way.
-- The plates and the portcullis line are where the user stood on 1480 (.gps, 2026-11-05); the plate is each
-- Gatekeeper's spawn point. No plate visual yet (a gap)

UPDATE `creature_template` SET `faction`=14, `ScriptName`='npc_icc_scenario_stoneskin_gatekeeper' WHERE `entry`=97320;
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `Entry`=97320 AND `DifficultyID`=0;

DELETE FROM `creature` WHERE `guid` BETWEEN 14800005 AND 14800006;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14800005, 97320, 1480, 0, 0, '12', 0, 0, 0, 0, 4221.56, 2761.36, 350.96, 1.5708, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Stoneskin Gatekeeper, south plate
(14800006, 97320, 1480, 0, 0, '12', 0, 0, 0, 0, 4221.47, 2777.25, 350.96, 4.7124, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Stoneskin Gatekeeper, north plate

-- The portcullis between the gargoyles and Baelgun, open until they lift off
DELETE FROM `gameobject` WHERE `guid`=14800005;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14800005, 251449, 1480, 0, 0, '12', 0, 0, 0, -1, 4236.00, 2769.54, 350.96, 3.14159, 0, 0, 1, 0, 86400, 255, 0, '', NULL, 0);

-- The approach: a sphere between the two plates, so the way in from the transporter and the walkway from the north both cross it
DELETE FROM `areatrigger_template` WHERE `Id`=14800003 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(14800003, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=14800003 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(14800003, 1, 14800003, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 12, 12, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId`=14800003;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(14800003, 14800003, 1, 1480, '12', 4221.50, 2769.30, 350.96, 0, 0, 0, 0, 'at_icc_scenario_gargoyle_gate', 'Icecrown Citadel (DK Frost scenario) - the corridor before the gargoyle gate: wake the Gatekeepers', 0);

-- The plates themselves: a frosty ground circle on each Gatekeeper's spawn point, so a death knight can see where to
-- bring them. A placeholder visual (203570 Cosmetic - Frosty Circle (Small)); the retail plate is a glowing rune circle
-- whose visual id is still to find (a fidelity gap). Client-side (Flags 0) so it is drawn; no script, nothing to trigger
DELETE FROM `areatrigger_template` WHERE `Id`=14800004 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(14800004, 1, 0, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=14800004 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(14800004, 1, 14800004, 1, 0, 0, 0, 0, 0, -1, 0, 0, 203570, 0, 1, 0, 3, 3, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId` BETWEEN 14800004 AND 14800005;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(14800004, 14800004, 1, 1480, '12', 4221.56, 2761.36, 350.96, 0, 0, 0, 0, '', 'Icecrown Citadel (DK Frost scenario) - the south gargoyle plate (placeholder visual)', 0),
(14800005, 14800004, 1, 1480, '12', 4221.47, 2777.25, 350.96, 0, 0, 0, 0, '', 'Icecrown Citadel (DK Frost scenario) - the north gargoyle plate (placeholder visual)', 0);
