-- Eye of Azshara (map 1456) scaffold: instance script, encounter row, boss respawn,
-- the Wrath of Azshara's spawn, and the level and faction fix (template rules L1, L5, L6).
-- Spec: docs/superpowers/specs/2026-11-16-dungeon-eye-of-azshara-design.md (local-only).

-- Instance script. Parent 0, as the other Legion dungeons on this core.
DELETE FROM `instance_template` WHERE `map` = 1456;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1456, 0, 'instance_eye_of_azshara');

-- Wrath of Azshara's final-encounter row held her journal encounter id (1492). The Normal
-- LFGDungeons row is 1174, as the sibling dungeons' final rows hold their Normal LFG id.
UPDATE `instance_encounters` SET `lastEncounterDungeon` = 1174 WHERE `entry` = 1814;

-- Bosses never respawn inside a run.
UPDATE `creature` SET `spawntimesecs` = 604800 WHERE `guid` IN (6003702, 6003724, 6003719, 6003772);

-- Aura 355784 is not in this client's Spell.db2: drop it from Parjesh and Deepbeard.
UPDATE `creature_template_addon` SET `auras` = '202146' WHERE `entry` = 91784;
UPDATE `creature_template_addon` SET `auras` = '' WHERE `entry` = 91797;

-- The Wrath of Azshara has no TDB spawn. Provisional spot: the Bubble object (guid 6002449)
-- at the island's centre; position and facing are a measured input (user's or a video read).
DELETE FROM `creature` WHERE `guid` = 6005486;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `VerifiedBuild`) VALUES
(6005486, 96028, 1456, 8040, 8081, '1,2,8,23', 0, 0, 0, -1, 0, 0, -3484.82, 4387.02, -3.66375, 0, 604800, 0, 0, 100, 0, 0);

-- Rule L1, spawned level-1 fighters: level = 110 + the sniffed delta (scaling 98-110 kept).
UPDATE `creature_template_difficulty` SET `MinLevel` = 112, `MaxLevel` = 112
 WHERE `DifficultyID` = 0 AND `Entry` IN (91784, 91789, 91797, 91808);
UPDATE `creature_template_difficulty` SET `MinLevel` = 111, `MaxLevel` = 111
 WHERE `DifficultyID` = 0 AND `Entry` IN (91782, 91792, 91796, 95939, 97171, 98173, 100248, 100249, 100250);

-- Rule L1, entries with no fighting spawn that fights summon.
-- Wrath of Azshara and the Blazing Hydra Spawn keep their sniffed delta 2.
UPDATE `creature_template_difficulty` SET `MinLevel` = 112, `MaxLevel` = 112
 WHERE `DifficultyID` = 0 AND `Entry` IN (96028, 97259);
-- Arcane Hydra Spawn: no sniffed delta; mirrors the Blazing spawn (user ruling 2026-11-26).
UPDATE `creature_template_difficulty` SET `MinLevel` = 112, `MaxLevel` = 112, `LevelScalingMin` = 98, `LevelScalingMax` = 110,
 `LevelScalingDeltaMin` = 2, `LevelScalingDeltaMax` = 2, `HealthScalingExpansion` = 6
 WHERE `DifficultyID` = 0 AND `Entry` = 97260;
-- Boss adds and the casters of damaging boss spells (rule L3): 110, scaling 98-110, delta 0.
-- 97264 Shellbreaker, 97269 Crestrider, 98293 Saltsea Globule, 97673 Mystic Tornado,
-- 99852 Monsoon, 97916 Quake.
UPDATE `creature_template_difficulty` SET `MinLevel` = 110, `MaxLevel` = 110, `LevelScalingMin` = 98, `LevelScalingMax` = 110,
 `LevelScalingDeltaMin` = 0, `LevelScalingDeltaMax` = 0, `HealthScalingExpansion` = 6
 WHERE `DifficultyID` = 0 AND `Entry` IN (97264, 97269, 98293, 97673, 99852, 97916);
-- Call the Seas stalkers carry delta 2 at level 110: make level and delta agree.
UPDATE `creature_template_difficulty` SET `MinLevel` = 112, `MaxLevel` = 112
 WHERE `DifficultyID` = 0 AND `Entry` = 97844;

-- Rule L5, faction 16: the five bosses, the 30 spawned Enemy Forces entries of scenario 1169
-- that TDB has at 35 (97170 is already 16), and the five adds boss spells summon.
-- Stalker factions are decided by each boss task.
UPDATE `creature_template` SET `faction` = 16 WHERE `entry` IN
(91784, 91789, 91797, 91808, 96028,
 91781, 91782, 91783, 91785, 91786, 91787, 91790, 91792, 91793, 91794, 91796, 95861, 95920, 95939, 95940, 95947,
 97171, 97172, 97173, 98173, 99629, 99630, 100216, 100248, 100249, 100250, 101414, 106785, 106786, 106787,
 97264, 97269, 98293, 97259, 97260);
