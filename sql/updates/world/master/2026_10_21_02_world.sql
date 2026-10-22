-- Orders for Your Captains (38813): the three captains had their gossip flag but no menu linked, so
-- Player::CanSeeGossipOn hid the flag and none of them could be briefed. Each briefing menu, its conditions and the
-- gossip SmartAI that gives the "briefed" credit were already present.
DELETE FROM `creature_template_gossip` WHERE (`CreatureID` = 90247 AND `MenuID` = 17260) OR (`CreatureID` = 93693 AND `MenuID` = 18447) OR (`CreatureID` = 94435 AND `MenuID` = 18434);
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(90247, 17260, 0), -- Battlelord Gaardoun
(93693, 18447, 0), -- Lady S'theno
(94435, 18434, 0); -- Matron Mother Malevolence
