-- Those Who Remember to Our Very Bones, up to Prince Farondis. Idri and Farondis were sniffed; nothing here that the player fights was.
-- The naga stand inside They Came From the Sea's POI area and Zarrin on its single point in the low hills. Heights come from terrain, positions rough.
-- The bones have no source in the data. The Salteye murlocs sit in the id block beside the Hatecoil naga and match the text's fish-men, so they carry them
-- on the shore inside Our Very Bones' POI, which covers mostly seabed.
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(37690, 42271),
(37256, 37690),
(37733, 37256),
(37257, 37733)
ON DUPLICATE KEY UPDATE `PrevQuestID`=VALUES(`PrevQuestID`);

DELETE FROM `creature_queststarter` WHERE `id`=88115 AND `quest`=37257;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(88115, 37257, 0);

UPDATE `creature_template` SET `faction`=16 WHERE `entry` IN (88089, 88090);
UPDATE `creature_template_difficulty` SET `LootID`=`Entry` WHERE `Entry` IN (88090, 88099, 88101) AND `DifficultyID`=0;
DELETE FROM `creature_loot_template` WHERE (`Entry`=88090 AND `Item`=120079) OR (`Entry` IN (88099, 88101) AND `Item`=120080);
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(88090, 0, 120079, 100, 1, 1, 0, 1, 1, 'Fathom-Commander Zarrin - Head of Fathom-Commander Zarrin'),
(88099, 0, 120080, 100, 1, 1, 0, 4, 6, 'Salteye Hookblade - Pilfered Night Elf Bone'),
(88101, 0, 120080, 100, 1, 1, 0, 4, 6, 'Salteye Tide-Shaman - Pilfered Night Elf Bone');

-- Farondis' other options belong to Trailing the Tidestone and do nothing until it is built
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=17377 AND `SourceEntry` IN (0, 1, 4);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(15, 17377, 0, 0, 0, 8, 0, 37497, 0, 0, 0, 'Prince Farondis - hidden until Trailing the Tidestone'),
(15, 17377, 1, 0, 0, 8, 0, 37497, 0, 0, 0, 'Prince Farondis - hidden until Trailing the Tidestone'),
(15, 17377, 4, 0, 0, 8, 0, 37497, 0, 0, 0, 'Prince Farondis - hidden until Trailing the Tidestone');

DELETE FROM `creature` WHERE `guid` BETWEEN 12204001 AND 12204040;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204001, 88090, 1220, 7334, 7357, '0', 0, 0, 0, 0, 185.00, 6866.00, 71.40, 4.71, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Zarrin, on the POI
(12204002, 88084, 1220, 7334, 7357, '0', 0, 0, 0, 0, 235.00, 6735.00, 52.60, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204003, 88084, 1220, 7334, 7357, '0', 0, 0, 0, 0, 215.00, 6700.00, 52.90, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204004, 88084, 1220, 7334, 7357, '0', 0, 0, 0, 0, 190.00, 6740.00, 45.50, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204005, 88089, 1220, 7334, 7357, '0', 0, 0, 0, 0, 160.00, 6700.00, 48.90, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204006, 88084, 1220, 7334, 7357, '0', 0, 0, 0, 0, 140.00, 6760.00, 45.70, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204007, 88084, 1220, 7334, 7357, '0', 0, 0, 0, 0, 110.00, 6720.00, 40.20, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204008, 88089, 1220, 7334, 7357, '0', 0, 0, 0, 0, 85.00, 6780.00, 46.20, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204009, 88084, 1220, 7334, 7357, '0', 0, 0, 0, 0, 60.00, 6730.00, 50.30, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204010, 88084, 1220, 7334, 7357, '0', 0, 0, 0, 0, 205.00, 6780.00, 52.20, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204011, 88089, 1220, 7334, 7357, '0', 0, 0, 0, 0, 120.00, 6810.00, 50.30, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204012, 88084, 1220, 7334, 7357, '0', 0, 0, 0, 0, 160.00, 6800.00, 53.90, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204013, 88089, 1220, 7334, 7357, '0', 0, 0, 0, 0, 175.00, 6850.00, 72.00, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204014, 88084, 1220, 7334, 7357, '0', 0, 0, 0, 0, 200.00, 6855.00, 72.40, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204021, 88099, 1220, 7334, 7357, '0', 0, 0, 0, 0, 300.00, 6505.00, 3.70, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204022, 88101, 1220, 7334, 7357, '0', 0, 0, 0, 0, 280.00, 6525.00, 4.20, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204023, 88099, 1220, 7334, 7357, '0', 0, 0, 0, 0, 255.00, 6505.00, 0.50, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204024, 88101, 1220, 7334, 7357, '0', 0, 0, 0, 0, 225.00, 6530.00, 0.50, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204025, 88099, 1220, 7334, 7357, '0', 0, 0, 0, 0, 195.00, 6540.00, 0.50, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204026, 88101, 1220, 7334, 7357, '0', 0, 0, 0, 0, 165.00, 6545.00, 0.30, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204027, 88099, 1220, 7334, 7357, '0', 0, 0, 0, 0, 130.00, 6555.00, 1.50, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204028, 88101, 1220, 7334, 7357, '0', 0, 0, 0, 0, 100.00, 6550.00, 0.80, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204029, 88099, 1220, 7334, 7357, '0', 0, 0, 0, 0, 75.00, 6545.00, 2.20, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204030, 88101, 1220, 7334, 7357, '0', 0, 0, 0, 0, 45.00, 6545.00, 1.40, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204031, 88099, 1220, 7334, 7357, '0', 0, 0, 0, 0, 240.00, 6555.00, 4.60, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204032, 88101, 1220, 7334, 7357, '0', 0, 0, 0, 0, 205.00, 6560.00, 0.50, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204033, 88099, 1220, 7334, 7357, '0', 0, 0, 0, 0, 150.00, 6560.00, 0.60, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204034, 88101, 1220, 7334, 7357, '0', 0, 0, 0, 0, 270.00, 6550.00, 6.30, 0, 60, 5, 0, 1, NULL, NULL, NULL, NULL, 0);
