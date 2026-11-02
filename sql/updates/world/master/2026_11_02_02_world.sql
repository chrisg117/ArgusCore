-- DH class hall campaign: Things Gaardoun Needs and Broken Warriors, given and taken by Kor'vas.
-- The mission appears through its player condition (2026_11_02_00_hotfixes.sql). 44161 and mission 1389 are a later duplicate of
-- 42677 with the same text and objectives, and nothing in the data tells them apart: 44161 stays unbuilt.

DELETE FROM `quest_template_addon` WHERE `ID` IN (42677, 42679);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(42677, 2048, 0),
(42679, 2048, 42677);

DELETE FROM `creature_queststarter` WHERE `quest` IN (42677, 42679);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(103761, 42677, 0),
(103761, 42679, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry`=42677;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 42677, 0, 0, 47, 0, 42670, 64, 0, 0, 'Things Gaardoun Needs: after Rise, Champions (Kayn)'),
(19, 0, 42677, 0, 1, 47, 0, 42671, 64, 0, 0, 'Things Gaardoun Needs: after Rise, Champions (Altruis)');
