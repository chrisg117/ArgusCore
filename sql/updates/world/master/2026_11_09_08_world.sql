-- Quest texts, Azsuna QuestLine 82, slices 1-4: offer (quest_offer_reward), progress (quest_request_items) and detail emotes
-- (quest_details), copied by quest id. Written only where the world DB had no row.
-- lint: poi 37256 no quest_poi row in the data; outside this texts-only file (listed for its chain)
-- lint: poi 37729 no quest_poi row in the data; outside this texts-only file (listed for its chain)
-- lint: poi 42371 no quest_poi row in the data; outside this texts-only file (listed for its chain)

DELETE FROM `quest_request_items` WHERE `ID` IN (37256, 37257, 37450, 37469, 37653, 37656, 37729, 37730, 37736, 37853, 42271, 42371);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(37256, 0, 0, 0, 0, '<The night guard looks at you expectantly.>', 21384),
(37257, 0, 0, 0, 0, 'Моим людям до меня особого дела нет, но я все равно буду охранять их покой – это мой долг.', 21384),
(37450, 1, 0, 0, 0, 'If there are still blue dragons left, we must make every effort to earn their favour and gain their support.', 21384),
(37469, 0, 0, 0, 0, '<The last fragment of Tidal Stone lies before you.>', 21384),
(37653, 1, 0, 0, 0, '我们需要那些恶魔之魂，$c。', 21491),
(37656, 396, 0, 0, 0, 'If we intend to win the war against the Legion, we must know exactly what the demons are planning.', 22522),
(37729, 0, 0, 0, 0, 'For business, for business! Move your legs!', 22522),
(37730, 0, 0, 0, 0, 'Ну что, удалось тебе встретиться с госпожой директрисой?', 21384),
(37736, 0, 0, 0, 0, 'Everyone is trying to get into the academy, as I take a look.', 21384),
(37853, 0, 0, 0, 0, 'Может, конечно, это звучит глупо... Но я всегда верила, что дедушка будет с нами до конца времен.', 21384),
(42271, 0, 0, 0, 0, 'О, нет...', 21384),
(42371, 0, 0, 0, 0, 'I have to admit that serious scientific research is an extremely tiring affair.', 21384);
