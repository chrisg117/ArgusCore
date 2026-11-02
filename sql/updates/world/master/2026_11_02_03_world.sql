-- DH class hall campaign: Loramus, Is That You? and Demonic Improvements.
-- "Loramus' story" is a kill credit from a line of his, offered while the quest is open.

DELETE FROM `quest_template_addon` WHERE `ID` IN (42681, 42683);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(42681, 2048, 42679),
(42683, 2048, 42681);

-- 210796 is not in Spell.db2: the core would log an error at the turn-in and give nothing
UPDATE `quest_template` SET `RewardSpell`=0 WHERE `ID`=42683;

DELETE FROM `creature_queststarter` WHERE `quest` IN (42681, 42683);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(103761, 42681, 0),
(103761, 42683, 0);

DELETE FROM `gossip_menu_option` WHERE `MenuID`=1519010 AND `OptionID`=1;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `VerifiedBuild`) VALUES
(1519010, 1, 0, 'Tell me your story, Loramus.', 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=1519010;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(15, 1519010, 1, 0, 0, 47, 0, 42683, 8, 0, 0, 'Loramus'' story: during Demonic Improvements');

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=110599;
DELETE FROM `smart_scripts` WHERE `entryorguid`=110599 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_param1`, `event_param2`, `action_type`, `action_param1`, `target_type`, `comment`) VALUES
(110599, 0, 0, 1, 62, 1519010, 1, 33, 110599, 7, 'Loramus: on his story, credit the listener'),
(110599, 0, 1, 0, 61, 0, 0, 72, 0, 7, 'Loramus: on his story, close the gossip');
