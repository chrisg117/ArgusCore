-- DK campaign: The Call of Icecrown, scenario 901 step 1811 "Travel to the Frozen Throne" (Task 3c).
-- The step needs event 45597, which only 185869 "Teleport to Throne" sends (teleport to a DB destination, then the
-- event). The pad is the user's ping 8 (4357.08, 2770.32, 356.05), the spot of 631's Frozen Throne transporter (GO 202223
-- at 4356.93, 2769.41, 355.955): 1480 reuses 631's coordinates. The landing is 631's own throne teleport (70860), with
-- the facing from the spell. If the map draws no pad here, the transporter model (251460) goes in a file of its own

DELETE FROM `spell_target_position` WHERE `ID`=185869;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(185869, 0, 0, 1480, 529.302, -2124.49, 840.857, 3.11718, 0);

-- The pad: a server-side sphere on the teleporter
DELETE FROM `areatrigger_template` WHERE `Id`=14800005 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(14800005, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=14800005 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(14800005, 1, 14800005, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 2.5, 2.5, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId`=14800006;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(14800006, 14800005, 1, 1480, '12', 4356.93, 2769.41, 355.955, 0, 0, 0, 0, 'at_icc_scenario_spire_teleporter', 'Icecrown Citadel (DK Frost scenario) - the Spire teleporter: to the Frozen Throne', 0);
