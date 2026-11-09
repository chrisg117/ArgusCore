-- Quest texts, death knight Call to War, artifact, Acherus and campaign slices 1-4: offer (quest_offer_reward), progress (quest_request_items) and detail emotes
-- (quest_details), copied by quest id. Written only where the world DB had no row.
-- lint: giver 40714 no starter in the data or the fork; outside this texts-only file (listed for its chain)
-- lint: texts pending 40715 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 42535 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 43264 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 43268 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 43539 no progress text in the source world data (the quest likely has none on retail); the user pass decides

DELETE FROM `quest_offer_reward` WHERE `ID` IN (40714, 40715);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(40714, 1, 0, 0, 0, 0, 0, 0, 0, 'У тебя, наверное, полно вопросов, но с ответами на них придется подождать. Идет война!$b$bМедлить нельзя ни минуты, но я постараюсь вкратце обрисовать тебе ситуацию.', 21287),
(40715, 1, 0, 0, 0, 0, 0, 0, 0, 'That''s a great choice.', 21287);
DELETE FROM `quest_details` WHERE `ID` IN (38990, 39832, 40714, 40715, 42449, 42484, 42533, 42534, 42535, 42536, 42537, 43264, 43265, 43266, 43267, 43268, 43539);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(38990, 0, 0, 0, 0, 0, 0, 0, 0, 21287),
(39832, 1, 0, 0, 0, 0, 0, 0, 0, 21287),
(40714, 1, 0, 0, 0, 0, 0, 0, 0, 21287),
(40715, 1, 0, 0, 0, 0, 0, 0, 0, 21287),
(42449, 0, 0, 0, 0, 0, 0, 0, 0, 23420),
(42484, 0, 0, 0, 0, 0, 0, 0, 0, 23420),
(42533, 0, 0, 0, 0, 0, 0, 0, 0, 23420),
(42534, 0, 0, 0, 0, 0, 0, 0, 0, 23420),
(42535, 0, 0, 0, 0, 0, 0, 0, 0, 23420),
(42536, 0, 0, 0, 0, 0, 0, 0, 0, 23420),
(42537, 0, 0, 0, 0, 0, 0, 0, 0, 23420),
(43264, 1, 1, 0, 0, 0, 0, 0, 0, 23420),
(43265, 1, 1, 0, 0, 0, 0, 0, 0, 23420),
(43266, 1, 1, 0, 0, 0, 0, 0, 0, 23420),
(43267, 1, 1, 0, 0, 0, 0, 0, 0, 23420),
(43268, 1, 1, 0, 0, 0, 0, 0, 0, 23420),
(43539, 0, 0, 0, 0, 0, 0, 0, 0, 23420);
