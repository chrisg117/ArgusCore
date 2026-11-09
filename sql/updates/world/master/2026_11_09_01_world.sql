-- Warlock opening quests: offer (quest_offer_reward) and progress (quest_request_items) texts, and the givers' detail emotes
-- (quest_details), for the ten quests from The Sixth to The Path of the Dreadscar. Written only where the world DB had
-- no row.
-- lint: texts pending 40684 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40729 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40731 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40821 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 43153 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 43254 no progress text in the source world data (the quest likely has none on retail); the user pass decides


DELETE FROM `quest_offer_reward` WHERE `ID` IN (40684, 40716, 40729, 40731, 40821, 40823, 40824, 43100, 43153, 43254);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(40684, 0, 0, 0, 0, 0, 0, 0, 0, 'Yes, the choice is worth considering!', 21287),
(40716, 0, 0, 0, 0, 0, 0, 0, 0, 'Ты $gпринял:приняла; правильное решение. $B$BНадеюсь, ты действительно так же $gхорош:хороша; в деле, как о тебе рассказывают.', 21287),
(40729, 0, 0, 0, 0, 0, 0, 0, 0, 'Калиду жаль, что так вышло с твоими друзьями, но если мы поторопимся, они еще могут выжить. $B$BДа, Джагганот будет их пытать, но убивать своих пленников он не спешит.', 21287),
(40731, 0, 0, 0, 0, 0, 0, 0, 0, '', 21232),
(40821, 0, 0, 0, 0, 0, 0, 0, 0, '', 21232),
(40823, 0, 0, 0, 0, 0, 0, 0, 0, '', 21232),
(40824, 0, 0, 0, 0, 0, 0, 0, 0, '', 21232),
(43100, 0, 0, 0, 0, 0, 0, 0, 0, '', 21491),
(43153, 0, 0, 0, 0, 0, 0, 0, 0, '', 21491),
(43254, 0, 0, 0, 0, 0, 0, 0, 0, '', 21491);
DELETE FROM `quest_request_items` WHERE `ID` IN (40823, 40824, 43100);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(40823, 0, 0, 0, 0, 'I will not rest until the council has been rebuilt.', 21232),
(40824, 0, 0, 0, 0, 'Meet me at the scrying pool, and we will plan our action from there.', 21232),
(43100, 1, 0, 0, 0, 'This gets complicated, but Calydus have idea!', 21491);
DELETE FROM `quest_details` WHERE `ID` IN (40684, 40716, 40729, 40731, 40821, 40823, 40824, 43100, 43153, 43254);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(40684, 0, 0, 0, 0, 0, 0, 0, 0, 21232),
(40716, 0, 0, 0, 0, 0, 0, 0, 0, 21287),
(40729, 0, 0, 0, 0, 0, 0, 0, 0, 21232),
(40731, 0, 0, 0, 0, 0, 0, 0, 0, 21232),
(40821, 0, 0, 0, 0, 0, 0, 0, 0, 21232),
(40823, 0, 0, 0, 0, 0, 0, 0, 0, 21232),
(40824, 0, 0, 0, 0, 0, 0, 0, 0, 21232),
(43100, 1, 0, 0, 0, 0, 0, 0, 0, 21491),
(43153, 1, 0, 0, 0, 0, 0, 0, 0, 21491),
(43254, 1, 0, 0, 0, 0, 0, 0, 0, 21491);
