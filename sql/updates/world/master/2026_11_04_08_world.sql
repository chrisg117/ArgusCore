-- DK campaign: The Call of Icecrown, the Lich King's talking head.

-- Retail shows "The Lich King" with the helmed Bolvar portrait. 99456 carries the name; its own model 64011 is a bare
-- placeholder human, so the actor row names the helmed display 64642 (95942's). Sniffed talking-head rows all carry a
-- ConversationActorId (the 7.3.5 client has no data to match it against), so ours gets one from the map 1480 block.
-- Known gap: the portrait still draws an empty frame on this fork, though the sniffed 705 (Illidan) renders; see the
-- DK frost ledger for what was ruled out
UPDATE `conversation_actors` SET `ConversationActorId`=14800001, `CreatureId`=99456, `CreatureDisplayInfoId`=64642 WHERE `ConversationId`=14800001 AND `Idx`=0;
