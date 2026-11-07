-- DK class hall campaign, slice 1: the quests' turn-in texts, which were blank (the 2026-11-07 pass).

-- Turn-in texts, from Wowhead's Progress and Completion sections; every quest of the slice was looked at. Rise, Champions
-- and the two Champion quests have no Progress text there, so they get none
DELETE FROM `quest_offer_reward` WHERE `ID` IN (42449, 42484, 43264, 39816, 39818, 43265);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(42449, 1, 0, 0, 0, 0, 0, 0, 0, 'The world of the living will forever revile us for the dark task that we begin this day...', 0),
(42484, 1, 0, 0, 0, 0, 0, 0, 0, 'Well done, Deathlord. One of the Horde''s greatest generals in life now serves you in undeath as your first horseman!', 0),
(43264, 1, 0, 0, 0, 0, 0, 0, 0, 'Well done, Deathlord. Nazgrim and Thassarian are powerful champions. I am sure they will carry out their orders with deadly precision.', 0),
(39816, 1, 0, 0, 0, 0, 0, 0, 0, 'You and I have been through much together, $n. Now, you stand before me as Deathlord and leader of our order. I could not be more proud!$B$BThe battle against the Burning Legion will be hard fought and filled with peril. You should not embark on this alone, my friend.$B$BI will go wherever you command and I will kill whoever you wish me to kill.', 0),
(39818, 1, 0, 0, 0, 0, 0, 0, 0, 'These foolish class trainers speak to me as if I have never held an axe before!$B$BThey have nothing to teach that I do not already know. The dark power of undeath flows through my veins and it seeks blood and battle!$B$BLet''s waste no more time in this dusty necropolis! Send me to the front so that I may spill the blood of our enemies!', 0),
(43265, 1, 0, 0, 0, 0, 0, 0, 0, 'It appears that Nazgrim and Thassarian were successful in their mission. We have identified several promising candidates to aid us in the war against the Burning Legion.', 0);

DELETE FROM `quest_request_items` WHERE `ID` IN (42449, 42484, 43264, 39816, 39818, 43265);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(42484, 1, 1, 0, 0, 'General Nazgrim will make a fine $c.', 0),
(43265, 6, 6, 0, 0, 'What is the status of the mission?', 0);
