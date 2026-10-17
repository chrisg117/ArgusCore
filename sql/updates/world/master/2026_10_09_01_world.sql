-- Broken Shore scenario (map 1460), "Defeat the Commander": how Dread Commander Arganoth (90705) arrives, what
-- he and the Alliance's leaders say, and the infernals he calls down.
--
-- The scenario's lines are broadcast texts whose sounds are numbered in the order they are spoken. Whose a
-- line is, is not in the data: the women's are Jaina's, and which of the men's are Genn's and which the
-- commander's follows from what they say and from a video of the retail scenario. Text, emote and sound are
-- those of each line's broadcast text.
UPDATE `creature_template` SET `ScriptName`='npc_broken_shore_dread_commander_arganoth' WHERE `entry`=90705;

DELETE FROM `creature_text` WHERE `CreatureID`=90705;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(90705, 0, 0, 'You wish to face the might of the Legion? Ha, very well. The master will reward me for your souls.', 14, 0, 100, 0, 0, 53368, 0, 94639, 0, 'Dread Commander Arganoth - before he arrives'),
(90705, 1, 0, 'The legion will crush you! INFERNO!', 14, 0, 100, 0, 0, 53371, 0, 95056, 0, 'Dread Commander Arganoth - calls an infernal down'),
(90705, 2, 0, 'Your souls will fuel the soul forge!', 14, 0, 100, 0, 0, 53373, 0, 95059, 0, 'Dread Commander Arganoth - in the fight'),
(90705, 2, 1, 'I will flay the flesh from your bones!', 14, 0, 100, 0, 0, 53374, 0, 95060, 0, 'Dread Commander Arganoth - in the fight'),
(90705, 2, 2, 'You should have stayed home!', 14, 0, 100, 0, 0, 53375, 0, 95567, 0, 'Dread Commander Arganoth - in the fight'),
(90705, 3, 0, 'The master will mend my flesh again and again. You have not won. You cannot win. We are endless! We are Legion!', 14, 0, 100, 0, 0, 53376, 0, 95568, 0, 'Dread Commander Arganoth - at low health'),
(90705, 4, 0, 'I... will... return...', 14, 0, 100, 0, 0, 53377, 0, 95953, 0, 'Dread Commander Arganoth - death');

DELETE FROM `creature_text` WHERE (`CreatureID`=90714 AND `GroupID` BETWEEN 5 AND 8) OR (`CreatureID`=90717 AND `GroupID` BETWEEN 6 AND 8);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(90714, 5, 0, 'We''ve got him now! Focus on their commander!', 14, 0, 100, 0, 0, 53370, 0, 95055, 0, 'Lady Jaina Proudmoore - the commander has arrived'),
(90714, 6, 0, 'Look out, infernals!', 14, 0, 100, 0, 0, 53372, 0, 95058, 0, 'Lady Jaina Proudmoore - the commander''s infernals'),
(90714, 7, 0, 'Is everyone alright?', 14, 0, 100, 0, 0, 53378, 0, 95954, 0, 'Lady Jaina Proudmoore - the commander is slain'),
(90714, 8, 0, 'We''ll have to mourn them later, we need to move before more demons arrive.', 14, 0, 100, 0, 0, 53380, 0, 95958, 0, 'Lady Jaina Proudmoore - answers Genn after the commander'),
(90717, 6, 0, 'Finally, enough of your chatter.', 14, 0, 100, 0, 0, 53369, 0, 94641, 0, 'Genn Greymane - answers the commander'),
(90717, 7, 0, 'Singed, but alive. We lost many good troops.', 14, 0, 100, 0, 0, 53379, 0, 95955, 0, 'Genn Greymane - answers Jaina after the commander'),
(90717, 8, 0, 'Agreed.', 14, 0, 100, 0, 0, 53381, 0, 95959, 0, 'Genn Greymane - agrees to move on');

-- The Felblaze Infernal that "Summon Felblaze Infernal" (183956) brings down still had the placeholder faction
-- 35 and level 1. It gets the faction of the scenario's demons and their level range and scaling.
UPDATE `creature_template` SET `faction`=2780 WHERE `entry`=93060;
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `DifficultyID`=0 AND `Entry`=93060;
