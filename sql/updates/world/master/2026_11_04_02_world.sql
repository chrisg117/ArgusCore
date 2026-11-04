-- DK Frost artifact, scenario 901 "The Blades of the Fallen Prince" on map 1480 (Icecrown Citadel, difficulty 12),
-- slice 3a: entering. 200428's Death Gate lands the player on Acherus beside the citadel (WorldSafeLocs 5081); across
-- the bone bridge the Scourge Transporter opens the gate (step 0, event 47614) and puts them before Saurfang's door
-- (5076); walking through the door enters the citadel (step 1, event 45677) and moves them into the Spire (5075).
-- 1480 uses ICC 631's coordinates: the transporter stands where 631's does. Spawn ids 14800000-14800999 are ours.
DELETE FROM `instance_template` WHERE `map`=1480;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1480, 0, 'instance_icecrown_citadel_scenario');

DELETE FROM `scenarios` WHERE `map`=1480;
INSERT INTO `scenarios` (`map`, `difficulty`, `scenario_A`, `scenario_H`) VALUES
(1480, 12, 901, 901);

-- Scourge Transporter: ICC's template with a script of ours on this spawn; selectable here (the template's flags 48
-- hide it until ICC's own event)
DELETE FROM `gameobject` WHERE `guid`=14800000;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14800000, 202244, 1480, 0, 0, '12', 0, 0, 0, -1, -549.073, 2211.29, 539.223, 3.14159, 0, 0, 1, 0.00000126759, 300, 100, 1, 'go_icc_scenario_scourge_transporter', NULL, 0);
DELETE FROM `gameobject_overrides` WHERE `spawnId`=14800000;
INSERT INTO `gameobject_overrides` (`spawnId`, `faction`, `flags`) VALUES
(14800000, 0, 32);

-- Saurfang's door (631's door object stands at -469.6): crossing it enters the citadel
DELETE FROM `areatrigger_template` WHERE `Id`=14800000 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(14800000, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=14800000 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(14800000, 1, 14800000, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 4, 4, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId`=14800000;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(14800000, 14800000, 1, 1480, '12', -471.5, 2211.3, 541.1, 0, 0, 0, 0, 'at_icc_scenario_door_to_deathbringers_rise', 'Icecrown Citadel (DK Frost scenario) - Saurfang''s door: enter the citadel', 0);
