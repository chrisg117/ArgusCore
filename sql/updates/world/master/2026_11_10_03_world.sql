-- Calydus in the Dreadscar prison scenario (101050, named by the instance script) was missed by 2026_11_10_00:
-- level 1 with no scaling. Same shape as the rest of the warlock opening.
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110
WHERE `DifficultyID`=0 AND `Entry`=101050;
