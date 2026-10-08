-- Legion intro chain: The Legion Returns (40519) -> To Be Prepared (42782) -> The Battle for Broken Shore (42740).
-- quest_template.RewardNextQuest already links the three, but neither follow-up had a starter,
-- so the chain stopped after the first turn-in. Each is offered by the NPC that ends the quest before it.
DELETE FROM `creature_queststarter` WHERE (`id`=107934 AND `quest`=42782) OR (`id`=108916 AND `quest`=42740);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(107934, 42782, 0), -- Recruiter Lee - To Be Prepared
(108916, 42740, 0); -- Knight Dameron - The Battle for Broken Shore

DELETE FROM `quest_template_addon` WHERE `ID` IN (42782, 42740);
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(42782, 40519),
(42740, 42782);

-- Captain Angelica (108920) is the credit for the objective "Ship taken to the Broken Shore", but players could
-- not talk to her: she had no gossip flag and no gossip menu. Player::CanSeeGossipOn hides the flag from players
-- unless a menu is linked, so both are needed.
-- Her menu is already in the database, only the link to her was missing: menu 19870 with greeting 29515
-- ("Welcome aboard, $n. ...") and the option "I am ready to face the Legion.".
UPDATE `creature_template` SET `npcflag`=`npcflag`|1 WHERE `entry`=108920;

DELETE FROM `creature_template_gossip` WHERE `CreatureID`=108920;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(108920, 19870, 0);

-- Only offer the option while "The Battle for Broken Shore" (42740) is in the quest log
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=19870 AND `SourceEntry`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 19870, 0, 0, 0, 9, 0, 42740, 0, 0, 0, 0, 0, '', 'Captain Angelica - Show gossip option 0 if quest The Battle for Broken Shore is taken');
