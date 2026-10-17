-- The demons on the Alliance beach of the Broken Shore scenario (map 1460) were never sniffed. They kept
-- the placeholder faction 35 and level 1, which made them friendly level 1 creatures.
-- Felguard Legionnaire 109604, from the same beach, does have sniffed values: faction 2780 and level 98-110.
-- Faction 2780 is hostile to players and belongs to the faction that the scenario's Alliance leaders
-- (faction 2879) list as their enemy, and 98-110 is the LevelScalingMin/Max already stored for all of them.
UPDATE `creature_template` SET `faction`=2780 WHERE `entry` IN
(90686,  -- Felstalker Dreadhound
 91588,  -- Fel Lord Kurduz
 91704,  -- Anchoring Crystal
 109586, -- Fel Lord Rakkan
 109587, -- Fel Lord Zardak
 109591, -- Felguard Legionnaire
 109592); -- Felguard Legionnaire

UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry` IN (90686, 91588, 91704, 109586, 109587, 109591, 109592);
