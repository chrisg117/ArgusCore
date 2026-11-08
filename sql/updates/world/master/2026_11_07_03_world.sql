-- DK class hall campaign, slice 2 (QuestLine 232): Recruiting the Troops and Troops in the Field.
-- Both are given and ended by Siouxsie on Wowhead (the enders are already in the data). State masks: 8 in progress,
-- 2 complete, 64 rewarded.

DELETE FROM `quest_template_addon` WHERE `ID` IN (43266, 43267);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(43266, 32, 43265),
(43267, 32, 43266);

DELETE FROM `creature_queststarter` WHERE `quest` IN (43266, 43267);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(93568, 43266, 0), -- Siouxsie: Recruiting the Troops (also offered on Spread the Word's turn-in)
(93568, 43267, 0); -- Siouxsie: Troops in the Field (also offered on Recruiting the Troops' turn-in)

-- 210796 is not in Spell.db2: the core would log an error at the turn-in and give nothing
UPDATE `quest_template` SET `RewardSpell`=0 WHERE `ID`=43267;

-- Dark Summoner Marogh raises the troops. The data has no spawn: he stands at Recruiting the Troops' objective point on
-- the ring at 270 (height from the navmesh), facing the deck, and joins the hall with that quest
UPDATE `creature_template` SET `npcflag`=`npcflag`|1 WHERE `entry`=106435;
DELETE FROM `creature` WHERE `guid`=12207008;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12207008, 106435, 1220, 0, 0, '0', 0, 0, 0, 0, -1580.00, 1094.00, 270.27, 5.77, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Dark Summoner Marogh

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry`=106435;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 106435, 0, 0, 47, 0, 43266, 74, 0, 0, 'Dark Summoner Marogh: from Recruiting the Troops on');

-- His work order, as for the demon hunter's recruiters (2026_11_02_00): the tutorial order 331 (5 s, its completion
-- spell 219802 credits the Pack of Ghouls) while Recruiting the Troops is in the log, the real order 260 otherwise.
-- No broadcast text names the option, so the wording is the fork's
DELETE FROM `gossip_menu` WHERE `MenuID`=1220701;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(1220701, 68, 0);

DELETE FROM `gossip_menu_option` WHERE `MenuID`=1220701;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `VerifiedBuild`) VALUES
(1220701, 0, 28, 'I need ghouls.', 0),
(1220701, 1, 28, 'I need ghouls.', 0);

DELETE FROM `gossip_menu_option_addon` WHERE `MenuID`=1220701;
INSERT INTO `gossip_menu_option_addon` (`MenuID`, `OptionID`, `GarrTalentTreeID`, `CharShipmentID`, `VerifiedBuild`) VALUES
(1220701, 0, NULL, 331, 0),
(1220701, 1, NULL, 260, 0);

DELETE FROM `creature_template_gossip` WHERE `CreatureID`=106435;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(106435, 1220701, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=1220701;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(15, 1220701, 0, 0, 0, 47, 0, 43266, 10, 0, 0, 'The tutorial order: during Recruiting the Troops'),
(15, 1220701, 1, 0, 0, 47, 0, 43266, 10, 0, 1, 'The real order: otherwise');

-- The troops are picked up from the data's own Training Troops object for container 134, beside Marogh
DELETE FROM `gameobject` WHERE `guid`=12207004;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(12207004, 250888, 1220, 0, 0, '0', 0, 0, -1577.40, 1092.50, 270.18, 5.77, 0, 0, 0.2537861, -0.9672604, 300, 255, 1, 0);

-- Turn-in texts, from Wowhead's Progress and Completion sections
DELETE FROM `quest_offer_reward` WHERE `ID` IN (43266, 43267);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(43266, 1, 0, 0, 0, 0, 0, 0, 0, 'Good work, Deathlord.$B$BThese ghouls will be invaluable in the war effort on the Broken Isles. Deploy them on missions just as you would your other champions.', 0),
(43267, 1, 0, 0, 0, 0, 0, 0, 0, 'Excellent work, Deathlord. The mission was a complete success.$B$BWith continued success in the field, the Burning Legion will surely be beaten back.', 0);

DELETE FROM `quest_request_items` WHERE `ID` IN (43266, 43267);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(43266, 6, 6, 0, 0, 'Has Dark Summoner Marogh begun his work?', 0),
(43267, 6, 6, 0, 0, 'How did it go?', 0);
