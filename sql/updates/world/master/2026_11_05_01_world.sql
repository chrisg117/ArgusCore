-- DK campaign: The Call of Icecrown, scenario 901 step 1810 "Seek the Fragments" (Task 3b-1, the four souls).
-- Design: dk-frost-task3b-design.md, corrected from the user's retail recording (youtu.be/MqN9SFayjfY, from 3:09).
-- Positions are staged from the 631 Spire layout and the wiki's directions (no sniff) for the user to correct; the kits
-- are the abilities the recording shows by name, with ids chosen from the client's spells (no sniff).

-- The four souls of Frostmourne: hostile, scripted, scaled to the player (sniffed at a flat 110; the scenario is level 98+)
UPDATE `creature_template` SET `faction`=14, `ScriptName`='npc_icc_scenario_fragment_soul' WHERE `entry` IN (93948, 93949, 93950, 93951);
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `Entry` IN (93948, 93949, 93950, 93951) AND `DifficultyID`=0;

DELETE FROM `creature` WHERE `guid` BETWEEN 14800001 AND 14800004;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14800001, 93948, 1480, 0, 0, '12', 0, 0, 0, 0, 4240.00, 2769.00, 351.20, 3.14159, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Baelgun Flamebeard, the Spire
(14800002, 93949, 1480, 0, 0, '12', 0, 0, 0, 0, 4420.00, 2700.00, 344.15, 2.29000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Halahk the Deathbringer, northeast
(14800003, 93950, 1480, 0, 0, '12', 0, 0, 0, 0, 4420.00, 2840.00, 344.15, 4.01000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Magroth the Defender, northwest
(14800004, 93951, 1480, 0, 0, '12', 0, 0, 0, 0, 4360.00, 2769.00, 356.15, 3.14159, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Gavinrad the Cruel, central Spire

-- Each fragment lies where its soul guards it (the recording: an ice-encased shard beside the soul). It cannot be
-- selected until the soul dies. Its loot is item 127009, whose on-looted spell 218934 sends event 45689: one of four
DELETE FROM `gameobject` WHERE `guid` BETWEEN 14800001 AND 14800004;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14800001, 251882, 1480, 0, 0, '12', 0, 0, 0, -1, 4244.50, 2769.00, 351.14, 3.14159, 0, 0, 1, 0, 86400, 255, 1, '', NULL, 0), -- Baelgun's fragment
(14800002, 251884, 1480, 0, 0, '12', 0, 0, 0, -1, 4422.60, 2697.00, 344.15, 2.29000, 0, 0, 0.9108, 0.4128, 86400, 255, 1, '', NULL, 0), -- Halahk's fragment
(14800003, 251885, 1480, 0, 0, '12', 0, 0, 0, -1, 4422.60, 2843.10, 344.15, 4.01000, 0, 0, 0.9070, -0.4212, 86400, 255, 1, '', NULL, 0), -- Magroth's fragment
(14800004, 251886, 1480, 0, 0, '12', 0, 0, 0, -1, 4364.00, 2769.00, 356.15, 3.14159, 0, 0, 1, 0, 86400, 255, 1, '', NULL, 0); -- Gavinrad's fragment
DELETE FROM `gameobject_overrides` WHERE `spawnId` BETWEEN 14800001 AND 14800004;
INSERT INTO `gameobject_overrides` (`spawnId`, `faction`, `flags`) VALUES
(14800001, 0, 16), -- GO_FLAG_NOT_SELECTABLE until the soul dies
(14800002, 0, 16),
(14800003, 0, 16),
(14800004, 0, 16);
DELETE FROM `gameobject_loot_template` WHERE `Entry`=60754;
INSERT INTO `gameobject_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(60754, 0, 127009, 100, 0, 1, 0, 1, 1, 'Fragment of Frostmourne');

-- The Lich King on the way: 1761/1762 on entering the Spire, one line as the player nears each soul. Talking heads shaped
-- like 14800001 (the portrait is a known gap). End times follow the text length and the line's additional duration
DELETE FROM `conversation_template` WHERE `Id` BETWEEN 14800002 AND 14800006;
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(14800002, 1761, 16000, 0, '', 0), -- Seek the Fragments: the souls wander the halls
(14800003, 1768, 12000, 0, '', 0), -- Baelgun
(14800004, 1769, 12000, 0, '', 0), -- Halahk
(14800005, 1770, 12000, 0, '', 0), -- Magroth
(14800006, 1771, 12000, 0, '', 0); -- Gavinrad
DELETE FROM `conversation_actors` WHERE `ConversationId` BETWEEN 14800002 AND 14800006;
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(14800002, 14800001, 0, 0, 99456, 64642, 0, 0, 0),
(14800003, 14800001, 0, 0, 99456, 64642, 0, 0, 0),
(14800004, 14800001, 0, 0, 99456, 64642, 0, 0, 0),
(14800005, 14800001, 0, 0, 99456, 64642, 0, 0, 0),
(14800006, 14800001, 0, 0, 99456, 64642, 0, 0, 0);
DELETE FROM `conversation_line_template` WHERE `Id` IN (1761, 1762, 1768, 1769, 1770, 1771);
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(1761, 0, 296, 0, 0, 0),
(1762, 8500, 296, 0, 0, 0),
(1768, 0, 296, 0, 0, 0),
(1769, 0, 296, 0, 0, 0),
(1770, 0, 296, 0, 0, 0),
(1771, 0, 296, 0, 0, 0);
