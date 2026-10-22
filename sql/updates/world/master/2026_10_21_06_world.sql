-- Give Me Sight Beyond Sight (39262): Jace Darkweaver at Cryptic Hollow (96436) had no gossip menu linked, so the
-- "Speak with Jace Darkweaver" option (menu 19175, handled by npc_jayce_darkweaver_cryptic_hollow) never showed.
DELETE FROM `creature_template_gossip` WHERE `CreatureID` = 96436 AND `MenuID` = 19175;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(96436, 19175, 0);
