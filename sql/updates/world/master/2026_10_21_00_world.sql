-- Enter the Illidari: Coilskar (40379): the Ashtongue Mystic (99914) had its gossip flag but no menu linked, so
-- Player::CanSeeGossipOn hid the flag and the Mystic could not be talked to. Menu 19015, its option, conditions,
-- the gossip SmartAI (spell 196724) and the event 47548 script that gives the "Soul sacrificed" credit were all present.
DELETE FROM `creature_template_gossip` WHERE `CreatureID` = 99914 AND `MenuID` = 19015;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(99914, 19015, 0);
