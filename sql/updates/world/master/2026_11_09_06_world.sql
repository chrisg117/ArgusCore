-- Quest texts, priest artifact and Netherlight Temple opening: offer (quest_offer_reward), progress (quest_request_items) and detail emotes
-- (quest_details), copied by quest id. Written only where the world DB had no row.
-- lint: texts pending 40705 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40710 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41017 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41019 no progress text in the source world data (the quest likely has none on retail); the user pass decides

DELETE FROM `quest_offer_reward` WHERE `ID` IN (40705, 41019);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(40705, 1, 0, 0, 0, 0, 0, 0, 0, 'Как я понимаю, ты $gсогласен:согласна;? Если ты $gготов:готова; действовать, предлагаю не терять зря времени. Я не сомневаюсь в твоих талантах, но в нашей ситуации мы не можем себе позволить полагаться на волю случая!', 21287),
(41019, 1, 0, 0, 0, 0, 0, 0, 0, 'Прекрасный выбор! Если что, я всегда окажу содействие в твоих персональных заданиях и на Расколотых островах. Не забудь поручить мне какое-нибудь дело!', 21287);
DELETE FROM `quest_details` WHERE `ID` IN (40705, 40710, 40938, 41015, 41017, 41019);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(40705, 1, 0, 0, 0, 0, 0, 0, 0, 21287),
(40710, 1, 0, 0, 0, 0, 0, 0, 0, 21287),
(40938, 1, 0, 0, 0, 0, 0, 0, 0, 21287),
(41015, 1, 0, 0, 0, 0, 0, 0, 0, 21287),
(41017, 1, 0, 0, 0, 0, 0, 0, 0, 21287),
(41019, 1, 0, 0, 0, 0, 0, 0, 0, 21287);
