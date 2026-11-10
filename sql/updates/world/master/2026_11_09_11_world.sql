-- These rows held non-English text (Cyrillic; Chinese for the progress text of 37653), and it was
-- double-encoded on the way in, so the client shows mojibake. Replace them with the English text where there is one; the ten progress texts without English are dropped.
DELETE FROM `quest_offer_reward` WHERE `ID` IN (38689, 38690, 38723, 38727, 38728, 38819, 39261, 39515, 39516, 39663,
39682, 39684, 39685, 39686, 39688, 39689, 39690, 39694, 40051, 40373, 40705, 40714, 40716, 40729, 41019, 42670, 42671,
42677, 42679, 42681, 42682, 42683, 42695, 42697);
DELETE FROM `quest_request_items` WHERE `ID` IN (37257, 37653, 37730, 37853, 38723, 38728, 38759, 39049, 39516, 39683,
39694, 39761, 39799, 40222, 40373, 42271);

-- English texts for the replaced rows.
DELETE FROM `quest_offer_reward` WHERE `ID` IN (38689, 38690, 38723, 38727, 38728, 38819, 39261, 39515, 39516, 39663, 39682, 39684, 39685, 39686, 39688, 39689, 39690, 39694, 40051, 40373, 40705, 40714, 40716, 40729, 41019, 42670, 42671, 42677, 42679, 42681, 42682, 42683, 42695, 42697);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(38689, 273, 0, 0, 0, 0, 0, 0, 0, 'The color has returned to your face, $n.', 20994),
(38690, 396, 0, 0, 0, 0, 0, 0, 0, 'We always knew the Illidari would be called upon to defend this world. It is why we sacrificed everything to become who... WHAT... we are.', 20994),
(38723, 274, 0, 0, 0, 0, 0, 0, 0, 'Excellent. You have absorbed the powers of these brutes. Look how strong you have grown!', 20994),
(38727, 1, 0, 0, 0, 0, 0, 0, 0, 'True leadership is hard to come by. Leading from the front is an even rarer trait.$B$BWe are just about ready to assail Brood Queen Tyranna''s command center.', 20994),
(38728, 1, 0, 0, 0, 0, 0, 0, 0, 'Now that we have the keystone in our possession, there is just one last thing left to do...', 20994),
(38819, 273, 6, 0, 0, 0, 0, 0, 0, 'I am in complete agreement. From where I''m standing, you utterly decimated their forces.$B$BWhat do you think? Are we ready to fly up to that command center and take the Sargerite Keystone?', 20994),
(39261, 1, 0, 0, 0, 0, 0, 0, 0, 'We have much to do, and precious little time to do it.', 20994),
(39515, 1, 274, 0, 0, 0, 0, 0, 0, 'I blame myself, $n. Mannethrel was already struggling. It is a never-ending battle we all must endure... to control the fel energies that we have taken in.$B$BMaybe it would be best if you held off teaching the others for now. We can not afford to lose anyone else.', 21287),
(39516, 1, 274, 0, 0, 0, 0, 0, 0, 'I blame myself, $n. Mannethrel was already struggling. It is a never-ending battle we all must endure... to control the fel energies that we have taken in.$B$BMaybe it would be best if you held off teaching the others for now. We can not afford to lose anyone else.', 20994),
(39663, 6, 0, 0, 0, 0, 0, 0, 0, 'Can you sense her power? It will take all of you combined.$B$BEven so... I better go secure our way out of here.', 20994),
(39682, 274, 0, 0, 0, 0, 0, 0, 0, 'Kayn and Altruis said you would be coming. Just in time, $n.', 20994),
(39684, 274, 0, 0, 0, 0, 0, 0, 0, 'I had no doubt you would know what to do.', 20994),
(39685, 274, 0, 0, 0, 0, 0, 0, 0, 'I knew you could do it, $n. We could not let that creature escape.', 20994),
(39686, 274, 0, 0, 0, 0, 0, 0, 0, 'This is serious. Their core ideals are in direct conflict.$B$BI fear they will take this duel to the death.', 20994),
(39688, 274, 0, 0, 0, 0, 0, 0, 0, 'We escaped the Vault, but our work is far from over.$B$BThe Legion''s assault on this world is just beginning.$B$BIf Azeroth is to survive, we may have to work with this... Archmage Khadgar.', 21287),
(39689, 274, 0, 0, 0, 0, 0, 0, 0, 'If you are ready, I will take you to our destination.$B$BAre you prepared to leave?', 21287),
(39690, 274, 0, 0, 0, 0, 0, 0, 0, 'If you are ready, I will take you to our destination.$B$BAre you prepared to leave?', 20994),
(39694, 274, 0, 0, 0, 0, 0, 0, 0, 'We escaped the Vault, but our work is far from over.$B$BThe Legion''s assault on this world is just beginning.$B$BIf Azeroth is to survive, we may have to work with this... Archmage Khadgar.', 20994),
(40051, 273, 0, 0, 0, 0, 0, 0, 0, 'Surprising. I agree. Well chosen, $n.$B$BNow, you must teach the rest of us.', 20994),
(40373, 274, 0, 0, 0, 0, 0, 0, 0, 'I see. I support you no matter your choice. With this new beginning, we shall rebuild the Illidari.', 20994),
(40705, 1, 0, 0, 0, 0, 0, 0, 0, 'Can I take that as a yes?$B$BIf you are ready, I''d like to get started immediately.$B$BI know you are immensely capable, but let''s not leave anything to chance!', 21287),
(40714, 1, 0, 0, 0, 0, 0, 0, 0, 'I''m sure you have many questions, but they''re just going to have to wait. We''re at war now!$B$BWe have to move quickly, but I''ll try to explain what I can.', 21287),
(40716, 0, 0, 0, 0, 0, 0, 0, 0, 'You have made the right decision.$B$BLet us hope you live up to your reputation.', 21287),
(40729, 0, 0, 0, 0, 0, 0, 0, 0, 'Calydus sorry we not save your friends, but maybe they survive if we hurry.$B$BJagganoth torture them, yes, but he likes to keep victims alive... mostly.', 21287),
(41019, 1, 0, 0, 0, 0, 0, 0, 0, 'An excellent choice! We will do our best to support your efforts on the Broken Isles.', 21287),
(42670, 1, 0, 0, 0, 0, 0, 0, 0, 'You have selected your first two champions. There are great things ahead of us, $n.', 23222),
(42671, 1, 0, 0, 0, 0, 0, 0, 0, 'You have selected your first two champions. There are great things ahead of us, $n.', 23222),
(42677, 1, 0, 0, 0, 0, 0, 0, 0, 'Well, at least we''re going to have some fresh faces around here. If a broken''s face can ever be called fresh, that is.$B$B<Kor''vas smirks.>', 23222),
(42679, 273, 0, 0, 0, 0, 0, 0, 0, 'Outstanding. These new Ashtongue will give you the flexibility you need to position troops wherever you see fit.', 23222),
(42681, 1, 0, 0, 0, 0, 0, 0, 0, 'They''ve returned with him. It''s Loramus, alright. But, he''s trapped within the body of the dreadlord, Razelikh the Defiler!$B$BUntil we can make sure he doesn''t pose a danger, I''ve ordered him kept in one of our demon traps, below.', 23222),
(42682, 6, 0, 0, 0, 0, 0, 0, 0, 'Do we really need all of these ingredients to contact the Master''s soul?', 23222),
(42683, 1, 274, 0, 0, 0, 0, 0, 0, 'That sounds like good news. He is already contributing to the cause and we are that much stronger for his help.$B$BStill, he is not fully in control of Razelikh. I think it best that we leave him in that trap until we can find him a new body. Don''t you?$B$BAnd, if he proves to be too much of a problem, we can take ''other'' measures.', 23222),
(42695, 1, 273, 0, 0, 0, 0, 0, 0, 'An honor, $n. Of course I will serve as your champion and second-in-command. Could it have been any other way?$B$BWe have only just begun to move forward with the master''s plan, but already, you have made great strides. If we are to defeat the Burning Legion, we must remain vigilant. Any other goal pales in comparison. The fate of everything hangs in the balance.$B$BAt the end of the day, we will stand victorious... no matter the cost!', 23222),
(42697, 6, 1, 0, 0, 0, 1000, 0, 0, 'You want me to be one of your champions?$B$B<Asha thoughtfully ponders your request a moment.>$B$BI accept, but on one condition. I want you to put me into field duty. While I am of use here on the Fel Hammer, my talents will be put to better use back on Azeroth.$B$BBeyond that, I trust that the plan is still to bring the Burning Legion to its knees?', 23222);
DELETE FROM `quest_request_items` WHERE `ID` IN (37257, 37730, 38728, 38759, 40222, 42271);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(37257, 0, 0, 0, 0, 'My people may not care much for me, but that will not stop me from caring for them.', 21384),
(37730, 0, 0, 0, 0, 'Were you able to meet with the headmistress?', 21384),
(38728, 6, 0, 0, 0, '$n, are you ready?', 20994),
(38759, 6, 0, 0, 0, 'Some of our demon hunters are missing. Did you see them back on the Molten Shore?', 20994),
(40222, 6, 0, 0, 0, 'Do you have it?', 20994),
(42271, 0, 0, 0, 0, 'Oh no...', 21384);
