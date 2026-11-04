-- Azsuna slice 4b, part 1: Dressing With Class (37736), Hit the Books (37678), The Haunted Halls (37518).
-- Positions from the quest POIs and Wowhead's pins (see 2026_11_03_04); floor heights from the navmesh: entrance hall
-- about -30, library about -15, lower halls about -53.

-- Givers, each quest only after the one before it
DELETE FROM `creature_queststarter` WHERE (`id`, `quest`) IN ((89660, 37736), (89660, 37518), (89849, 37678));
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(89660, 37736, 0),
(89660, 37518, 0),
(89849, 37678, 0);

INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(37736, 37468),
(37678, 37468),
(37518, 37736),
(42370, 37518)
ON DUPLICATE KEY UPDATE `PrevQuestID` = VALUES(`PrevQuestID`);

-- The uniform. Hat, wand and the Academy Bookshelf (the spellbook's source on Wowhead) are sniffed objects with no
-- loot: their own entry as loot id, and the quest as the chest's quest (Data8) so they sparkle only for it.
UPDATE `gameobject_template` SET `Data1` = `entry`, `Data8` = 37736 WHERE `entry` IN (239744, 239745, 238940);

-- Nothing in the data or on Wowhead gives the robes; the Drowned Student lies at their POI. A pile of robes of ours.
DELETE FROM `gameobject_template` WHERE `entry` = 1220001;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data8`, `VerifiedBuild`) VALUES
(1220001, 3, 6400, 'Nar''thalas Academy Robes', '', '', '', 1, 93, 1220001, 37736, 0);

DELETE FROM `gameobject_loot_template` WHERE `Entry` IN (239744, 239745, 238940, 1220001);
INSERT INTO `gameobject_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(239744,  0, 120947, 100, 1, 1, 0, 1, 1, 'Nar''thalas Academy Hat - Nar''thalas Academy Hat (Dressing With Class)'),
(239745,  0, 120949, 100, 1, 1, 0, 1, 1, 'Nar''thalas Academy Wand - Nar''thalas Academy Wand (Dressing With Class)'),
(238940,  0, 120946, 100, 1, 1, 0, 1, 1, 'Academy Bookshelf - Nar''thalas Academy Spellbook (Dressing With Class)'),
(1220001, 0, 120948, 100, 1, 1, 0, 1, 1, 'Nar''thalas Academy Robes - Nar''thalas Academy Robes (Dressing With Class)');

DELETE FROM `gameobject` WHERE `guid` BETWEEN 12201211 AND 12201214;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12201211, 239744,  1220, 7334, 0, '0', 0, 0, 0, -1, 153.0, 6326.0, -29.8, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0), -- hat, 37736 POI
(12201212, 239745,  1220, 7334, 0, '0', 0, 0, 0, -1,  96.0, 6273.0, -14.8, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0), -- wand, 37736 POI
(12201213, 238940,  1220, 7334, 0, '0', 0, 0, 0, -1, 161.0, 6340.0, -30.2, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0), -- bookshelf, Wowhead pin
(12201214, 1220001, 1220, 7334, 0, '0', 0, 0, 0, -1,  50.0, 6306.0, -15.0, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0); -- robes, beside the Drowned Student

-- Andellis, the librarian, beside Thyrillion (37678 POI 183,6344); Instructor Nidriel in the lower hall (37518 POI
-- 201,6470); the Ancient Tomes in the library (37678's POI box 47..109, 6274..6336), hovering over the floor
DELETE FROM `creature` WHERE `guid` BETWEEN 12204157 AND 12204166;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204157, 89849, 1220, 7334, 0, '0', 0, 0, 0, 0, 180.0, 6348.0, -30.2, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204158, 89661, 1220, 7334, 0, '0', 0, 0, 0, 0, 201.0, 6470.0, -52.9, 4.71, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204159, 89834, 1220, 7334, 0, '0', 0, 0, 0, 0,  49.0, 6327.0, -12.6, 0, 60, 4, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204160, 89834, 1220, 7334, 0, '0', 0, 0, 0, 0, 101.0, 6314.0, -12.5, 0, 60, 4, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204161, 89834, 1220, 7334, 0, '0', 0, 0, 0, 0, 101.0, 6300.0, -13.0, 0, 60, 4, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204162, 89834, 1220, 7334, 0, '0', 0, 0, 0, 0,  49.0, 6287.0, -12.9, 0, 60, 4, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204163, 89834, 1220, 7334, 0, '0', 0, 0, 0, 0,  85.0, 6330.0, -13.0, 0, 60, 4, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204164, 89834, 1220, 7334, 0, '0', 0, 0, 0, 0,  60.0, 6280.0, -12.3, 0, 60, 4, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204165, 89834, 1220, 7334, 0, '0', 0, 0, 0, 0, 105.0, 6285.0, -13.1, 0, 60, 4, 0, 1, NULL, NULL, NULL, NULL, 0),
(12204166, 89834, 1220, 7334, 0, '0', 0, 0, 0, 0,  75.0, 6320.0, -13.0, 0, 60, 4, 0, 1, NULL, NULL, NULL, NULL, 0);

-- Who sees them: nothing of the academy before Into the Academy is taken
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND ((`SourceGroup` = 3 AND `SourceEntry` IN (89849, 89661, 89834))
    OR (`SourceGroup` = 5 AND `SourceEntry` IN (239744, 239745, 238940, 1220001)));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 89849,   0, 0, 47, 0, 37468, 74, 0, '', 0, 0, 0, '', 'Andellis is visible once quest 37468 (Into the Academy) is taken'),
(32, 3, 89661,   0, 0, 47, 0, 37518, 74, 0, '', 0, 0, 0, '', 'Instructor Nidriel is visible once quest 37518 (The Haunted Halls) is taken'),
(32, 3, 89834,   0, 0, 47, 0, 37678,  8, 0, '', 0, 0, 0, '', 'Ancient Tome is visible while quest 37678 (Hit the Books) is in progress'),
(32, 5, 239744,  0, 0, 47, 0, 37736,  8, 0, '', 0, 0, 0, '', 'Nar''thalas Academy Hat is visible while quest 37736 (Dressing With Class) is in progress'),
(32, 5, 239745,  0, 0, 47, 0, 37736,  8, 0, '', 0, 0, 0, '', 'Nar''thalas Academy Wand is visible while quest 37736 (Dressing With Class) is in progress'),
(32, 5, 238940,  0, 0, 47, 0, 37736,  8, 0, '', 0, 0, 0, '', 'Academy Bookshelf is visible while quest 37736 (Dressing With Class) is in progress'),
(32, 5, 1220001, 0, 0, 47, 0, 37736,  8, 0, '', 0, 0, 0, '', 'Nar''thalas Academy Robes are visible while quest 37736 (Dressing With Class) is in progress');
