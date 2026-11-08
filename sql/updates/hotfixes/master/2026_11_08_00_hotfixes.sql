-- Tech It Up A Notch's "Class hall upgrade chosen" is a tree the client holds without a criterion, as the demon hunter's
-- Demonic Improvements (2026_11_02_00_hotfixes.sql): any research started credits it
DELETE FROM `criteria` WHERE `ID`=39860;
INSERT INTO `criteria` (`ID`, `Asset`, `Type`, `Flags`, `VerifiedBuild`) VALUES
(39860, 0, 201, 32, 0);

DELETE FROM `criteria_tree` WHERE `ID`=65559;
INSERT INTO `criteria_tree` (`ID`, `Description`, `Amount`, `Flags`, `Operator`, `CriteriaID`, `Parent`, `OrderIndex`, `VerifiedBuild`) VALUES
(65559, '', 1, 0, 0, 39860, 50190, 0, 0);

-- Salanar the Horseman (mission 1259): as for Troops in the Field's 1211 in 2026_11_07_01_hotfixes.sql, the condition the
-- client names is not held, and a condition with no row counts as met. While 43539 is in the log and not complete;
-- -1 is "any".
DELETE FROM `player_condition` WHERE `ID`=42090;
INSERT INTO `player_condition` (`ID`, `MinLevel`, `CurrQuestLogic`, `CurrQuestID1`, `CurrentCompletedQuestLogic`, `CurrentCompletedQuestID1`, `Gender`, `NativeGender`, `MinExpansionLevel`, `MaxExpansionLevel`, `MinExpansionTier`, `MaxExpansionTier`, `ChrSpecializationIndex`, `ChrSpecializationRole`, `PowerType`, `VerifiedBuild`) VALUES
(42090, 0, 1, 43539, 65537, 43539, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0); -- mission 1259 while Salanar the Horseman is in the log
