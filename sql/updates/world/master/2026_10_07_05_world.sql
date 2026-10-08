-- Dread Commander Arganoth (90705), the commander of the Alliance beach in the Broken Shore scenario (map 1460),
-- was never sniffed and kept the placeholder faction 35 and level 1.
-- Give him the values sniffed for Felguard Legionnaire 109604 of the same beach, as was done for its other
-- demons: faction 2780 and level 98-110, the level scaling range already stored for him.
UPDATE `creature_template` SET `faction`=2780 WHERE `entry`=90705;

UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry`=90705;
