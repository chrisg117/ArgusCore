-- DH class hall campaign: Time is of the Essence and Direct Our Wrath, up to the zone choice on the Command Map.
-- From the client: QuestLine 161 runs The Hunter's Gaze -> Time is of the Essence (41096 Altruis / 41067 Kayn) ->
-- Direct Our Wrath (41099 / 41069). Neither has a giver in the data. Allari, who ends The Hunter's Gaze, sends the
-- player up to the star map; the leader there gives Direct Our Wrath. The Command Map 102589 opens the adventure map,
-- whose pins (AdventureMapPOI) are the zone quests, and each casts its Chosen spell on accept: that is the credit.

DELETE FROM `quest_template_addon` WHERE `ID` IN (41096, 41067, 41099, 41069);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(41096, 2048, 41066),
(41067, 2048, 41066),
(41099, 2048, 41096),
(41069, 2048, 41067);

DELETE FROM `creature_queststarter` WHERE `quest` IN (41096, 41067, 41099, 41069);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(95238, 41096, 0),
(95238, 41067, 0),
(95237, 41099, 0),
(95240, 41069, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (41096, 41067);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 41096, 0, 0, 47, 0, 40375, 64, 0, 0, 'Time is of the Essence: Altruis chosen'),
(19, 0, 41067, 0, 0, 47, 0, 40374, 64, 0, 0, 'Time is of the Essence: Kayn chosen');

-- Kayn's lines are Wowhead's; Altruis's versions are not recorded there, so his take the same words
DELETE FROM `quest_offer_reward` WHERE `ID` IN (41096, 41067, 41099, 41069);
INSERT INTO `quest_offer_reward` (`ID`, `RewardText`) VALUES
(41096, '<Altruis sighs.>$B$BI know that look. I, too, gazed into the crucible and saw the horrors of the tomb.$B$BWe must act quickly.'),
(41067, '<Kayn sighs.>$B$BI know that look. I, too, gazed into the crucible and saw the horrors of the tomb.$B$BWe must act quickly.'),
(41099, 'Agreed. You are as skilled a tactician as myself.'),
(41069, 'Agreed. You are as skilled a tactician as myself.');

-- Retail offers Rise, Champions once the first assault point is chosen
UPDATE `quest_template_addon` SET `PrevQuestID`=41069 WHERE `ID`=42670;
UPDATE `quest_template_addon` SET `PrevQuestID`=41099 WHERE `ID`=42671;
