-- Death knight Acherus opening: progress texts (quest_request_items) and any missing offer texts or detail emotes for
-- 39757, 39761 and 39799, which lint found without progress text. Written only where the world DB had no row.
-- lint: texts pending 39757 no progress text in the source world data (the quest likely has none on retail); the user pass decides


DELETE FROM `quest_request_items` WHERE `ID` IN (39761, 39799);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(39761, 1, 0, 0, 0, 'Твое оружие обладает огромным нераскрытым потенциалом. Возвращайся, когда оно поглотит еще больше душ.', 21287),
(39799, 2, 0, 0, 0, 'Привет тебе, $ct.', 21287);
DELETE FROM `quest_details` WHERE `ID` IN (39757, 39761, 39799);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(39757, 1, 1, 0, 0, 0, 0, 0, 0, 21287),
(39761, 1, 1, 0, 0, 0, 0, 0, 0, 21287),
(39799, 1, 1, 0, 0, 0, 0, 0, 0, 21287);
