-- Quest texts, demon hunter artifact, Fel Hammer opening and hall campaign slice 1: offer (quest_offer_reward), progress (quest_request_items) and detail emotes
-- (quest_details), copied by quest id. Written only where the world DB had no row.
-- lint: texts pending 39247 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40814 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40816 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41033 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41037 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41060 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: poi 41062 no quest_poi row in the data; outside this texts-only file (listed for its chain)
-- lint: texts pending 41062 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41066 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41069 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41070 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41099 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41119 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41121 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 41221 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 42670 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 42671 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 42677 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: poi 42679 no quest_poi row in the data; outside this texts-only file (listed for its chain)
-- lint: texts pending 42679 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 42681 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 42682 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 42683 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: poi 42690 no quest_poi row in the data; outside this texts-only file (listed for its chain)
-- lint: poi 42695 no quest_poi row in the data; outside this texts-only file (listed for its chain)
-- lint: poi 42697 no quest_poi row in the data; outside this texts-only file (listed for its chain)
-- lint: texts pending 42869 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 42872 no progress text in the source world data (the quest likely has none on retail); the user pass decides

DELETE FROM `quest_offer_reward` WHERE `ID` IN (39047, 39051, 39247, 39261, 40814, 40816, 40819, 41119, 41120, 41121, 42670, 42671, 42677, 42679, 42681, 42682, 42683, 42690, 42695, 42697);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(39047, 0, 0, 0, 0, 0, 0, 0, 0, '', 21287),
(39051, 0, 0, 0, 0, 0, 0, 0, 0, 'Suramar is not far from here.', 20994),
(39247, 0, 0, 0, 0, 0, 0, 0, 0, 'What of the others? Did they...?$B$BI see. They gave their lives for a worthy cause - the Twinblades will change the tide of this war, mark my words.', 20994),
(39261, 1, 0, 0, 0, 0, 0, 0, 0, 'Хорошо, что Кор''вас быстро нашла тебя, $gповелитель:повелительница;.', 20994),
(40814, 1, 0, 0, 0, 0, 0, 0, 0, 'Well chosen. There isn''t a moment to spare.', 20994),
(40816, 0, 0, 0, 0, 0, 0, 0, 0, '', 21287),
(40819, 1, 0, 0, 0, 0, 0, 0, 0, 'I hold little love for these mages and their ilk, but they are necessary allies.', 20994),
(41119, 0, 0, 0, 0, 0, 0, 0, 0, 'What of the others? Did they...?$B$BI see. They gave their lives for a worthy cause - the Twinblades will change the tide of this war, mark my words.', 0),
(41120, 0, 0, 0, 0, 0, 0, 0, 0, 'Let''s get this done with. These bureaucrats already test my patience.', 0),
(41121, 0, 0, 0, 0, 0, 0, 0, 0, 'Suramar? That is only a short flight from here.', 0),
(42670, 1, 0, 0, 0, 0, 0, 0, 0, 'Тебе удалось завербовать первых двух защитников. Впереди нас ждут великие дела, $n.\n\n<Кор''вас бормочет себе под нос, что-то ругательное и связанное с тем, что ее не выбрали.>', 23222),
(42671, 1, 0, 0, 0, 0, 0, 0, 0, 'Тебе удалось завербовать первых двух защитников. Впереди нас ждут великие дела, $n.\n\n<Кор''вас бормочет себе под нос, что-то ругательное и связанное с тем, что ее не выбрали.>', 23222),
(42677, 1, 0, 0, 0, 0, 0, 0, 0, 'Ну, теперь хотя бы увидим новые лица. Если так можно сказать про сломленных.\n\n<Кор''вас ухмыляется.>', 23222),
(42679, 273, 0, 0, 0, 0, 0, 0, 0, 'Превосходно. С этими Пеплоустами ты сможешь размещать войска там, где сочтешь нужным.', 23222),
(42681, 1, 0, 0, 0, 0, 0, 0, 0, 'Они вернулись и привели его. Это Лорам, да. Но он заперт в теле повелителя ужаса Разелиха Осквернителя!\n\nПока мы не убедимся, что он не представляет опасности, по моему приказу его будут держать в ловушке для демонов.', 23222),
(42682, 6, 0, 0, 0, 0, 0, 0, 0, 'Нам правда нужно все это, чтобы установить контакт с душой Иллидана?', 23222),
(42683, 1, 274, 0, 0, 0, 0, 0, 0, 'Это добрые вести. Он начинает вносить вклад в общее дело, и мы с его помощью становимся гораздо сильнее.\n\nОднако он не может полностью контролировать Разелиха. Думаю, будет лучше оставить его в ловушке, пока не подыщем для Лорама новое тело. Как считаешь?\n\nИ если он будет доставлять слишком много хлопот, мы примем другие меры.', 23222),
(42690, 0, 0, 0, 0, 0, 0, 0, 0, 'You honor me, $n. Of course I will serve as your champion and right-hand man. How could I refuse?$B$BWe have only just begun to change the way in which the Illidari operate, but already, you have made great strides. If we are to defeat the Burning Legion, it must be done in concert with everyone working together. Not just the Illidari standing alone.$B$BAnd not standing upon the smoking husk of a world that we have carelessly destroyed while trying to save it.', 0),
(42695, 1, 273, 0, 0, 0, 0, 0, 0, 'Это честь для меня, $n. Конечно, я стану твоим защитником и помощником. Разве может быть иначе?\n\nМы только начали воплощать в жизнь разработанный план, но тебе уже удалось сделать большие шаги вперед. Если мы хотим одолеть Пылающий Легион, то не должны терять бдительность. Все прочие цели меркнут в сравнении с этой. На чаше весов – судьба всего сущего.\n\nВ конце концов победа будет нашей... и неважно, какой ценой она нам достанется!', 23222),
(42697, 6, 1, 0, 0, 0, 1000, 0, 0, 'Ты хочешь, чтобы я стала одним из твоих защитников?\n\n<Аша какое-то время раздумывает.>\n\nЯ соглашусь, но при одном условии. Я хочу на поле боя. Хоть от меня и есть польза здесь, на "Молоте Скверны", мои таланты больше пригодились бы в Азероте.\n\nКроме того, мы ведь по-прежнему собираемся поставить Пылающий Легион на колени?', 23222);
DELETE FROM `quest_request_items` WHERE `ID` IN (39051, 41064);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(39051, 1, 0, 0, 0, 'A truly impressive display.', 20994),
(41064, 1, 0, 0, 0, 'Your blades... they glow with such power...', 20994);
DELETE FROM `quest_details` WHERE `ID` IN (39047, 39051, 39247, 39261, 40814, 40816, 40819, 41033, 41037, 41060, 41062, 41064, 41066, 41067, 41069, 41070, 41096, 41099, 41221, 42671, 42677, 42679, 42681, 42682, 42683, 42869, 42872);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(39047, 0, 0, 0, 0, 0, 0, 0, 0, 21287),
(39051, 1, 1, 0, 0, 0, 0, 0, 0, 20994),
(39247, 1, 1, 1, 0, 0, 0, 0, 0, 20994),
(39261, 1, 1, 0, 0, 0, 0, 0, 0, 20994),
(40814, 1, 0, 0, 0, 0, 0, 0, 0, 20994),
(40816, 0, 0, 0, 0, 0, 0, 0, 0, 21287),
(40819, 1, 1, 0, 0, 0, 0, 0, 0, 20994),
(41033, 0, 0, 0, 0, 0, 0, 0, 0, 21287),
(41037, 0, 0, 0, 0, 0, 0, 0, 0, 20994),
(41060, 0, 0, 0, 0, 0, 0, 0, 0, 21287),
(41062, 1, 1, 0, 0, 0, 0, 0, 0, 20994),
(41064, 1, 0, 0, 0, 0, 0, 0, 0, 20994),
(41066, 1, 0, 0, 0, 0, 0, 0, 0, 20994),
(41067, 1, 0, 0, 0, 0, 0, 0, 0, 20994),
(41069, 1, 1, 1, 0, 0, 0, 0, 0, 20994),
(41070, 0, 0, 0, 0, 0, 0, 0, 0, 21287),
(41096, 0, 0, 0, 0, 0, 0, 0, 0, 21287),
(41099, 0, 0, 0, 0, 0, 0, 0, 0, 21287),
(41221, 1, 1, 0, 0, 0, 0, 0, 0, 20994),
(42671, 1, 66, 0, 0, 0, 0, 0, 0, 23222),
(42677, 1, 273, 0, 0, 0, 0, 0, 0, 23222),
(42679, 1, 1, 0, 0, 0, 0, 0, 0, 23222),
(42681, 1, 6, 0, 0, 0, 0, 0, 0, 23222),
(42682, 25, 1, 0, 0, 0, 0, 0, 0, 23222),
(42683, 273, 1, 0, 0, 0, 0, 0, 0, 23222),
(42869, 1, 1, 0, 0, 0, 0, 0, 0, 21737),
(42872, 1, 1, 0, 0, 0, 0, 0, 0, 23222);
