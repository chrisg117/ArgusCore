-- Il'gynoth creatures are level 110 (they were level 1, so health was the level 1 base)
UPDATE `creature_template_difficulty` SET `MinLevel` = 110, `MaxLevel` = 110 WHERE `Entry` IN (105393, 105906, 105304, 105322, 105383, 105591, 105721) AND `DifficultyID` = 0;
