-- DK campaign in Dalaran: The Call To War (40714) and A Pact of Necessity (40715).
-- The Dalaran intro aura 224240 starts the Call through 200023; Duke Lankral 101441 ends it and starts the weapon choice.
-- No sniff of this exists: Lankral's position is the quest POI and the choice's text is ours. 199985 launches choice 253,
-- which no other class uses here; only Frost is offered until the Blood and Unholy artifact quests are built.
UPDATE `creature_template` SET `npcflag`=`npcflag`|1, `AIName`='SmartAI' WHERE `entry`=101441;

DELETE FROM `creature_queststarter` WHERE `quest`=40715;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(101441, 40715, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (40714, 40715);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(40714, 32, 0),
(40715, 32, 40714);

-- The blades come from their own scenario (The Call of Icecrown); this tracking quest is auto-rewarded by the choice's spell and would hand them over here
UPDATE `quest_template` SET `RewardItem1`=0, `RewardAmount1`=0 WHERE `ID`=40723;

DELETE FROM `creature` WHERE `guid`=12204150;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204150, 101441, 1220, 7502, 7505, '0', 0, 0, 0, 0, -842.00, 4272.00, 746.48, 1.00, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Duke Lankral

DELETE FROM `gossip_menu` WHERE `MenuID`=7502002;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(7502002, 1, 0);
DELETE FROM `gossip_menu_option` WHERE `MenuID`=7502002;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(7502002, 0, 0, 'I would like to review weapons we might pursue.', 101848, 0, 0, 0, 0, NULL, 0, 0);
DELETE FROM `creature_template_gossip` WHERE `CreatureID`=101441;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(101441, 7502002, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry`=101441;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=7502002;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 101441, 0, 0, 15, 0, 32, 0, 0, 0, 0, 0, '', 'Duke Lankral (Krasus'' Landing): death knights only'),
(15, 7502002, 0, 0, 0, 47, 0, 40715, 8, 0, 0, 0, 0, '', 'Review weapons: A Pact of Necessity in the log, no weapon chosen yet');

-- The choice opens on accept; the gossip line reopens it if the player closed the window
DELETE FROM `smart_scripts` WHERE `entryorguid`=101441 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(101441, 0, 0, 0, 19, 0, 100, 0, 40715, 0, 0, 0, 0, 85, 199985, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Duke Lankral - On quest A Pact of Necessity accepted - Player: cast Player Choice'),
(101441, 0, 1, 0, 62, 0, 100, 0, 7502002, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Duke Lankral - On gossip review weapons - Close gossip'),
(101441, 0, 2, 0, 62, 0, 100, 0, 7502002, 0, 0, 0, 0, 85, 199985, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Duke Lankral - On gossip review weapons - Player: cast Player Choice');

DELETE FROM `playerchoice` WHERE `ChoiceId`=253;
INSERT INTO `playerchoice` (`ChoiceId`, `UiTextureKitId`, `Question`, `HideWarboardHeader`, `MaxResponses`, `ScriptName`, `InfiniteRange`, `VerifiedBuild`) VALUES
(253, 0, 'Which weapon should we pursue first?', 0, NULL, 'playerchoice_the_pact_of_necessity', 0, 0);
DELETE FROM `playerchoice_response` WHERE `ChoiceId`=253;
INSERT INTO `playerchoice_response` (`ChoiceId`, `ResponseId`, `Index`, `ChoiceArtFileId`, `Header`, `Answer`, `Description`, `Confirmation`, `VerifiedBuild`) VALUES
(253, 2531, 0, 1390098, 'Frost', 'Select', 'The Blades of the Fallen Prince, forged from the shards of Frostmourne, still lie in Icecrown Citadel.\n\nClaim the Lich King''s runeblades and turn their cold against the Legion.', 'CONFIRM_ARTIFACT_CHOICE', 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_dk_force_specialization', 'spell_dk_artifacts_choice_reward');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(211222, 'spell_dk_force_specialization'),
(211223, 'spell_dk_force_specialization'),
(211225, 'spell_dk_force_specialization'),
(201261, 'spell_dk_artifacts_choice_reward');

-- The hall founding dispatches by class: site 163 is the demon hunter's own, every other class founds site 161
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_dh_create_order_hall', 'spell_order_hall_founding');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(192190, 'spell_order_hall_founding');
