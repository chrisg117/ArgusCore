-- DK campaign: The Call of Icecrown, fidelity after the user's third run.

-- The Lich King's talking head showed an empty frame, with display 64011 and with 64642 alike. The client draws the
-- head from the creature, and 94369 lists the invisible trigger model 11686 next to 64011. 99456, also "The Lich King",
-- has 64011 alone, the model of Legion's Lich King entries
UPDATE `conversation_actors` SET `CreatureId`=99456, `CreatureDisplayInfoId`=64011 WHERE `ConversationId`=14800001 AND `Idx`=0;

-- Mograine's start spot, from the user's placement on the landing (he faces the arriving player)
UPDATE `creature` SET `position_x`=-723.53, `position_y`=2210.59, `position_z`=535.73, `orientation`=3.51 WHERE `guid`=14800000;
