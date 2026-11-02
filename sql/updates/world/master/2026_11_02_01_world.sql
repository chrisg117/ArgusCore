-- DH class hall campaign: Rise, Champions and the first champions of the Fel Hammer.
-- From the client: the quests and their map points. Wowhead has 103025 Gaardoun as the giver of Rise, Champions, at the map point by
-- the forge chamber's stair (1561,1424); here that spot is held by 103068, the hub's Gaardoun, the entry that stays there.
-- Retail gates it behind Direct Our Wrath (41069/41067); when the zone choice lands, add those as its previous quests.

DELETE FROM `quest_template_addon` WHERE `ID` IN (42670, 42671, 42690, 42695, 42697);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(42670, 2048, 41066),
(42671, 2048, 41066),
(42690, 2048, 0),
(42695, 2048, 0),
(42697, 2048, 0);

-- 225995 is not in Spell.db2: the core would log an error at each turn-in and give nothing
UPDATE `quest_template` SET `RewardSpell`=0 WHERE `ID` IN (42670, 42671);

DELETE FROM `creature_queststarter` WHERE `quest` IN (42670, 42671, 42690, 42695, 42697);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(103068, 42670, 0),
(103068, 42671, 0),
(108571, 42690, 0),
(108572, 42695, 0),
(108326, 42697, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (42670, 42671, 42690, 42695, 42697);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 42670, 0, 0, 47, 0, 40374, 64, 0, 0, 'Rise, Champions: Kayn chosen'),
(19, 0, 42671, 0, 0, 47, 0, 40375, 64, 0, 0, 'Rise, Champions: Altruis chosen'),
(19, 0, 42690, 0, 0, 47, 0, 42670, 8, 0, 0, 'Champion: Altruis: during Rise, Champions'),
(19, 0, 42695, 0, 0, 47, 0, 42671, 8, 0, 0, 'Champion: Kayn: during Rise, Champions'),
(19, 0, 42697, 0, 0, 47, 0, 42670, 8, 0, 0, 'Champion: Asha: during Rise, Champions (Kayn)'),
(19, 0, 42697, 0, 1, 47, 0, 42671, 8, 0, 0, 'Champion: Asha: during Rise, Champions (Altruis)');

-- The one not chosen comes back as a champion, at the door where the other arrived
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (108571, 108572) AND `ElseGroup`=1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 108571, 0, 1, 47, 0, 40374, 64, 0, 0, 'Altruis the champion: Kayn chosen'),
(32, 3, 108571, 0, 1, 47, 0, 42670, 74, 0, 0, 'Altruis the champion: from Rise, Champions on'),
(32, 3, 108572, 0, 1, 47, 0, 40375, 64, 0, 0, 'Kayn the champion: Altruis chosen'),
(32, 3, 108572, 0, 1, 47, 0, 42671, 74, 0, 0, 'Kayn the champion: from Rise, Champions on');
