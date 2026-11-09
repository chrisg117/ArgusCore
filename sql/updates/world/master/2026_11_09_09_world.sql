-- Quest texts, zone introductions (Highmountain, Stormheim, Val'sharah) reached from the Command Maps: offer (quest_offer_reward), progress (quest_request_items) and detail emotes
-- (quest_details), copied by quest id. Written only where the world DB had no row.
-- lint: giver 39864 no starter in the data or the fork; outside this texts-only file (listed for its chain)
-- lint: texts pending 39864 no progress text in the source world data (the quest likely has none on retail); the user pass decides

DELETE FROM `quest_offer_reward` WHERE `ID` IN (38307, 39864);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(38307, 0, 0, 0, 0, 0, 0, 0, 0, '', 21232),
(39864, 0, 0, 0, 0, 0, 0, 0, 0, '', 21232);
DELETE FROM `quest_request_items` WHERE `ID` IN (38307);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(38307, 0, 0, 0, 0, 'If only she hadn''t ordered me to take you to her.$gsafe and sound:safe and sound;...', 21215);
DELETE FROM `quest_details` WHERE `ID` IN (38307, 39864);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(38307, 0, 0, 0, 0, 0, 0, 0, 0, 21232),
(39864, 0, 0, 0, 0, 0, 0, 0, 0, 21232);
