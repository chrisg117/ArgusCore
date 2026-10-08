-- Gul'dan (94276) and Mo'arg Spinebreaker (105205), who end the Broken Shore scenario (map 1460) before the Tomb
-- of Sargeras, were never sniffed and kept the placeholder faction 35 and level 1.
-- Give them the values already stored for the demons of the scenario: faction 2780 and level 98-110, which is
-- also the level scaling range stored for both.
UPDATE `creature_template` SET `faction`=2780 WHERE `entry` IN (94276, 105205);

UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry` IN (94276, 105205);
