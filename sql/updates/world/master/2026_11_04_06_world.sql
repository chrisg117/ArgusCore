-- DK campaign: The Call of Icecrown, third pass after the user's run.

-- Step 1 "The Call of the North" is not a click: the player steps onto the lower deck's transporter pad, which belongs
-- to the map itself. Its criteria is event 47614, and no client data sends it, so a custom areatrigger on the pad does.
-- The spawned 631 transporter (202244) sat on top of the map's own pad, so it goes
DELETE FROM `gameobject` WHERE `guid`=14800000;
DELETE FROM `gameobject_overrides` WHERE `spawnId`=14800000;

DELETE FROM `areatrigger_template` WHERE `Id`=14800002 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(14800002, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=14800002 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(14800002, 1, 14800002, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 3, 3, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId`=14800002;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(14800002, 14800002, 1, 1480, '12', -549.073, 2211.29, 539.223, 0, 0, 0, 0, 'at_icc_scenario_scourge_transporter', 'Icecrown Citadel (DK Frost scenario) - the transporter pad: open the gate', 0);

-- The Lich King's line is a talking head: the core reads a row with a CreatureId and no spawn as one only when
-- NoActorObject is 0 (NoActorObject 1 made it a world object actor with nothing to show)
UPDATE `conversation_actors` SET `NoActorObject`=0 WHERE `ConversationId`=14800001 AND `Idx`=0;
