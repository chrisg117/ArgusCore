-- The Keystone (38728) could not be started or done. Cyana Nightglaive (97297) on the Fel Hammer's deck now gives it
-- after On Felbat Wings; Brood Queen Tyranna (93802) is spawned in her command chamber, already fighting Kayn, Jace and
-- Allari (the retail video: the others "already charged in"), and drops the Sargerite Keystone; the portal chamber
-- below has Kayn Sunfury (97303), who ends it, with Jace, Cyana, Allari and Belath. Positions from Wowhead pins and the
-- retail video's coordinates, heights from the navmesh.

DELETE FROM `creature_queststarter` WHERE `id` = 97297 AND `quest` = 38728;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(97297, 38728, 0);

DELETE FROM `quest_template_addon` WHERE `ID` = 38728;
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(38728, 39663);

-- Cyana's two lines on accepting it (retail video; voice files vo_70_cyana_nightglaive)
DELETE FROM `creature_text` WHERE `CreatureID` IN (97297, 93802, 97244);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(97297, 0, 0, 'I''ll secure the exit portal back to the Black Temple once the others fly up here.', 12, 0, 100, 0, 0, 55096, 0, 100418, 0, 'Cyana Nightglaive - The Keystone accepted'),
(97297, 1, 0, 'Hurry. Kayn has already launched the attack on Tyranna.', 12, 0, 100, 0, 0, 55095, 0, 99829, 0, 'Cyana Nightglaive - The Keystone accepted 2'),
(93802, 0, 0, '$n! You and your demon hunters are nothing.', 12, 0, 100, 0, 0, 55064, 0, 99826, 0, 'Brood Queen Tyranna - player near'),
(93802, 1, 0, 'The keystone was entrusted to me by Sargeras himself. You will not have it!', 12, 0, 100, 0, 0, 55065, 0, 99827, 0, 'Brood Queen Tyranna - player near 2'),
(93802, 2, 0, 'None can resist my kiss.', 14, 0, 100, 0, 0, 55068, 0, 103514, 0, 'Brood Queen Tyranna - Queen''s Bite'),
(93802, 3, 0, 'Come forth, my children. Feast upon our enemies.', 14, 0, 100, 0, 0, 55069, 0, 103517, 0, 'Brood Queen Tyranna - Into the Shadows'),
(93802, 4, 0, 'My Lord Sargeras, no!!!', 14, 0, 100, 0, 0, 55055, 0, 99012, 0, 'Brood Queen Tyranna - death'),
(97244, 0, 0, 'Grab the keystone! Let''s get to the portal down below and back to the Black Temple!', 14, 0, 100, 0, 0, 55268, 0, 102063, 0, 'Kayn Sunfury - Brood Queen Tyranna died');

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 97297;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 97297;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `Difficulties`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param_string`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `action_param7`, `action_param_string`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_param_string`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(97297, 0, 0, 0, '', 19, 0, 100, 0, 38728, 0, 0, 0, 0, '', 1, 0, 5000, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Cyana Nightglaive - On quest The Keystone accepted - Talk 0 to invoker'),
(97297, 0, 1, 0, '', 52, 0, 100, 0, 0, 97297, 0, 0, 0, '', 1, 1, 0, 0, 0, 0, 0, 0, NULL, 1, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Cyana Nightglaive - On text 0 over - Talk 1');

-- Tyranna and the three Illidari fighting her in the command chamber, which find each other by their StringId
UPDATE `creature_template` SET `ScriptName` = 'npc_brood_queen_tyranna' WHERE `entry` = 93802;
UPDATE `creature_template_difficulty` SET `LootID` = 93802 WHERE `Entry` = 93802 AND `DifficultyID` = 0;

DELETE FROM `creature_loot_template` WHERE `Entry` = 93802;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(93802, 0, 124672, 100, 1, 1, 0, 1, 1, 'Brood Queen Tyranna - Sargerite Keystone');

-- Queen's Bite picks a nearby creature of the listed entries: the Illidari fighting her
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 197486;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 3, 197486, 0, 0, 31, 0, 3, 97244, 0, '', 0, 0, 0, '', 'Queen''s Bite targets Kayn Sunfury'),
(13, 3, 197486, 0, 1, 31, 0, 3, 97959, 0, '', 0, 0, 0, '', 'Queen''s Bite targets Jace Darkweaver'),
(13, 3, 197486, 0, 2, 31, 0, 3, 97962, 0, '', 0, 0, 0, '', 'Queen''s Bite targets Allari the Souleater');

-- The squad fight script now serves both places under one name
UPDATE `creature` SET `ScriptName` = 'npc_mardum_squad_fighter' WHERE `ScriptName` = 'npc_cryptic_hollow_squad_fighter';

DELETE FROM `creature` WHERE `guid` BETWEEN 14810009 AND 14810017;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
-- command chamber
(14810009, 93802, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1566.0, 1415.0, 237.23, 3.142, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', 'FelHammerCommandChamber', 0),
(14810010, 97244, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1561.0, 1415.0, 237.23, 0.000, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'FelHammerCommandChamber', 0),
(14810011, 97959, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1562.0, 1411.0, 237.23, 0.785, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'FelHammerCommandChamber', 0),
(14810012, 97962, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1562.0, 1419.0, 237.23, 5.498, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'FelHammerCommandChamber', 0),
-- portal chamber below, facing the way in from the command chamber
(14810013, 97303, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1632.8, 1415.2, 209.33, 3.142, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14810014, 97964, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1628.5, 1411.5, 209.19, 3.142, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14810015, 98714, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1628.5, 1418.5, 209.25, 3.142, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14810016, 97965, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1633.5, 1419.5, 209.38, 3.142, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14810017, 96654, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1607.6, 1415.2, 211.24, 0.000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0);

-- "Find the way downstairs" (101760): Mardum has no client area triggers, so a server-side one fills the portal chamber.
-- The objective is sequenced, so it only counts once the keystone is looted.
DELETE FROM `areatrigger_template` WHERE `Id` = 1481003 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1481003, 1, 1, 0, 0, 0);

DELETE FROM `areatrigger_create_properties` WHERE `Id` = 1481003 AND `IsCustom` = 1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(1481003, 1, 1481003, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 10, 10, 0, 0, 0, 0, '', 0);

DELETE FROM `areatrigger` WHERE `SpawnId` = 1481001;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1481001, 1481003, 1, 1481, '0', 1630.0, 1415.0, 209.26, 0, 0, 0, 0, 'SmartAreaTriggerAI', 'The Fel Hammer - portal chamber (Quest: "The Keystone" - Find the way downstairs)', 0);

DELETE FROM `smart_scripts` WHERE `source_type` = 12 AND `entryorguid` = 1481003;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `Difficulties`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param_string`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `action_param7`, `action_param_string`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_param_string`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(1481003, 12, 0, 0, '', 46, 0, 100, 0, 0, 0, 0, 0, 0, '', 33, 101760, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'On areatrigger enter - Kill credit "The Keystone" Make Your Way Downstairs to invoker');

-- The quest's reward spell 193387 plays scene 1142, whose package is "The Keystone: Metamorphosis" (1512)
DELETE FROM `scene_template` WHERE `SceneId` = 1142;
INSERT INTO `scene_template` (`SceneId`, `Flags`, `ScriptPackageID`, `ScriptName`) VALUES
(1142, 16, 1512, '');
