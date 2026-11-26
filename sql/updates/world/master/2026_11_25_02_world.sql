-- Azsuna: three hand-placed spawns moved onto the WoW-Pro 7.3.5 points they were off from (guidecheck 2026-11-25),
-- each checked against the navmesh: height from the mesh, at least 4 yd from any wall. The old spots were 1.7 to 3.1 yd
-- from walls.

-- Archmage Khadgar (93337) at Illidari Stand, giver of Journey to the Repose (38443): 24.7 yd off. The guide point
-- (-101.5, 6961.5) is 2.3 yd from a wall; the nearest spot with 4 yd is 3 yd from it
UPDATE `creature` SET `position_x` = -100.98, `position_y` = 6964.45, `position_z` = 12.62 WHERE `guid` = 12202001 AND `id` = 93337;

-- Sythorne (107334) and his podium (250362) in the Nar'thalas Academy, Study Hall: Combat Research (42371): both about
-- 16 yd off in the same direction (the floor-17 conversion was checked against Thyrillion: exact to 2 yd)
UPDATE `creature` SET `position_x` = 191.40, `position_y` = 6463.30, `position_z` = -53.44, `orientation` = 0.17 WHERE `guid` = 12204167 AND `id` = 107334;
UPDATE `gameobject` SET `position_x` = 193.70, `position_y` = 6463.70, `position_z` = -53.49 WHERE `guid` = 12201218 AND `id` = 250362;
