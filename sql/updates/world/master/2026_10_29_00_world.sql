-- To Be Prepared (42782): "Warmed up with a duel" had no way to be done. The duelists in Stormwind Harbor had the retail
-- gossip ("How about a duel?" / "Let's duel.") but no gossip flag, and nothing ran the duel; they are friendly (35), so
-- the kill credit could never happen. npc_to_be_prepared_duelist now duels the player who picks "Let's duel." (3 s
-- countdown, then hostile to players only, faction 168, so the harbor guards stay out of it) and yields at 10% health
-- with one of the client's duel lines, giving the credit (108722). The option shows only while the objective is open.
-- Only the 22 duelists that are spawned are scripted; 113542-113961 (also crediting 108722) have no spawns.
-- Not built: the duel flag and the countdown's look (no Legion duel spell found in the client data).

UPDATE `creature_template` SET `npcflag` = `npcflag` | 1, `ScriptName` = 'npc_to_be_prepared_duelist' WHERE `entry` IN (108722, 108723, 108742, 108743, 108744, 108745, 108746, 108747, 108748, 108749, 108750, 108751, 108752, 108753, 108754, 108755, 108756, 108757, 108758, 108759, 108765, 108767);

-- Three duelists had no menu; they get the others' one
DELETE FROM `creature_template_gossip` WHERE `CreatureID` IN (108723, 108765, 108767);
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(108723, 19861, 0),
(108765, 19861, 0),
(108767, 19861, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 19861 AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 19861, 0, 0, 0, 47, 0, 42782, 8, 0, '', 0, 0, 0, '', 'Let''s duel.: To Be Prepared in progress'),
(15, 19861, 0, 0, 0, 48, 0, 284739, 0, 1, '', 1, 0, 0, '', 'Let''s duel.: not yet warmed up with a duel');

DELETE FROM `creature_text` WHERE `CreatureID` IN (108722, 108723, 108742, 108743, 108744, 108745, 108746, 108747, 108748, 108749, 108750, 108751, 108752, 108753, 108754, 108755, 108756, 108757, 108758, 108759, 108765, 108767);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(108722, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108722, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108723, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108723, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108742, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108742, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108743, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108743, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108744, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108744, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108745, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108745, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108746, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108746, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108747, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108747, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108748, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108748, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108749, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108749, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108750, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108750, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108751, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108751, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108752, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108752, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108753, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108753, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108754, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108754, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108755, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108755, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108756, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108756, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108757, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108757, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108758, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108758, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108759, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108759, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108765, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108765, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields'),
(108767, 0, 0, 'Thank you for the duel.', 12, 0, 50, 0, 0, 0, 0, 114452, 0, 'To Be Prepared duelist - yields'),
(108767, 0, 1, 'Fine duel.', 12, 0, 50, 0, 0, 0, 0, 114457, 0, 'To Be Prepared duelist - yields');
