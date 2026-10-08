-- The demons of the Black City in the Broken Shore scenario (map 1460) were never sniffed and kept the
-- placeholder faction 35 and level 1.
-- Give them the values already stored for Mother Virila (100621) of the same city: faction 2780 and
-- level 98-110, which is also the level scaling range stored for all of them but the three imps.
UPDATE `creature_template` SET `faction`=2780 WHERE `entry` IN (91967, 91970, 94189, 94190, 94191, 97510, 102701, 102702, 102703, 102704, 102705, 102706, 103896, 103897, 103899, 110614, 110616, 110617);

UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry` IN (91967, 91970, 94189, 94190, 94191, 97510, 102701, 102702, 102703, 102704, 102705, 102706, 103896, 103897, 103899, 110614, 110616, 110617);

-- Felfire Imp, Fiery Trickster and Shadowflame Imp have no level scaling range stored at all, so they would not
-- scale to the player like the rest of the city. Give them the same range.
UPDATE `creature_template_difficulty` SET `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `DifficultyID`=0 AND `Entry` IN (103896, 103897, 103899);
