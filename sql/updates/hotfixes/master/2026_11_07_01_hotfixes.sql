-- Troops in the Field (mission 1211): as for Spread the Word's 1210 in 2026_11_07_00_hotfixes.sql, the condition the client
-- names is not held, and a condition with no row counts as met. While 43267 is in the log and not complete; -1 is "any".
DELETE FROM `player_condition` WHERE `ID`=41569;
INSERT INTO `player_condition` (`ID`, `MinLevel`, `CurrQuestLogic`, `CurrQuestID1`, `CurrentCompletedQuestLogic`, `CurrentCompletedQuestID1`, `Gender`, `NativeGender`, `MinExpansionLevel`, `MaxExpansionLevel`, `MinExpansionTier`, `MaxExpansionTier`, `ChrSpecializationIndex`, `ChrSpecializationRole`, `PowerType`, `VerifiedBuild`) VALUES
(41569, 0, 1, 43267, 65537, 43267, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0); -- mission 1211 while Troops in the Field is in the log
