-- More of the warlock opening at level 1 with no scaling, from the slices own spawns: Ritssyn in Dalaran, Jergosh
-- the Invoker and Calydus at Caer Darrow, the Dark-Sworn Warlock and Occul'tharon in Tol Barad, the Fel Bat and the
-- Scepter on the Broken Shore. Same shape as 2026_11_10_00.
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110
WHERE `DifficultyID`=0 AND `Entry` IN (101456, 109608, 109698, 109952, 109981, 110480, 110561);
