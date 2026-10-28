-- Archmage Khadgar outside the Vault of the Wardens (Between Us and Freedom, Illidari, We Are Leaving) stood at the
-- foot of the cliff, inside the rock there (the navmesh has a second surface 7 yards above him and no ground beside
-- him): the client got him and showed his quest marker, but he could not be seen. He now stands on open ground
-- between Kayn / Altruis and the next quest's turn-in point, facing the follower.
UPDATE `creature` SET `position_x` = 4178.00, `position_y` = -854.00, `position_z` = 292.30, `orientation` = 3.2600 WHERE `guid` = 14680288;
