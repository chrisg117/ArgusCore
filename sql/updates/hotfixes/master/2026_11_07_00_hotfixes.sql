-- Spread the Word (mission 1210): its condition is named by the client data but not held. A condition with no row counts
-- as met, so without this the mission would sit on the table from the start. While 43265 is in the log and not complete
-- (65537, as for the demon hunter's campaign missions in 2026_11_02_01_hotfixes.sql); -1 is "any".
DELETE FROM `player_condition` WHERE `ID`=41568;
INSERT INTO `player_condition` (`ID`, `MinLevel`, `CurrQuestLogic`, `CurrQuestID1`, `CurrentCompletedQuestLogic`, `CurrentCompletedQuestID1`, `Gender`, `NativeGender`, `MinExpansionLevel`, `MaxExpansionLevel`, `MinExpansionTier`, `MaxExpansionTier`, `ChrSpecializationIndex`, `ChrSpecializationRole`, `PowerType`, `VerifiedBuild`) VALUES
(41568, 0, 1, 43265, 65537, 43265, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0); -- mission 1210 while Spread the Word is in the log
