-- Broken Shore scenario (map 1460), "Find Varian": the script of the stage's cutscene, what is said outside
-- it, and the script of the Fel Meteors on the way to him.
--
-- The cutscene is scene 1356, which the spell "Stage 2 Scene" (218626) plays. It asks for a look at Varian's
-- forces and for the player to be taken to the hilltop it looks out from, which its script sees to.
-- Its flags, which were sniffed, say that it cannot be skipped (0x04), but a video of the retail scenario
-- shows a player skip it, so that flag is taken off. A player who skips it is taken to the hilltop at once.
UPDATE `scene_template` SET `Flags`=`Flags`&~0x04, `ScriptName`='scene_broken_shore_alliance_stage_2' WHERE `SceneId`=1356;

-- Text, emote and sound are those of each line's broadcast text. Whose a line is, is not in the data: it
-- follows from what is said, and for Genn's first two and Jaina's from a video of the retail scenario, which
-- also shows that the first is said and the next two are yelled. Those that are said reach everyone in the
-- scenario, because players are spread out between the beach and Varian by then.
DELETE FROM `creature_text` WHERE `CreatureID`=90713 OR (`CreatureID`=90714 AND `GroupID`=9) OR (`CreatureID`=90717 AND `GroupID` BETWEEN 9 AND 11);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(90713, 0, 0, 'Genn, Jaina, it''s good to see you safe.', 12, 0, 100, 0, 0, 53390, 0, 96107, 3, 'King Varian Wrynn - greets Jaina and Genn'),
(90714, 9, 0, 'Varian!', 14, 0, 100, 0, 0, 53388, 0, 96049, 0, 'Lady Jaina Proudmoore - sees Varian from the hilltop'),
(90717, 9, 0, 'Varian''s forces should have landed just around this hill, let''s hope they had more luck.', 12, 0, 100, 0, 0, 53382, 0, 95960, 3, 'Genn Greymane - sets off for Varian'),
(90717, 10, 0, 'Let''s go!', 14, 0, 100, 0, 0, 53389, 0, 96106, 0, 'Genn Greymane - on the hilltop'),
(90717, 11, 0, 'And you.', 12, 0, 100, 0, 0, 53391, 0, 99175, 3, 'Genn Greymane - answers Varian');

-- "Fel Meteor" (199036) is what lands where the stalkers above the way to Varian send their meteors. Its
-- script leaves it players, and has it kill them.
DELETE FROM `spell_script_names` WHERE `spell_id`=199036 AND `ScriptName`='spell_broken_shore_fel_meteor';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(199036, 'spell_broken_shore_fel_meteor');
