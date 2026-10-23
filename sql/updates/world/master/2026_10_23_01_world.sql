-- Cry Havoc and Let Slip the Illidari! (39516) and Vengeance Will Be Mine! (39515): the player teaches Allari, Cyana,
-- Kayn, Kor'vas and then Mannethrel what the Tome of Fel Secrets held. Their gossip menus (sniffed, with the teaching
-- option and each one's greeting) existed but were never linked, so none of them could be talked to; Mannethrel
-- Darkstar (96652) was not spawned at the Illidari Foothold either. Picking the option makes the player cast that
-- teacher's "Taught" spell (194996, 195019-195022: the credit), and the teacher answers with their retail line (voice
-- files vo_70_<name>_NN). Both quests credit the same teachers, so the same spells serve Vengeance.

DELETE FROM `creature_template_gossip` WHERE `CreatureID` IN (93127, 96655, 96420, 99045, 96652);
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(93127, 18435, 0), -- Kayn Sunfury
(96655, 18935, 0), -- Allari the Souleater
(96420, 18936, 0), -- Cyana Nightglaive
(99045, 18937, 0), -- Kor'vas Bloodthorn
(96652, 18823, 0); -- Mannethrel Darkstar

-- The teaching option shows while the quest is in the log and that teacher is not yet taught; Mannethrel's objective
-- is sequenced, so his waits for the other four
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND ((`SourceGroup` = 18435 AND `SourceEntry` = 1) OR (`SourceGroup` IN (18935, 18936, 18937, 18823) AND `SourceEntry` = 0));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 18935, 0, 0, 0, 47, 0, 39516, 8, 0, '', 0, 0, 0, '', 'Allari: teach Havoc while Cry Havoc is in progress'),
(15, 18935, 0, 0, 0, 48, 0, 280458, 0, 1, '', 1, 0, 0, '', 'Allari: not yet taught Havoc'),
(15, 18935, 0, 0, 1, 47, 0, 39515, 8, 0, '', 0, 0, 0, '', 'Allari: teach Vengeance while Vengeance Will Be Mine! is in progress'),
(15, 18935, 0, 0, 1, 48, 0, 280463, 0, 1, '', 1, 0, 0, '', 'Allari: not yet taught Vengeance'),
(15, 18936, 0, 0, 0, 47, 0, 39516, 8, 0, '', 0, 0, 0, '', 'Cyana: teach Havoc while Cry Havoc is in progress'),
(15, 18936, 0, 0, 0, 48, 0, 280459, 0, 1, '', 1, 0, 0, '', 'Cyana: not yet taught Havoc'),
(15, 18936, 0, 0, 1, 47, 0, 39515, 8, 0, '', 0, 0, 0, '', 'Cyana: teach Vengeance while Vengeance Will Be Mine! is in progress'),
(15, 18936, 0, 0, 1, 48, 0, 280464, 0, 1, '', 1, 0, 0, '', 'Cyana: not yet taught Vengeance'),
(15, 18435, 1, 0, 0, 47, 0, 39516, 8, 0, '', 0, 0, 0, '', 'Kayn: teach Havoc while Cry Havoc is in progress'),
(15, 18435, 1, 0, 0, 48, 0, 280460, 0, 1, '', 1, 0, 0, '', 'Kayn: not yet taught Havoc'),
(15, 18435, 1, 0, 1, 47, 0, 39515, 8, 0, '', 0, 0, 0, '', 'Kayn: teach Vengeance while Vengeance Will Be Mine! is in progress'),
(15, 18435, 1, 0, 1, 48, 0, 280465, 0, 1, '', 1, 0, 0, '', 'Kayn: not yet taught Vengeance'),
(15, 18937, 0, 0, 0, 47, 0, 39516, 8, 0, '', 0, 0, 0, '', 'Kor''vas: teach Havoc while Cry Havoc is in progress'),
(15, 18937, 0, 0, 0, 48, 0, 280461, 0, 1, '', 1, 0, 0, '', 'Kor''vas: not yet taught Havoc'),
(15, 18937, 0, 0, 1, 47, 0, 39515, 8, 0, '', 0, 0, 0, '', 'Kor''vas: teach Vengeance while Vengeance Will Be Mine! is in progress'),
(15, 18937, 0, 0, 1, 48, 0, 280466, 0, 1, '', 1, 0, 0, '', 'Kor''vas: not yet taught Vengeance'),
(15, 18823, 0, 0, 0, 47, 0, 39516, 8, 0, '', 0, 0, 0, '', 'Mannethrel: teach Havoc while Cry Havoc is in progress'),
(15, 18823, 0, 0, 0, 48, 0, 280462, 0, 1, '', 1, 0, 0, '', 'Mannethrel: not yet taught Havoc'),
(15, 18823, 0, 0, 0, 48, 0, 280458, 0, 1, '', 0, 0, 0, '', 'Mannethrel: after Allari (Havoc)'),
(15, 18823, 0, 0, 0, 48, 0, 280459, 0, 1, '', 0, 0, 0, '', 'Mannethrel: after Cyana (Havoc)'),
(15, 18823, 0, 0, 0, 48, 0, 280460, 0, 1, '', 0, 0, 0, '', 'Mannethrel: after Kayn (Havoc)'),
(15, 18823, 0, 0, 0, 48, 0, 280461, 0, 1, '', 0, 0, 0, '', 'Mannethrel: after Kor''vas (Havoc)'),
(15, 18823, 0, 0, 1, 47, 0, 39515, 8, 0, '', 0, 0, 0, '', 'Mannethrel: teach Vengeance while Vengeance Will Be Mine! is in progress'),
(15, 18823, 0, 0, 1, 48, 0, 280467, 0, 1, '', 1, 0, 0, '', 'Mannethrel: not yet taught Vengeance'),
(15, 18823, 0, 0, 1, 48, 0, 280463, 0, 1, '', 0, 0, 0, '', 'Mannethrel: after Allari (Vengeance)'),
(15, 18823, 0, 0, 1, 48, 0, 280464, 0, 1, '', 0, 0, 0, '', 'Mannethrel: after Cyana (Vengeance)'),
(15, 18823, 0, 0, 1, 48, 0, 280465, 0, 1, '', 0, 0, 0, '', 'Mannethrel: after Kayn (Vengeance)'),
(15, 18823, 0, 0, 1, 48, 0, 280466, 0, 1, '', 0, 0, 0, '', 'Mannethrel: after Kor''vas (Vengeance)');

-- Their answers once taught; Kayn's "Teach us what you have learned" when the quest is taken
DELETE FROM `creature_text` WHERE (`CreatureID` = 93127 AND `GroupID` IN (2, 3)) OR (`CreatureID` IN (96655, 96420) AND `GroupID` = 1) OR `CreatureID` IN (99045, 96652);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(93127, 2, 0, 'I can feel it coursing through my veins. Altering my body.', 12, 0, 100, 0, 0, 55258, 0, 100746, 0, 'Kayn Sunfury - taught'),
(93127, 3, 0, 'Teach us what you have learned, $n.', 12, 0, 100, 0, 0, 55265, 0, 100722, 0, 'Kayn Sunfury - spec quest accepted'),
(96655, 1, 0, 'The demonic energy... I feel deadlier.', 12, 0, 100, 0, 0, 55006, 0, 100731, 0, 'Allari the Souleater - taught'),
(96420, 1, 0, 'More... I want more power!', 12, 0, 100, 0, 0, 55094, 0, 100740, 0, 'Cyana Nightglaive - taught'),
(99045, 0, 0, 'I could destroy the Legion on my own!', 12, 0, 100, 0, 0, 55292, 0, 100748, 0, 'Kor''vas Bloodthorn - taught'),
(96652, 0, 0, '$n, wait... I can''t contain the power.', 12, 0, 100, 0, 0, 55313, 0, 100753, 0, 'Mannethrel Darkstar - taught'),
(96652, 1, 0, 'Aaaaggggghhhhh!', 14, 0, 100, 0, 0, 55312, 0, 100754, 0, 'Mannethrel Darkstar - taught, after');

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (99045, 96652);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND ((`entryorguid` = 93127 AND `id` BETWEEN 5 AND 9) OR (`entryorguid` IN (96655, 96420) AND `id` BETWEEN 2 AND 4) OR `entryorguid` IN (99045, 96652));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `Difficulties`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param_string`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `action_param7`, `action_param_string`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_param_string`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(93127, 0, 5, 0, '', 62, 0, 100, 0, 18435, 1, 0, 0, 0, '', 85, 195020, 2, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Kayn Sunfury - On gossip option 1 selected - Invoker casts Kayn Taught'),
(93127, 0, 6, 0, '', 62, 0, 100, 0, 18435, 1, 0, 0, 0, '', 1, 2, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Kayn Sunfury - On gossip option 1 selected - Talk 2 to invoker'),
(93127, 0, 7, 0, '', 62, 0, 100, 0, 18435, 1, 0, 0, 0, '', 72, 0, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Kayn Sunfury - On gossip option 1 selected - Close gossip'),
(93127, 0, 8, 0, '', 19, 0, 100, 0, 39516, 0, 0, 0, 0, '', 1, 3, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Kayn Sunfury - On player accepted quest Cry Havoc and Let Slip the Illidari! - Talk 3 to invoker'),
(93127, 0, 9, 0, '', 19, 0, 100, 0, 39515, 0, 0, 0, 0, '', 1, 3, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Kayn Sunfury - On player accepted quest Vengeance Will Be Mine! - Talk 3 to invoker'),
(96655, 0, 2, 0, '', 62, 0, 100, 0, 18935, 0, 0, 0, 0, '', 85, 194996, 2, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Allari the Souleater - On gossip option 0 selected - Invoker casts Allari Taught'),
(96655, 0, 3, 0, '', 62, 0, 100, 0, 18935, 0, 0, 0, 0, '', 1, 1, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Allari the Souleater - On gossip option 0 selected - Talk 1 to invoker'),
(96655, 0, 4, 0, '', 62, 0, 100, 0, 18935, 0, 0, 0, 0, '', 72, 0, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Allari the Souleater - On gossip option 0 selected - Close gossip'),
(96420, 0, 2, 0, '', 62, 0, 100, 0, 18936, 0, 0, 0, 0, '', 85, 195019, 2, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Cyana Nightglaive - On gossip option 0 selected - Invoker casts Cyana Taught'),
(96420, 0, 3, 0, '', 62, 0, 100, 0, 18936, 0, 0, 0, 0, '', 1, 1, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Cyana Nightglaive - On gossip option 0 selected - Talk 1 to invoker'),
(96420, 0, 4, 0, '', 62, 0, 100, 0, 18936, 0, 0, 0, 0, '', 72, 0, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Cyana Nightglaive - On gossip option 0 selected - Close gossip'),
(99045, 0, 0, 0, '', 62, 0, 100, 0, 18937, 0, 0, 0, 0, '', 85, 195021, 2, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Kor''vas Bloodthorn - On gossip option 0 selected - Invoker casts Kor''vas Taught'),
(99045, 0, 1, 0, '', 62, 0, 100, 0, 18937, 0, 0, 0, 0, '', 1, 0, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Kor''vas Bloodthorn - On gossip option 0 selected - Talk 0 to invoker'),
(99045, 0, 2, 0, '', 62, 0, 100, 0, 18937, 0, 0, 0, 0, '', 72, 0, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Kor''vas Bloodthorn - On gossip option 0 selected - Close gossip'),
(96652, 0, 0, 0, '', 62, 0, 100, 0, 18823, 0, 0, 0, 0, '', 85, 195022, 2, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Mannethrel Darkstar - On gossip option 0 selected - Invoker casts Mannethrel Taught'),
(96652, 0, 1, 0, '', 62, 0, 100, 0, 18823, 0, 0, 0, 0, '', 1, 0, 3000, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Mannethrel Darkstar - On gossip option 0 selected - Talk 0 to invoker'),
(96652, 0, 2, 0, '', 62, 0, 100, 0, 18823, 0, 0, 0, 0, '', 72, 0, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Mannethrel Darkstar - On gossip option 0 selected - Close gossip'),
(96652, 0, 3, 0, '', 52, 0, 100, 0, 0, 96652, 0, 0, 0, '', 1, 1, 0, 0, 0, 0, 0, 0, NULL, 1, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Mannethrel Darkstar - On text 0 over - Talk 1');

-- Mannethrel Darkstar at the Illidari Foothold, from his Wowhead pins, once he has been freed (Set Them Free, 38759),
-- facing Kayn
DELETE FROM `creature` WHERE `guid` = 14810006;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14810006, 96652, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1463.7, 1775.0, 54.56, 4.114, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 96652;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 96652, 0, 0, 47, 0, 38759, 64, 0, '', 0, 0, 0, '', 'Mannethrel Darkstar is at the Illidari Foothold once Set Them Free is done');
