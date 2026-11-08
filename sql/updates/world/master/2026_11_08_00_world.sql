-- DK class hall campaign, slice 3 (QuestLine 232): Tech It Up A Notch and Salanar the Horseman.
-- Siouxsie gives and ends both (the enders are already in the data). 43539 follows 43268 in the QuestLine's order.

DELETE FROM `quest_template_addon` WHERE `ID` IN (43268, 43539);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(43268, 32, 43267),
(43539, 32, 43268);

DELETE FROM `creature_queststarter` WHERE `quest` IN (43268, 43539);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(93568, 43268, 0), -- Siouxsie: Tech It Up A Notch (also offered on Troops in the Field's turn-in)
(93568, 43539, 0); -- Siouxsie: Salanar the Horseman

-- 226293 is not in Spell.db2: the core would log an error at the turn-in and give nothing. The data's RewardNextQuest
-- 42449 (Return of the Four Horsemen) is where this quest stood in an earlier order; here 42449 is long done, so it
-- would offer a finished quest again
UPDATE `quest_template` SET `RewardSpell`=0, `RewardNextQuest`=0 WHERE `ID`=43539;

-- Archivist Zubashi keeps the Ebon Hold's upgrades (talent tree 128). The data has no spawn: he stands at Tech It Up A
-- Notch's objective point on the deck, facing Siouxsie. He is a standing hall service, so he has no condition
UPDATE `creature_template` SET `npcflag`=`npcflag`|1 WHERE `entry`=97485;
DELETE FROM `creature` WHERE `guid`=12207009;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12207009, 97485, 1220, 0, 0, '0', 0, 0, 0, 0, -1517.00, 1063.00, 260.50, 5.44, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Archivist Zubashi

-- His option opens the upgrades, as Loramus's does for the Fel Hammer (2026_11_02_03). No broadcast text holds his lines,
-- so the greeting is the generic 68 and the option's wording is the fork's
DELETE FROM `gossip_menu` WHERE `MenuID`=1220702;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(1220702, 68, 0);

DELETE FROM `gossip_menu_option` WHERE `MenuID`=1220702;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `VerifiedBuild`) VALUES
(1220702, 0, 32, 'What can be done to improve Acherus?', 0);

DELETE FROM `gossip_menu_option_addon` WHERE `MenuID`=1220702;
INSERT INTO `gossip_menu_option_addon` (`MenuID`, `OptionID`, `GarrTalentTreeID`, `CharShipmentID`, `VerifiedBuild`) VALUES
(1220702, 0, 128, NULL, 0);

DELETE FROM `creature_template_gossip` WHERE `CreatureID`=97485;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(97485, 1220702, 0);

-- Turn-in texts, from Wowhead's Completion sections (neither has a Progress line for Siouxsie)
DELETE FROM `quest_offer_reward` WHERE `ID` IN (43268, 43539);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(43268, 1, 0, 0, 0, 0, 0, 0, 0, 'That was an excellent choice, Deathlord. Feel free to speak to Zubashi whenever you require further upgrades.', 0),
(43539, 1, 0, 0, 0, 0, 0, 0, 0, 'The Ebon Blade champions sent to the Realm of Shadows have successfully returned to Acherus.$B$BSalanar the Horseman will be arriving shortly to begin preparations for the dread steeds. You should meet with him when he arrives.', 0);
