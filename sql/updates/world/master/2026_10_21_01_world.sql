-- Enter the Illidari: Shivarra (38765): Sevis Brightflame (99915) had its gossip flag but no menu linked, so
-- Player::CanSeeGossipOn hid the flag and she could not be talked to. 19016 is the top menu: its options open
-- 19133 (sacrifice Sevis) and 19132 (sacrifice me), which npc_sevis_brightflame_shivarra_gateway already handles.
DELETE FROM `creature_template_gossip` WHERE `CreatureID` = 99915 AND `MenuID` = 19016;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(99915, 19016, 0);
