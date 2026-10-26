-- Beam Me Up (39684): the credit and Altruis's lines come from the data; only Glazer's beam stays in C++.
-- - The Reflective Mirror (244449) is a plain goober again: its own spell 191917 "Rotating" is cast by the player
--   (playerCast), which gives the Glazer kill credit (96680) and sends event 46447.
-- - Event 46447 runs SmartAI (SmartEventTrigger): it turns the beam on Glazer (DoAction 1 on npc_beam_me_up_glazer) and
--   Altruis calls everyone out.
-- - Altruis's call to the player who walks in is a server-side area trigger around Glazer with SmartAI, only for a player
--   on the quest.

UPDATE `gameobject_template` SET `ScriptName` = '', `Data23` = 1 WHERE `entry` = 244449;

DELETE FROM `event_script_names` WHERE `Id` = 46447;
INSERT INTO `event_script_names` (`Id`, `ScriptName`) VALUES
(46447, 'SmartEventTrigger');

DELETE FROM `smart_scripts` WHERE `source_type` = 3 AND `entryorguid` = 46447;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `Difficulties`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param_string`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `action_param7`, `action_param_string`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_param_string`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(46447, 3, 0, 0, '', 87, 0, 100, 0, 0, 0, 0, 0, 0, '', 151, 1, 0, 0, 0, 0, 0, 0, NULL, 19, 96680, 60, 0, 0, NULL, 0, 0, 0, 0, 'On event 46447 (Reflective Mirror turned) - Do action 1 (beam redirected) on closest Glazer'),
(46447, 3, 1, 0, '', 87, 0, 100, 0, 0, 0, 0, 0, 0, '', 1, 1, 0, 0, 0, 0, 0, 0, NULL, 19, 99229, 80, 0, 0, NULL, 0, 0, 0, 0, 'On event 46447 (Reflective Mirror turned) - Closest Altruis the Sufferer says "The shield is broken!"');

-- Altruis calls on a player on the quest who comes within 40 yards of Glazer
DELETE FROM `areatrigger_template` WHERE `Id` = 1468001 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1468001, 1, 1, 0, 0, 0);

DELETE FROM `areatrigger_create_properties` WHERE `Id` = 1468001 AND `IsCustom` = 1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(1468001, 1, 1468001, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 40, 40, 0, 0, 0, 0, '', 0);

DELETE FROM `areatrigger` WHERE `SpawnId` = 1468001;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1468001, 1468001, 1, 1468, '0', 4450.98, -673.28, 117.29, 0, 0, 0, 0, 'SmartAreaTriggerAI', 'Vault of the Wardens - Vault of Mirrors, around Glazer (Quest: "Beam Me Up" - Altruis calls the player)', 0);

DELETE FROM `smart_scripts` WHERE `source_type` = 12 AND `entryorguid` = 1468001;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `Difficulties`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param_string`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `action_param7`, `action_param_string`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_param_string`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(1468001, 12, 0, 0, '', 46, 0, 100, 0, 0, 0, 0, 0, 0, '', 1, 0, 0, 0, 0, 0, 0, 0, NULL, 19, 99229, 60, 0, 0, NULL, 0, 0, 0, 0, 'On areatrigger enter - Closest Altruis the Sufferer says "the beam is focused upon me" to the invoker');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 1468001 AND `SourceId` = 12;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, 1468001, 12, 0, 47, 0, 39684, 8, 0, '', 0, 0, 0, '', 'Vault of Mirrors trigger: Altruis calls only a player with Beam Me Up incomplete');
