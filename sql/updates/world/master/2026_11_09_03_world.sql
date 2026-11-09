-- Quest texts, Alliance Stormwind and Broken Shore opening (with the demon hunter variants): offer (quest_offer_reward), progress (quest_request_items) and detail emotes
-- (quest_details), copied by quest id. Written only where the world DB had no row.
-- lint: texts pending 39691 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 44463 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 44471 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 44663 no progress text in the source world data (the quest likely has none on retail); the user pass decides

DELETE FROM `quest_offer_reward` WHERE `ID` IN (39691, 44463, 44471, 44473, 44663);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(39691, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark times have fallen upon Stormwind, $n.$B$BWith King Varian Wrynn slain on the Broken Shore, uncertainty and tension grip the Alliance leaders.$B$BThe timing of this tragedy is no accident. The Burning Legion seeks to sow disorder amongst its enemies, weakening its prey from within.$B$BWe must act.', 0),
(44463, 0, 0, 0, 0, 0, 0, 0, 0, 'I must admit, you saved us from what might have been a terrible tragedy. You have my gratitude, $C.', 0),
(44471, 0, 0, 0, 0, 0, 0, 0, 0, 'Of course the Legion would try to infiltrate this funeral. Having all the leaders of the Alliance gathered in one place at one time... it is too tasty a target for the demons to resist!', 0),
(44473, 0, 0, 0, 0, 0, 0, 0, 0, 'I trust the young king is safe.$B$BWe have more work ahead of us, $C. Much more.', 0),
(44663, 1, 1, 0, 0, 0, 0, 0, 0, '', 21963);
DELETE FROM `quest_request_items` WHERE `ID` IN (40517);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(40517, 1, 0, 0, 0, 'Welcome, $n. I am pleased you survived the battle. You have something for me?', 21491);
DELETE FROM `quest_details` WHERE `ID` IN (40519, 44663);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(40519, 0, 0, 0, 0, 0, 0, 0, 0, 21863),
(44663, 1, 1, 0, 0, 0, 500, 0, 0, 21952);
