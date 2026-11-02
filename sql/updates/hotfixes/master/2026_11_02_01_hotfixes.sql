-- The campaign missions leave the offers once their objective is done: a won mission's chest removes it, and while the quest
-- waits for its turn-in the table would offer it again. 65537: the first result inverted (not complete), and with the rest.
UPDATE `player_condition` SET `CurrentCompletedQuestLogic`=65537, `CurrentCompletedQuestID1`=`CurrQuestID1` WHERE `ID` IN (43443, 40713, 41521);
