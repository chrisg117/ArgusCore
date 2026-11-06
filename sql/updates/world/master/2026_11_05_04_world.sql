-- DK campaign: The Call of Icecrown, scenario 901 step 1810, tuning the Spire trash (Task 3b-2b) after the first pass.
-- Claudk (level 100, ilvl 650 plate, 228.6k health, about 10k damage per second) died twice to the first landing pair: a
-- pair dealt about 9k damage per second against 568k health. Target: a pair takes about 30 s and half her health.
-- Fidelity note: the recording shows about 533k health on a retail Phantom Mage; these values are tuned below retail
-- for a fresh 100 in quest gear. The Frostbolt cooldown is lengthened in the script (DamageModifier only scales melee).

-- Mage and Swordsman: half health; the Swordsman's melee halved
UPDATE `creature_template_difficulty` SET `HealthModifier`=0.5 WHERE `Entry` IN (94913, 95434) AND `DifficultyID`=0;
UPDATE `creature_template_difficulty` SET `DamageModifier`=0.5 WHERE `Entry`=95434 AND `DifficultyID`=0;
-- Spectral Knights
UPDATE `creature_template_difficulty` SET `HealthModifier`=0.7 WHERE `Entry`=94907 AND `DifficultyID`=0;
UPDATE `creature_template_difficulty` SET `HealthModifier`=1.0 WHERE `Entry`=97659 AND `DifficultyID`=0;

-- Gavinrad's pack: a Knight and one Swordsman (four at once killed her in 7 s); a fidelity note, the recording's pack is unread
DELETE FROM `creature_formations` WHERE `memberGUID`=14800018;
DELETE FROM `creature` WHERE `guid`=14800018;
