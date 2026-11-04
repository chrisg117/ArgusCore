-- DK campaign: The Call of Icecrown's arrival, second pass after the user's run.

-- 190789's scene 1076 is package 1467 "Icecrown Entrance Scene" (bone drakes over the Spire, Acherus docked on
-- Icecrown); 1466 "Looting the Blades (1. Loot Scene)" is the blades loot later in the scenario
UPDATE `scene_template` SET `ScriptPackageID`=1467 WHERE `SceneId`=1076;

-- Mograine starts when the player stands on the landing: an areatrigger there casts 188022 Icecrown - Mograine Ping
DELETE FROM `areatrigger_template` WHERE `Id`=14800001 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(14800001, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=14800001 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(14800001, 1, 14800001, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 10, 10, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId`=14800001;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(14800001, 14800001, 1, 1480, '12', -737.07, 2204.70, 535.67, 0, 0, 0, 0, 'at_icc_scenario_landing', 'Icecrown Citadel (DK Frost scenario) - the landing: Mograine starts', 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=188022;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 188022, 0, 0, 31, 0, 3, 95193, 0, 0, 0, 0, '', 'Icecrown - Mograine Ping: Highlord Darion Mograine');
DELETE FROM `spell_script_names` WHERE `spell_id`=188022;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(188022, 'spell_icc_scenario_mograine_ping');
