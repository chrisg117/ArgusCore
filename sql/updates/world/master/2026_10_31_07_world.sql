-- DH campaign in Dalaran: Call of the Illidari (39047 Altruis / 39261 Kayn) and The Power to Survive (40816 / 40814).
-- Kor'vas 99343 is summoned by the Dalaran intro aura 224240 and starts the Call; the leader the player chose at the Vault
-- (tracking quests 40375 / 40374) ends it and starts the weapon choice. No sniff of this exists: the NPC positions (the
-- quest POI) and the choice's text and art are ours. Choice 255 is the client's: spell 201092 launches it.
UPDATE `creature_template` SET `npcflag`=`npcflag`|2, `ScriptName`='npc_korvas_bloodthorn_dalaran' WHERE `entry`=99343;
UPDATE `creature_template` SET `npcflag`=`npcflag`|1, `AIName`='SmartAI' WHERE `entry` IN (99254, 99247);

DELETE FROM `creature_queststarter` WHERE `quest` IN (39047, 39261, 40816, 40814);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(99343, 39047, 0),
(99343, 39261, 0),
(99254, 40816, 0),
(99247, 40814, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (39047, 39261, 40816, 40814);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(39047, 2048, 0),
(39261, 2048, 0),
(40816, 2048, 39047),
(40814, 2048, 39261);

-- The artifact comes from its own scenario (The Hunt); these tracking quests are auto-rewarded by the choice's spell and would hand it over here
UPDATE `quest_template` SET `RewardItem1`=0, `RewardAmount1`=0 WHERE `ID` IN (40817, 40818);

-- Altruis and Kayn stand on the same spot: a player only ever sees the leader they chose
DELETE FROM `creature` WHERE `guid` IN (12202019, 12202020);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12202019, 99254, 1220, 7502, 7505, '0', 0, 0, 0, 0, -855.00, 4254.00, 746.28, 0.96, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Altruis
(12202020, 99247, 1220, 7502, 7505, '0', 0, 0, 0, 0, -855.00, 4254.00, 746.28, 0.96, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Kayn

DELETE FROM `gossip_menu` WHERE `MenuID`=7502001;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(7502001, 1, 0);
DELETE FROM `gossip_menu_option` WHERE `MenuID`=7502001;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(7502001, 0, 0, 'I would like to review weapons we might pursue.', 101848, 0, 0, 0, 0, NULL, 0, 0);
DELETE FROM `creature_template_gossip` WHERE `CreatureID` IN (99254, 99247);
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(99254, 7502001, 0),
(99247, 7502001, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (39047, 39261, 40816, 40814);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (99254, 99247);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=7502001;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 39047, 0, 0, 47, 0, 40375, 64, 0, 0, 0, 0, '', 'Call of the Illidari (Altruis): Altruis chosen'),
(19, 0, 40816, 0, 0, 47, 0, 40375, 64, 0, 0, 0, 0, '', 'The Power to Survive (Altruis): Altruis chosen'),
(19, 0, 39261, 0, 0, 47, 0, 40374, 64, 0, 0, 0, 0, '', 'Call of the Illidari (Kayn): Kayn chosen'),
(19, 0, 40814, 0, 0, 47, 0, 40374, 64, 0, 0, 0, 0, '', 'The Power to Survive (Kayn): Kayn chosen'),
(32, 3, 99254, 0, 0, 47, 0, 40375, 64, 0, 0, 0, 0, '', 'Altruis (Krasus'' Landing): Altruis chosen'),
(32, 3, 99254, 0, 0, 15, 0, 2048, 0, 0, 0, 0, 0, '', 'Altruis (Krasus'' Landing): demon hunters only'),
(32, 3, 99247, 0, 0, 47, 0, 40374, 64, 0, 0, 0, 0, '', 'Kayn (Krasus'' Landing): Kayn chosen'),
(32, 3, 99247, 0, 0, 15, 0, 2048, 0, 0, 0, 0, 0, '', 'Kayn (Krasus'' Landing): demon hunters only'),
(15, 7502001, 0, 0, 0, 47, 0, 40816, 8, 0, 0, 0, 0, '', 'Review weapons: The Power to Survive (Altruis) in the log, no weapon chosen yet'),
(15, 7502001, 0, 0, 1, 47, 0, 40814, 8, 0, 0, 0, 0, '', 'Review weapons: The Power to Survive (Kayn) in the log, no weapon chosen yet');

-- The choice opens on accept; the gossip line reopens it if the player closed the window
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (99254, 99247) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(99254, 0, 0, 0, 19, 0, 100, 0, 40816, 0, 0, 0, 0, 85, 201092, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Altruis - On quest The Power to Survive accepted - Player: cast Player Choice'),
(99254, 0, 1, 0, 62, 0, 100, 0, 7502001, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Altruis - On gossip review weapons - Close gossip'),
(99254, 0, 2, 0, 62, 0, 100, 0, 7502001, 0, 0, 0, 0, 85, 201092, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Altruis - On gossip review weapons - Player: cast Player Choice'),
(99247, 0, 0, 0, 19, 0, 100, 0, 40814, 0, 0, 0, 0, 85, 201092, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Kayn - On quest The Power to Survive accepted - Player: cast Player Choice'),
(99247, 0, 1, 0, 62, 0, 100, 0, 7502001, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Kayn - On gossip review weapons - Close gossip'),
(99247, 0, 2, 0, 62, 0, 100, 0, 7502001, 0, 0, 0, 0, 85, 201092, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Kayn - On gossip review weapons - Player: cast Player Choice');

DELETE FROM `playerchoice` WHERE `ChoiceId`=255;
INSERT INTO `playerchoice` (`ChoiceId`, `UiTextureKitId`, `Question`, `HideWarboardHeader`, `MaxResponses`, `ScriptName`, `InfiniteRange`, `VerifiedBuild`) VALUES
(255, 0, 'Which weapon should we pursue first?', 0, NULL, 'playerchoice_the_power_to_survive', 0, 0);
DELETE FROM `playerchoice_response` WHERE `ChoiceId`=255;
INSERT INTO `playerchoice_response` (`ChoiceId`, `ResponseId`, `Index`, `ChoiceArtFileId`, `Header`, `Answer`, `Description`, `Confirmation`, `VerifiedBuild`) VALUES
(255, 2551, 0, 1274664, 'Havoc', 'Select', 'The Twinblades of the Deceiver, Illidan''s own warglaives, lie in the hands of Varedis Felsoul at Felsoul Hold in Suramar.\n\nTake them back and carve through the Legion with the Betrayer''s own blades.', 'CONFIRM_ARTIFACT_CHOICE', 0),
(255, 2552, 1, 1274665, 'Vengeance', 'Select', 'The Aldrachi Warblades, the weapons of an ancient order of the Broken, have been shattered and scattered by the Legion.\n\nRecover them and stand unbroken against the demons, as the Aldrachi once did.', 'CONFIRM_ARTIFACT_CHOICE', 0);

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_dh_force_specialization';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(211226, 'spell_dh_force_specialization'),
(211227, 'spell_dh_force_specialization');
