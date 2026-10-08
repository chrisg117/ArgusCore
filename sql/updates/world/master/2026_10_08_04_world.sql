-- Krosus (90544), who rises from the fel pool in the Broken Shore scenario (map 1460), was never sniffed and kept
-- the placeholder faction 35 and level 1.
-- Give him the values already stored for the other demons of the scenario: faction 2780 and level 98-110, which
-- is also the level scaling range stored for him.
UPDATE `creature_template` SET `faction`=2780 WHERE `entry`=90544;

UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry`=90544;
