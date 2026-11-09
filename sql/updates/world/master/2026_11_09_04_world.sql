-- Quest texts, demon hunter start: Mardum and the Vault of the Wardens (QuestLines 90 and 150): offer (quest_offer_reward), progress (quest_request_items) and detail emotes
-- (quest_details), copied by quest id. Written only where the world DB had no row.
-- lint: texts pending 38689 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 38690 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 38727 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 38729 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 38766 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 39515 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 39663 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 39682 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 39684 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 39685 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 39686 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: giver 39687 no starter in the data or the fork; outside this texts-only file (listed for its chain)
-- lint: poi 39687 no quest_poi row in the data; outside this texts-only file (listed for its chain)
-- lint: texts pending 39688 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40051 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40253 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40254 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40255 no progress text in the source world data (the quest likely has none on retail); the user pass decides
-- lint: texts pending 40256 no progress text in the source world data (the quest likely has none on retail); the user pass decides

DELETE FROM `quest_offer_reward` WHERE `ID` IN (38672, 38689, 38690, 38723, 38727, 38728, 38729, 38819, 39515, 39516, 39663, 39682, 39683, 39684, 39685, 39686, 39687, 39688, 39689, 39690, 39694, 40051, 40222, 40253, 40254, 40255, 40256, 40373);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(38672, 0, 0, 0, 0, 0, 0, 0, 0, '< Pull the lever. >', 20994),
(38689, 273, 0, 0, 0, 0, 0, 0, 0, 'Твое лицо вновь заиграло красками, $n.', 20994),
(38690, 396, 0, 0, 0, 0, 0, 0, 0, 'Мы всегда знали, что помощь иллидари потребуется, чтобы защитить этот мир. Именно поэтому мы стали теми, кем.... ЧЕМ мы стали.  ', 20994),
(38723, 274, 0, 0, 0, 0, 0, 0, 0, 'Превосходно. Впитай в себя эту мощь, и ты почувствуешь, как твоя сила растет!', 20994),
(38727, 1, 0, 0, 0, 0, 0, 0, 0, 'Не каждому дано быть лидером. А еще сложнее командовать на передовой.\n\nМы практически готовы напасть на флагман королевы стаи Тиранны.', 20994),
(38728, 1, 0, 0, 0, 0, 0, 0, 0, 'Теперь, когда ключ у нас, осталось сделать только одну вещь...', 20994),
(38729, 0, 0, 0, 0, 0, 0, 0, 0, '<Since you''re the only one using a sargerite key, the Square portal is filled with supernatural energy.\n\nIt''s time for a triumphant return to the Black Temple, where you will help the Illidan lord throw away the invaders from Azeroth and Shattrat.', 20994),
(38819, 273, 6, 0, 0, 0, 0, 0, 0, 'Приятное зрелище. Мне кажется, тебе удалось полностью дезорганизовать войска Легиона.\n\nКак считаешь, мы уже готовы напасть на флагманский корабль, чтобы завладеть саргеритовым ключом?', 20994),
(39515, 1, 274, 0, 0, 0, 0, 0, 0, 'Это моя вина, $n. Маннетрель и до этого был на грани. В этой бесконечной битве нам всем нужно научиться управлять потоками Скверны, которая нас пропитала.\n\nМожет быть, тебе пока не стоит раскрывать это новое знание остальным. Мы не можем потерять еще одного бойца.', 21287),
(39516, 1, 274, 0, 0, 0, 0, 0, 0, 'Это моя вина, $n. Маннетрель и до этого был на грани. В этой бесконечной битве нам всем нужно научиться управлять потоками Скверны, которая нас пропитала.\n\nМожет быть, тебе пока не стоит раскрывать это новое знание остальным. Мы не можем потерять еще одного бойца.', 20994),
(39663, 6, 0, 0, 0, 0, 0, 0, 0, 'Чувствуешь ее мощь? Чтобы одолеть королеву, нам придется атаковать ее всеми силами.\n\nИ все же... Думаю, я лучше расчищу нам путь к отступлению.', 20994),
(39682, 274, 0, 0, 0, 0, 0, 0, 0, 'Кайн и Алтруис сказали, что ты скоро прибудешь. Как раз вовремя, $n.', 20994),
(39683, 274, 0, 0, 0, 0, 0, 0, 0, 'Yes ... you just look ... how much strength and knowledge is in it. $B$BTake it, $n.', 20994),
(39684, 274, 0, 0, 0, 0, 0, 0, 0, 'Я не сомневалась – ты поймешь, что нужно сделать.', 20994),
(39685, 274, 0, 0, 0, 0, 0, 0, 0, 'Ты $gсправился:справилась;, кто бы сомневался. Нельзя было допустить, чтобы это существо вырвалось на волю.', 20994),
(39686, 274, 0, 0, 0, 0, 0, 0, 0, 'Это уже не шутки. Они упорствут в своих убеждениях, и их противоречия неразрешимы.$B$BБоюсь, все это окончится как минимум одной смертью.', 20994),
(39687, 274, 0, 0, 0, 0, 0, 0, 0, 'Lord Illidan has shown us the way, but everyone must go his own way.', 20994),
(39688, 274, 0, 0, 0, 0, 0, 0, 0, 'Нам удалось выбраться из Казематов, но наше дело еще не окончено.$B$BНашествие Легиона на наш мир только начинается.$B$BЕсли мы хотим спасти Азерот, придется сотрудничать с этим... верховным магом Кадгаром.', 21287),
(39689, 274, 0, 0, 0, 0, 0, 0, 0, 'Ты $gготов:готова; отправиться в Даларан?$B$BДай мне знать, и я перенесу тебя туда.', 21287),
(39690, 274, 0, 0, 0, 0, 0, 0, 0, 'Ты $gготов:готова; отправиться в Даларан?$B$BДай мне знать, и я перенесу тебя туда.', 20994),
(39694, 274, 0, 0, 0, 0, 0, 0, 0, 'Нам удалось выбраться из Казематов, но наше дело еще не окончено.$B$BНашествие Легиона на наш мир только начинается.$B$BЕсли мы хотим спасти Азерот, придется сотрудничать с этим... верховным магом Кадгаром.', 20994),
(40051, 273, 0, 0, 0, 0, 0, 0, 0, 'Это и правда удивительно. Мне нравится твой выбор, $n.\n\nА теперь передай эти знания остальным.', 20994),
(40222, 6, 0, 0, 0, 0, 0, 0, 0, 'Can you feel the power emanating from the book? I cannot wait to learn its secrets.', 20994),
(40253, 0, 0, 0, 0, 0, 0, 0, 0, 'Excellent. You have absorbed the powers of these brutes. Look how strong you have grown!', 0),
(40254, 0, 0, 0, 0, 0, 0, 0, 0, 'Yes... just look you... surging with new power and knowledge!', 0),
(40255, 0, 0, 0, 0, 0, 0, 0, 0, 'We escaped the Vault, but our work is far from over.$B$BThe Legion''s assault on this world is just beginning.$B$BIf Azeroth is to survive, we may have to work with this... Archmage Khadgar.', 0),
(40256, 0, 0, 0, 0, 0, 0, 0, 0, 'We escaped the Vault, but our work is far from over.$B$BThe Legion''s assault on this world is just beginning.$B$BIf Azeroth is to survive, we may have to work with this... Archmage Khadgar.', 0),
(40373, 274, 0, 0, 0, 0, 0, 0, 0, 'Значит, так? Впрочем, я поддерживаю любой твой выбор. Теперь мы возродим иллидари.', 20994);
DELETE FROM `quest_request_items` WHERE `ID` IN (38672, 38723, 38728, 38759, 38819, 39049, 39516, 39683, 39687, 39694, 40222, 40373);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(38672, 0, 0, 0, 0, '<This lever appears to keep the door closed.>', 20994),
(38723, 0, 0, 0, 0, 'Уничтожь великанов и забери Печати.', 20994),
(38728, 6, 0, 0, 0, 'Ты $gготов:готова;, $n?', 20994),
(38759, 6, 0, 0, 0, 'Некоторые из наших охотников на демонов не вернулись. Ты $gвидел:видела; их на Раскаленном берегу?', 20994),
(38819, 1, 0, 0, 0, 'How do you rate the current situation, $n?', 21287),
(39049, 0, 0, 0, 0, 'Прости, что не могу отдать тебе честь, $n, так как занят ритуалом.', 20994),
(39516, 1, 0, 0, 0, 'Мы ожидаем твоих приказаний, $n.', 20994),
(39683, 0, 0, 0, 0, 'Сердце Хаоса у тебя?', 20994),
(39687, 0, 0, 0, 0, 'The source of justice may recall our past.', 20994),
(39694, 0, 0, 0, 0, 'Свобода дается дорогой ценой.', 20994),
(40222, 6, 0, 0, 0, 'Ну, что? $GПринес:Принесла;?', 20994),
(40373, 0, 0, 0, 0, 'Ты $gсделал:сделала; выбор?', 23420);
DELETE FROM `quest_details` WHERE `ID` IN (38672, 38689, 38690, 38723, 38728, 38729, 38766, 39515, 39516, 39663, 39682, 39683, 39684, 39685, 39686, 39687, 39688, 39689, 39690, 39694, 40051, 40222, 40373);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(38672, 0, 0, 0, 0, 0, 0, 0, 0, 20994),
(38689, 274, 0, 0, 0, 0, 0, 0, 0, 20994),
(38690, 1, 0, 0, 0, 0, 0, 0, 0, 20994),
(38723, 6, 0, 0, 0, 0, 0, 0, 0, 20994),
(38728, 396, 0, 0, 0, 0, 0, 0, 0, 20994),
(38729, 5, 0, 0, 0, 0, 0, 0, 0, 20994),
(38766, 1, 0, 0, 0, 0, 0, 0, 0, 20994),
(39515, 273, 0, 0, 0, 0, 0, 0, 0, 21287),
(39516, 273, 0, 0, 0, 0, 0, 0, 0, 20994),
(39663, 1, 0, 0, 0, 0, 0, 0, 0, 20994),
(39682, 6, 0, 0, 0, 0, 0, 0, 0, 20994),
(39683, 6, 0, 0, 0, 0, 0, 0, 0, 20994),
(39684, 6, 0, 0, 0, 0, 0, 0, 0, 20994),
(39685, 6, 0, 0, 0, 0, 0, 0, 0, 20994),
(39686, 6, 0, 0, 0, 0, 0, 0, 0, 20994),
(39687, 6, 0, 0, 0, 0, 0, 0, 0, 20994),
(39688, 6, 0, 0, 0, 0, 0, 0, 0, 21287),
(39689, 6, 0, 0, 0, 0, 0, 0, 0, 21287),
(39690, 6, 0, 0, 0, 0, 0, 0, 0, 20994),
(39694, 6, 0, 0, 0, 0, 0, 0, 0, 20994),
(40051, 1, 0, 0, 0, 0, 0, 0, 0, 20994),
(40222, 0, 0, 0, 0, 0, 0, 0, 0, 20994),
(40373, 6, 0, 0, 0, 0, 0, 0, 0, 20994);
