-- DH class hall campaign: Additional Accoutrements, given by Malevolence and taken by Kor'vas.
-- The mission's armament is not delivered: the engine leaves out items brought back from missions (reward type 21). The quest still credits.

DELETE FROM `quest_template_addon` WHERE `ID`=42682;
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(42682, 2048, 42683);

DELETE FROM `creature_queststarter` WHERE `quest`=42682;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(98632, 42682, 0);
