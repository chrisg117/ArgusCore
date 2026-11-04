-- Azsuna slice 4b, part 2: Wanding 101 (42370), Study Hall: Combat Research (42371), Pop Quiz: Advanced Rune Drawing
-- (37729), The Headmistress' Keys (37730), The Tidestone: Shattered (37469). Scripts in zone_azsuna.cpp.

-- Givers, each quest only after the one before it (Study Hall, a side quest, once the classes have started)
DELETE FROM `creature_queststarter` WHERE (`id`, `quest`) IN ((107334, 42371), (89661, 37729), (89661, 37730), (89660, 37469));
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(107334, 42371, 0),
(89661, 37729, 0),
(89661, 37730, 0),
(89660, 37469, 0);

INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(42371, 37518),
(37729, 42370),
(37730, 37729),
(37469, 37730)
ON DUPLICATE KEY UPDATE `PrevQuestID` = VALUES(`PrevQuestID`);

-- Wanding 101: the wand is the override bar's Wand Practice (OverrideSpellData 711 -> 212782). No spell in the client
-- applies it, so a server-side aura of ours does, in the academy while the quest is in progress.
DELETE FROM `serverside_spell` WHERE `Id` = 1220001;
INSERT INTO `serverside_spell` (`Id`, `DifficultyID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `SchoolMask`, `SpellName`) VALUES
(1220001, 0, 0x00000180, 0, 1, 21, 1, 1, 'Wanding 101: Nar''thalas Academy Wand (override bar 711)');
DELETE FROM `serverside_spell_effect` WHERE `SpellID` = 1220001;
INSERT INTO `serverside_spell_effect` (`SpellID`, `EffectIndex`, `DifficultyID`, `Effect`, `EffectAura`, `EffectMiscValue1`, `ImplicitTarget1`) VALUES
(1220001, 0, 0, 6, 293, 711, 1);
-- quest_end_status is a mask the end quest must match (SpellArea::IsFitToRequirements), not a forbidden one: 8 keeps
-- the aura while the quest is in progress, and autoremove drops it once it is complete
DELETE FROM `spell_area` WHERE `spell` = 1220001;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `flags`, `quest_start_status`, `quest_end_status`) VALUES
(1220001, 7358, 42370, 42370, 0, 0, 2, 3, 8, 8);
DELETE FROM `spell_script_names` WHERE `spell_id` = 212782;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(212782, 'spell_narthalas_wand_practice');

-- Pop Quiz: three rune outlines on the floor of the lower hall. Server-side area triggers of ours start the tracing
-- scenes 935 to 937; the scenes' "Credit" event counts the rune.
DELETE FROM `areatrigger_template` WHERE `Id` IN (1220006, 1220007, 1220008) AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1220006, 1, 1, 0, 0, 0),
(1220007, 1, 1, 0, 0, 0),
(1220008, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` IN (1220006, 1220007, 1220008) AND `IsCustom` = 1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ScriptName`, `VerifiedBuild`) VALUES
(1220006, 1, 1220006, 1, 0, 0, 2, 'at_narthalas_rune_outline', 0),
(1220007, 1, 1220007, 1, 0, 0, 2, 'at_narthalas_rune_outline', 0),
(1220008, 1, 1220008, 1, 0, 0, 2, 'at_narthalas_rune_outline', 0);
DELETE FROM `areatrigger` WHERE `SpawnId` IN (1220006, 1220007, 1220008);
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1220006, 1220006, 1, 1220, '0', 190.0, 6445.0, -53.3, 0, 0, 0, 0, '', 'Nar''thalas Academy - arcane rune outline (Quest: "Pop Quiz: Advanced Rune Drawing")', 0),
(1220007, 1220007, 1, 1220, '0', 205.0, 6440.0, -53.3, 0, 0, 0, 0, '', 'Nar''thalas Academy - fire rune outline (Quest: "Pop Quiz: Advanced Rune Drawing")', 0),
(1220008, 1220008, 1, 1220, '0', 198.0, 6425.0, -52.7, 0, 0, 0, 0, '', 'Nar''thalas Academy - frost rune outline (Quest: "Pop Quiz: Advanced Rune Drawing")', 0);
UPDATE `scene_template` SET `ScriptName` = 'scene_narthalas_rune_drawing' WHERE `SceneId` IN (935, 936, 937);

-- Study Hall: the three books from the shelf, Sythorne's Podium (its spell 212924 is not in the client's data: a
-- script of ours), the scenes into the books, and the drawings inside them
UPDATE `gameobject_template` SET `Data1` = `entry`, `Data8` = 42371 WHERE `entry` IN (250372, 250373, 250374);
UPDATE `gameobject_template` SET `ScriptName` = 'go_sythorne_podium' WHERE `entry` = 250362;
UPDATE `scene_template` SET `ScriptName` = 'scene_study_hall_into_the_book' WHERE `SceneId` IN (1279, 1280, 1281);

-- The Tidestone: Shattered: the five shards, each its own object, one shard each
UPDATE `gameobject_template` SET `Data1` = `entry`, `Data8` = 37469 WHERE `entry` IN (239341, 245483, 245484, 245485, 245486);

DELETE FROM `gameobject_loot_template` WHERE `Entry` IN (250372, 250373, 250374, 239341, 245483, 245484, 245485, 245486);
INSERT INTO `gameobject_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(250373, 0, 137423, 100, 1, 1, 0, 1, 1, 'Encyclopedia Azsunica (K-M) (Study Hall: Combat Research)'),
(250372, 0, 137422, 100, 1, 1, 0, 1, 1, 'Courtship Rituals of the Skrog (Study Hall: Combat Research)'),
(250374, 0, 137426, 100, 1, 1, 0, 1, 1, 'Draconic Compendium, Volume III (Study Hall: Combat Research)'),
(239341, 0, 120401, 100, 1, 1, 0, 1, 1, 'Tidestone Shard (The Tidestone: Shattered)'),
(245483, 0, 120401, 100, 1, 1, 0, 1, 1, 'Tidestone Shard (The Tidestone: Shattered)'),
(245484, 0, 120401, 100, 1, 1, 0, 1, 1, 'Tidestone Shard (The Tidestone: Shattered)'),
(245485, 0, 120401, 100, 1, 1, 0, 1, 1, 'Tidestone Shard (The Tidestone: Shattered)'),
(245486, 0, 120401, 100, 1, 1, 0, 1, 1, 'Tidestone Shard (The Tidestone: Shattered)');

-- The Headmistress' Keys: Elya Azuremoon (88275; Wowhead lists her and 88859 as dropping the key) carries it
UPDATE `creature_template_difficulty` SET `LootID` = 88275 WHERE `Entry` = 88275 AND `DifficultyID` = 0;
DELETE FROM `creature_loot_template` WHERE `Entry` = 88275;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(88275, 0, 120169, 100, 1, 1, 0, 1, 1, 'Elya Azuremoon - Tidestone Vault Key (The Headmistress'' Keys)');

-- Spawns: Sythorne by Wowhead's pin (no POI) beside Nidriel, Elya at 37730's POI 218,6482
DELETE FROM `creature` WHERE `guid` BETWEEN 12204167 AND 12204168;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204167, 107334, 1220, 7334, 0, '0', 0, 0, 0, 0, 188.0, 6480.0, -52.6, 4.71, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204168, 88275,  1220, 7334, 0, '0', 0, 0, 0, 0, 218.0, 6482.0, -51.8, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0);

-- Books by Wowhead's pins on the shelf by Sythorne, the podium beside him, the shards at 37469's POI points, the
-- Tidestone Core at its turn-in POI 304,6261
DELETE FROM `gameobject` WHERE `guid` BETWEEN 12201215 AND 12201224;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12201215, 250373, 1220, 7334, 0, '0', 0, 0, 0, -1, 179.0, 6486.0, -52.6, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0),
(12201216, 250372, 1220, 7334, 0, '0', 0, 0, 0, -1, 179.0, 6479.0, -52.6, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0),
(12201217, 250374, 1220, 7334, 0, '0', 0, 0, 0, -1, 181.0, 6482.0, -52.6, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0),
(12201218, 250362, 1220, 7334, 0, '0', 0, 0, 0, -1, 182.0, 6474.0, -52.9, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0),
(12201219, 239341, 1220, 7334, 0, '0', 0, 0, 0, -1, 251.0, 6294.0, -52.9, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0),
(12201220, 245483, 1220, 7334, 0, '0', 0, 0, 0, -1, 230.0, 6287.0, -52.2, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0),
(12201221, 245485, 1220, 7334, 0, '0', 0, 0, 0, -1, 238.0, 6281.0, -52.3, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0),
(12201222, 245484, 1220, 7334, 0, '0', 0, 0, 0, -1, 260.0, 6261.0, -53.7, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0),
(12201223, 245486, 1220, 7334, 0, '0', 0, 0, 0, -1, 246.0, 6250.0, -52.7, 0, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0),
(12201224, 240552, 1220, 7334, 0, '0', 0, 0, 0, -1, 304.0, 6261.0, -50.8, 3.14, 0, 0, 0, 1, 30, 255, 1, '', NULL, 0);

-- Who sees them
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND ((`SourceGroup` = 3 AND `SourceEntry` IN (107334, 88275))
    OR (`SourceGroup` = 5 AND `SourceEntry` IN (250372, 250373, 250374, 250362, 239341, 245483, 245484, 245485, 245486, 240552)));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 107334, 0, 0, 47, 0, 37518, 74, 0, '', 0, 0, 0, '', 'Sythorne is visible once quest 37518 (The Haunted Halls) is taken'),
(32, 3, 88275,  0, 0, 47, 0, 37730,  8, 0, '', 0, 0, 0, '', 'Elya Azuremoon is visible while quest 37730 (The Headmistress'' Keys) is in progress'),
(32, 5, 250372, 0, 0, 47, 0, 42371,  8, 0, '', 0, 0, 0, '', 'Courtship Rituals of the Skrog is visible while quest 42371 (Study Hall) is in progress'),
(32, 5, 250373, 0, 0, 47, 0, 42371,  8, 0, '', 0, 0, 0, '', 'Encyclopedia Azsunica (K-M) is visible while quest 42371 (Study Hall) is in progress'),
(32, 5, 250374, 0, 0, 47, 0, 42371,  8, 0, '', 0, 0, 0, '', 'Draconic Compendium, Volume III is visible while quest 42371 (Study Hall) is in progress'),
(32, 5, 250362, 0, 0, 47, 0, 42371,  8, 0, '', 0, 0, 0, '', 'Sythorne''s Podium is visible while quest 42371 (Study Hall) is in progress'),
(32, 5, 239341, 0, 0, 47, 0, 37469,  8, 0, '', 0, 0, 0, '', 'Tidestone Shard is visible while quest 37469 (The Tidestone: Shattered) is in progress'),
(32, 5, 245483, 0, 0, 47, 0, 37469,  8, 0, '', 0, 0, 0, '', 'Tidestone Shard is visible while quest 37469 (The Tidestone: Shattered) is in progress'),
(32, 5, 245484, 0, 0, 47, 0, 37469,  8, 0, '', 0, 0, 0, '', 'Tidestone Shard is visible while quest 37469 (The Tidestone: Shattered) is in progress'),
(32, 5, 245485, 0, 0, 47, 0, 37469,  8, 0, '', 0, 0, 0, '', 'Tidestone Shard is visible while quest 37469 (The Tidestone: Shattered) is in progress'),
(32, 5, 245486, 0, 0, 47, 0, 37469,  8, 0, '', 0, 0, 0, '', 'Tidestone Shard is visible while quest 37469 (The Tidestone: Shattered) is in progress'),
(32, 5, 240552, 0, 0, 47, 0, 37469, 10, 0, '', 0, 0, 0, '', 'Tidestone Core is visible while quest 37469 (The Tidestone: Shattered) is in progress or complete');
