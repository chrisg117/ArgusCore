-- DH campaign, The Hunt and Making Arrangements. The Kayn or Altruis at Krasus' Landing who sends the player to the Violet Hold
-- leaves when the quest is taken, and the one at the hold appears: ours, no source says how retail hands over.
-- The Hunt (Kayn) had no quest POI: it takes those sniffed for its Altruis twin, 41119.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (99247, 99254, 94902, 103156) AND `ConditionTypeOrReference`=47 AND `ConditionValue1` IN (40819, 41120);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 99247, 0, 0, 47, 0, 40819, 1, 0, 0, 'Kayn at Krasus'' Landing: until Making Arrangements is taken'),
(32, 3, 99254, 0, 0, 47, 0, 41120, 1, 0, 0, 'Altruis at Krasus'' Landing: until Making Arrangements is taken'),
(32, 3, 94902, 0, 0, 47, 0, 40819, 74, 0, 0, 'Kayn at the Violet Hold: once Making Arrangements is taken'),
(32, 3, 103156, 0, 0, 47, 0, 41120, 74, 0, 0, 'Altruis at the Violet Hold: once Making Arrangements is taken');

DELETE FROM `quest_poi` WHERE `QuestID`=39247;
INSERT INTO `quest_poi` (`QuestID`, `BlobIndex`, `Idx1`, `ObjectiveIndex`, `QuestObjectiveID`, `QuestObjectID`, `MapID`, `WorldMapAreaId`, `Floor`, `Priority`, `Flags`, `WorldEffectID`, `PlayerConditionID`, `SpawnTrackingID`, `AlwaysAllowMergingBlobs`, `VerifiedBuild`)
SELECT 39247, `BlobIndex`, `Idx1`, `ObjectiveIndex`, IF(`QuestObjectiveID`=281890, 279376, `QuestObjectiveID`), `QuestObjectID`, `MapID`, `WorldMapAreaId`, `Floor`, `Priority`, `Flags`, `WorldEffectID`, `PlayerConditionID`, `SpawnTrackingID`, `AlwaysAllowMergingBlobs`, `VerifiedBuild` FROM `quest_poi` WHERE `QuestID`=41119;
DELETE FROM `quest_poi_points` WHERE `QuestID`=39247;
INSERT INTO `quest_poi_points` (`QuestID`, `Idx1`, `Idx2`, `X`, `Y`, `VerifiedBuild`)
SELECT 39247, `Idx1`, `Idx2`, `X`, `Y`, `VerifiedBuild` FROM `quest_poi_points` WHERE `QuestID`=41119;

-- Stage 3 spawn heights, from the navmesh. The first ones came from teleport probes that read the ground under a rock.
UPDATE `creature` SET `position_z`=56.89 WHERE `guid`=12204109;
UPDATE `creature` SET `position_z`=63.08 WHERE `guid`=12204110;
UPDATE `creature` SET `position_x`=1130.67, `position_y`=5047.87, `position_z`=49.58 WHERE `guid`=12204111;
UPDATE `creature` SET `position_z`=56.93 WHERE `guid`=12204112;
UPDATE `creature` SET `position_z`=57.87 WHERE `guid`=12204113;
UPDATE `creature` SET `position_z`=56.74 WHERE `guid`=12204114;
UPDATE `creature` SET `position_z`=51.78 WHERE `guid`=12204115;
UPDATE `creature` SET `position_z`=53.08 WHERE `guid`=12204116;
UPDATE `creature` SET `position_z`=54.03 WHERE `guid`=12204117;
UPDATE `creature` SET `position_z`=50.47 WHERE `guid`=12204118;
UPDATE `creature` SET `position_z`=55.55 WHERE `guid`=12204119;
UPDATE `creature` SET `position_z`=55.95 WHERE `guid`=12204120;
UPDATE `creature` SET `position_z`=51.06 WHERE `guid`=12204121;
UPDATE `creature` SET `position_z`=50.91 WHERE `guid`=12204122;
UPDATE `creature` SET `position_z`=55.01 WHERE `guid`=12204123;
UPDATE `creature` SET `position_z`=63.37 WHERE `guid`=12204124;
UPDATE `creature` SET `position_z`=50.45 WHERE `guid`=12204125;
UPDATE `creature` SET `position_z`=55.77 WHERE `guid`=12204126;
UPDATE `creature` SET `position_z`=59.56 WHERE `guid`=12204127;
UPDATE `gameobject` SET `position_z`=58.95 WHERE `guid`=12204001;
UPDATE `gameobject` SET `position_z`=49.25 WHERE `guid`=12204002;
UPDATE `gameobject` SET `position_z`=60.81 WHERE `guid`=12204003;

-- Stage 5: the Twinblades. Spell 189524 (from the client) creates the item from its loot, plays the scene and forces it equipped.
-- Kor'vas takes the turn-in at the sniffed POI of 41119's exit. Ours: that she leaves with the quest, until Task 4 needs her.
UPDATE `gameobject_template` SET `ScriptName`='go_twinblades_of_the_deceiver' WHERE `entry`=251878;
DELETE FROM `spell_loot_template` WHERE `Entry`=189524;
INSERT INTO `spell_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(189524, 0, 127829, 100, 0, 1, 0, 1, 1, 'The Twinblades of the Deceiver');

DELETE FROM `creature` WHERE `guid`=12204128;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204128, 102799, 1220, 0, 0, '0', 0, 0, 0, 0, -837.00, 4264.00, 746.28, 4.2, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry`=102799;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 102799, 0, 0, 47, 0, 39247, 10, 0, 0, 'Kor''vas at Krasus'' Landing: with The Hunt (Kayn)'),
(32, 3, 102799, 0, 1, 47, 0, 41119, 10, 0, 0, 'Kor''vas at Krasus'' Landing: with The Hunt (Altruis)');

DELETE FROM `spell_script_names` WHERE `spell_id`=189523;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(189523, 'spell_felsoul_hold_equip_twinblades');

-- A range here is a random level at spawn, and health follows it: Varedis had 522 k at 98 and 1.6 M at 101. Ours, one level.
UPDATE `creature_template_difficulty` SET `MinLevel`=100, `MaxLevel`=100 WHERE `DifficultyID`=0 AND `Entry` IN (94375, 95313, 95423, 95285, 95452, 94836, 94900);
