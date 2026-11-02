-- The conditions of the demon hunter's troops and campaign missions, which the client data names but does not hold.
-- A condition with no row counts as met, so without these the troops would show their upgraded names at once.
DELETE FROM `player_condition` WHERE `ID` IN (42844, 41636, 41635, 41637, 43443, 40713, 41521);
INSERT INTO `player_condition` (`ID`, `MinLevel`, `CurrQuestLogic`, `CurrQuestID1`, `VerifiedBuild`) VALUES
(42844, 0, 0, 0, 0),     -- Ashtongue Warriors
(41636, 0, 0, 0, 0),     -- Illidari Adepts
(41635, 111, 0, 0, 0),   -- Naga Myrmidon: no 7.3.5 character reaches it; to be wired to talent 417 of the third tier
(41637, 111, 0, 0, 0),   -- Transformed Illidari Adepts: the same; talent 418
(43443, 0, 1, 42677, 0), -- mission 953 while Things Gaardoun Needs is in the log
(40713, 0, 1, 42681, 0), -- mission 954 while Loramus, Is That You? is in the log
(41521, 0, 1, 42682, 0); -- mission 1155 while Additional Accoutrements is in the log

-- Demonic Improvements' "Class hall upgrade chosen" is a tree the client holds without a criterion: any research started
DELETE FROM `criteria` WHERE `ID`=39859;
INSERT INTO `criteria` (`ID`, `Asset`, `Type`, `Flags`, `VerifiedBuild`) VALUES
(39859, 0, 201, 32, 0);

DELETE FROM `criteria_tree` WHERE `ID`=65558;
INSERT INTO `criteria_tree` (`ID`, `Description`, `Amount`, `Flags`, `Operator`, `CriteriaID`, `Parent`, `OrderIndex`, `VerifiedBuild`) VALUES
(65558, '', 1, 0, 0, 39859, 50184, 0, 0);
