-- Demons Among Us (40593), which nothing started: nobody cast 199491, whose offer the core would refuse anyway (it takes
-- no quest from the player themself), and the Jace of The Fallen Lion's reward stood silent. As in the retail beta video
-- he now comes with the quest over his head, tells Anduin why (npc_demons_among_us_jace), and at the player's word reveals
-- the Felblade Assassins, so far seen only by demon hunters. A standing Jace (100675, Wowhead's giver of the quest) takes
-- his place when the summon's 18 seconds end. The gossip option's wording is ours: no data has it.
-- The two portals had no spawns. The Courtyard's stands at Wowhead's pin; the Petitioner's Chamber's has no known place,
-- and stands in the hall of that name, among the Assassins there. Using a goober gives its credit; each portal shows only
-- to a player who has yet to destroy it.
-- Anduin 100973, who ends the quest, was the demon hunter's alone. He also starts Illidari Allies (44120), which the
-- core will only offer on turn-in from a creature that starts it; Elerion, who ends that, was also the demon hunter's.

UPDATE `creature_template` SET `npcflag` = `npcflag` | 3, `ScriptName` = 'npc_demons_among_us_jace' WHERE `entry` IN (100675, 101858);

DELETE FROM `creature_queststarter` WHERE (`id` IN (100675, 101858) AND `quest` = 40593) OR (`id` = 100973 AND `quest` = 44120);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(100675, 40593, 0),
(101858, 40593, 0),
(100973, 44120, 0);

INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES (44120, 40593)
ON DUPLICATE KEY UPDATE `PrevQuestID` = VALUES(`PrevQuestID`);

-- Where the reward's spell brings him, facing as it does
DELETE FROM `creature` WHERE `guid` = 15190032;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(15190032, 100675, 0, 1519, 6292, '0', 0, 0, 0, 0, -8393.52, 229.79, 155.35, 0.53, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0);

DELETE FROM `creature_text` WHERE `CreatureID` IN (100675, 101858, 107574);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(101858, 0, 0, 'I bring urgent news, My King. Danger is all around you!', 12, 0, 100, 0, 0, 0, 119325, 0, 'Jace Darkweaver - arrives'),
(101858, 1, 0, 'Say the word and I shall reveal the enemy!', 12, 0, 100, 0, 0, 0, 122115, 0, 'Jace Darkweaver - quest accepted'),
(101858, 2, 0, 'The enemy is among us!', 12, 0, 100, 0, 0, 0, 120160, 0, 'Jace Darkweaver - reveals the enemy'),
(100675, 1, 0, 'Say the word and I shall reveal the enemy!', 12, 0, 100, 0, 0, 0, 122115, 0, 'Jace Darkweaver - quest accepted'),
(100675, 2, 0, 'The enemy is among us!', 12, 0, 100, 0, 0, 0, 120160, 0, 'Jace Darkweaver - reveals the enemy'),
(107574, 0, 0, 'I will hear what this demon hunter has to say.', 12, 0, 100, 0, 0, 0, 122249, 0, 'Anduin Wrynn - Jace arrives');

DELETE FROM `npc_text` WHERE `ID` = 1519001;
INSERT INTO `npc_text` (`ID`, `Probability0`, `BroadcastTextID0`, `VerifiedBuild`) VALUES (1519001, 1, 122115, 0);

DELETE FROM `gossip_menu` WHERE `MenuID` = 1519001;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (1519001, 1519001, 0);

DELETE FROM `gossip_menu_option` WHERE `MenuID` = 1519001;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(1519001, 0, 0, 'Reveal the enemy.', 0, 0, 0, 0, 0, NULL, 0, 0);

DELETE FROM `creature_template_gossip` WHERE `CreatureID` IN (100675, 101858);
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(100675, 1519001, 0),
(101858, 1519001, 0);

DELETE FROM `gameobject` WHERE `guid` IN (15190003, 15190004);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(15190003, 246706, 0, 1519, 6292, '0', 0, 0, -8322.50, 286.00, 156.83, 0, 0, 0, 0, 1, 120, 255, 1, 0), -- the Courtyard
(15190004, 246707, 0, 1519, 6292, '0', 0, 0, -8402.00, 310.00, 147.01, 0, 0, 0, 0, 1, 120, 255, 1, 0); -- the Petitioner's Chamber

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 1519001;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 100675;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 5 AND `SourceEntry` IN (246706, 246707);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 100993 AND `ElseGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 100973 AND `ElseGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 107574 AND ((`ElseGroup` = 0 AND `ConditionValue1` = 40593) OR `ElseGroup` = 2);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 101004 AND `ElseGroup` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 1519001, 0, 0, 0, 47, 0, 40593, 8, 0, '', 0, 0, 0, '', 'Reveal the enemy: Demons Among Us in progress'),
(15, 1519001, 0, 0, 0, 48, 0, 285798, 0, 1, '', 1, 0, 0, '', 'Reveal the enemy: not yet revealed'),
(32, 3, 100675, 0, 0, 47, 0, 40517, 64, 0, '', 0, 0, 0, '', 'Jace (standing): from The Fallen Lion''s turn-in'),
(32, 3, 100675, 0, 0, 47, 0, 40593, 64, 0, '', 1, 0, 0, '', 'Jace (standing): until Demons Among Us is turned in'),
(32, 3, 100675, 0, 0, 1, 0, 200891, 0, 0, '', 1, 0, 0, '', 'Jace (standing): not while the summoned one is there'),
(32, 3, 100993, 0, 1, 48, 0, 285798, 0, 1, '', 0, 0, 0, '', 'Felblade Assassin: once Jace has revealed them'),
(32, 5, 246706, 0, 0, 47, 0, 40593, 8, 0, '', 0, 0, 0, '', 'Demon Portal (Courtyard): Demons Among Us in progress'),
(32, 5, 246706, 0, 0, 48, 0, 285798, 0, 1, '', 0, 0, 0, '', 'Demon Portal (Courtyard): once Jace has revealed the enemy'),
(32, 5, 246706, 0, 0, 48, 0, 281580, 0, 1, '', 1, 0, 0, '', 'Demon Portal (Courtyard): not yet destroyed'),
(32, 5, 246707, 0, 0, 47, 0, 40593, 8, 0, '', 0, 0, 0, '', 'Demon Portal (Petitioner''s Chamber): Demons Among Us in progress'),
(32, 5, 246707, 0, 0, 48, 0, 285798, 0, 1, '', 0, 0, 0, '', 'Demon Portal (Petitioner''s Chamber): once Jace has revealed the enemy'),
(32, 5, 246707, 0, 0, 48, 0, 281581, 0, 1, '', 1, 0, 0, '', 'Demon Portal (Petitioner''s Chamber): not yet destroyed'),
(32, 3, 100973, 0, 1, 47, 0, 40593, 74, 0, '', 0, 0, 0, '', 'Anduin (quest version): from Demons Among Us on'),
(32, 3, 100973, 0, 1, 47, 0, 44120, 1, 0, '', 0, 0, 0, '', 'Anduin (quest version): until Illidari Allies is taken'),
(32, 3, 107574, 0, 0, 47, 0, 40593, 74, 0, '', 1, 0, 0, '', 'Anduin (usual): not from Demons Among Us on'),
(32, 3, 107574, 0, 2, 47, 0, 44120, 74, 0, '', 0, 0, 0, '', 'Anduin (usual): back once Illidari Allies is taken'),
(32, 3, 101004, 0, 1, 47, 0, 44120, 74, 0, '', 0, 0, 0, '', 'Elerion (Mage Quarter): from Illidari Allies on');
