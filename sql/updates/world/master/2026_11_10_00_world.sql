-- The creatures of the warlock opening (Dalaran, Broken Shore, Tol Barad, the Dreadscar prison and the hall) and its
-- kill-credit bunnies had level 1 and no level scaling stored, so they showed and fought as level 1. Give them
-- the usual Legion shape: level 110, scaling 98-110.
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110
WHERE `DifficultyID`=0 AND `Entry` IN (98771, 98797, 98831, 98834, 98836, 98873, 99145, 99146, 99147, 101097, 101454,
101455, 101457, 101460, 101892, 101921, 102589, 103505, 103506, 109838, 109955, 109956, 109964, 110556,
101969, 109659, 109693, 109696, 109836, 109837, 110523, 110581, 114337);
