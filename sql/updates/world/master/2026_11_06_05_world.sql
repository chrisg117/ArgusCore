-- DK campaign: The Call of Icecrown, scenario 901 from "The Purge" to its end (Task 3e). Design: dk-frost-task3e-paper.md.
-- The Lich King's "It is done..." follows the reforging, then the death knight is drawn inside the blades and fights Arthas
-- while Ner'zhul, out of reach, lashes out (user, from retail). Arthas's death ends The Purge (event 45607, which nothing in
-- the client sends) and 186389 carries the death knight back to the throne. Taking the blades (247386, its loot 128292)
-- plays "Looting the Blades" and casts Blade Credit (2224, and "You have done well", conversation 750). At the Lich King's
-- seat he gives his blessing (1827) and his orders (2923), and the Acherus Waygate takes the death knight to Acherus

-- Arthas, the soul fought within the blades, and Ner'zhul beside him (95941, 95946: no script, no spawn in the client data)
UPDATE `creature_template` SET `faction`=14, `ScriptName`='npc_icc_scenario_purge_arthas' WHERE `entry`=95941;
UPDATE `creature_template` SET `faction`=14, `ScriptName`='npc_icc_scenario_purge_nerzhul' WHERE `entry`=95946;
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `Entry` IN (95941, 95946) AND `DifficultyID`=0;

-- The room inside Frostmourne is 631's: the player lands on its west trigger (495.7, -2523.8); Ner'zhul stands on its east
-- one, Arthas between them
DELETE FROM `creature` WHERE `guid` IN (14800022, 14800023);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14800022, 95941, 1480, 0, 0, '12', 0, 0, 0, 0, 505.50, -2523.50, 1049.95, 3.14159, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Arthas
(14800023, 95946, 1480, 0, 0, '12', 0, 0, 0, 0, 514.554, -2523.16, 1049.95, 3.14159, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Ner'zhul

-- The return to the throne: 186389 Blades of the Fallen Prince teleports to a DB destination it has no row for; the spot
-- facing the blades
DELETE FROM `spell_target_position` WHERE `ID`=186389 AND `EffectIndex`=3;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(186389, 3, 0, 1480, 518.00, -2124.60, 840.86, 3.14159, 0);

-- The blades: their loot (64479, empty) is the artifact itself, and the chest's script plays the scene and the credit
UPDATE `gameobject_template` SET `ScriptName`='go_icc_scenario_blades_of_the_fallen_prince' WHERE `entry`=247386;
DELETE FROM `gameobject_loot_template` WHERE `Entry`=64479;
INSERT INTO `gameobject_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(64479, 0, 128292, 100, 0, 1, 0, 1, 1, 'Blades of the Fallen Prince');

-- The Lich King's seat (631's Lich King sits at 428.6, -2123.9, 864.96): reaching it once the blades are taken. The
-- trigger stands at the foot of the throne, where the death knight can reach (the dais top is about z 861)
DELETE FROM `areatrigger_template` WHERE `Id`=14800006 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(14800006, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=14800006 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(14800006, 1, 14800006, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 10, 10, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId`=14800007;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(14800007, 14800006, 1, 1480, '12', 432.00, -2123.90, 862.00, 0, 0, 0, 0, 'at_icc_scenario_frozen_throne_seat', 'Icecrown Citadel (DK Frost scenario) - the Lich King''s seat: his blessing', 0);

-- The Acherus Waygate (242469, its spell 186039 Icecrown Waygate → 191275, a DB destination with no row): to Acherus's
-- lowest deck, before Mograine. Both Mograines stand on the spot the user pinged (2026-11-06): the arrival one (113695,
-- shown while The Call of Icecrown is under way) and the hall one (93437, shown once it is complete), who takes it
DELETE FROM `spell_script_names` WHERE `spell_id`=186039 AND `ScriptName`='spell_icc_scenario_icecrown_waygate';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(186039, 'spell_icc_scenario_icecrown_waygate');
DELETE FROM `spell_target_position` WHERE `ID`=191275 AND `EffectIndex`=0;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(191275, 0, 0, 1220, -1501.00, 1052.00, 260.40, 3.74, 0);
UPDATE `creature` SET `position_x`=-1507.11, `position_y`=1047.86, `position_z`=260.40, `orientation`=0.60 WHERE `guid` IN (12207000, 12207001);

-- The Lich King as a talking head (actor 99456 / 64642, as 14800001-07):
-- 14800008 lines 1782-1783 "It is done..." / "...purge them", after the reforging
-- 750      line 1784 "You have done well. Ascend to my throne...", the conversation Blade Credit's 197001 makes
-- 14800010 lines 1786-1787, the blessing at his seat
-- 14800011 lines 1788-1789, his orders
DELETE FROM `conversation_template` WHERE `Id` IN (14800008, 750, 14800010, 14800011);
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(14800008, 1782, 10000, 0, 'conversation_icc_scenario_spirits_remain', 0),
(750, 1784, 6000, 0, '', 0),
(14800010, 1786, 14000, 0, 'conversation_icc_scenario_the_lich_kings_blessing', 0),
(14800011, 1788, 13000, 0, '', 0);
DELETE FROM `conversation_actors` WHERE `ConversationId` IN (14800008, 750, 14800010, 14800011);
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(14800008, 14800001, 0, 0, 99456, 64642, 0, 0, 0),
(750, 14800001, 0, 0, 99456, 64642, 0, 0, 0),
(14800010, 14800001, 0, 0, 99456, 64642, 0, 0, 0),
(14800011, 14800001, 0, 0, 99456, 64642, 0, 0, 0);
DELETE FROM `conversation_line_template` WHERE `Id` IN (1782, 1783, 1784, 1786, 1787, 1788, 1789);
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(1782, 0, 296, 0, 0, 0),
(1783, 6000, 296, 0, 0, 0),
(1784, 0, 296, 0, 0, 0),
(1786, 0, 296, 0, 0, 0),
(1787, 7000, 296, 0, 0, 0),
(1788, 0, 296, 0, 0, 0),
(1789, 7000, 296, 0, 0, 0);

-- The Lich King on his throne for the whole scenario (99456, as the talking heads; 631's spawn spot, facing the platform):
-- the one the death knight ascends to. Seated, friendly, no gossip
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110 WHERE `Entry`=99456 AND `DifficultyID`=0;
DELETE FROM `creature` WHERE `guid`=14800024;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14800024, 99456, 1480, 0, 0, '12', 0, 0, 64642, 0, 428.597, -2123.88, 864.959, 0, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- The Lich King
DELETE FROM `creature_addon` WHERE `guid`=14800024;
INSERT INTO `creature_addon` (`guid`, `StandState`) VALUES
(14800024, 6); -- UNIT_STAND_STATE_SIT_HIGH_CHAIR
