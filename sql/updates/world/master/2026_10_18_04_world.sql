-- Il'gynoth (creature 105393) and the Eye of Il'gynoth (summoned by the heart).
-- Positions were taken in game on 2026-10-18 (.gps readings in GM.log).

-- The templates were unsniffed: faction 35 (friendly), no script. Only the heart and the Eye
-- have an AI so far; the add templates get their ScriptName with their scripts.
UPDATE `creature_template` SET `faction` = 14, `ScriptName` = 'boss_ilgynoth' WHERE `entry` = 105393;
UPDATE `creature_template` SET `faction` = 14, `ScriptName` = 'npc_eye_of_ilgynoth' WHERE `entry` = 105906;

-- The Eye: immune to knockback (CREATURE_FLAG_EXTRA_IMMUNITY_KNOCKBACK). This fork has no
-- no-threat bit in flags_extra; the Eye is rooted by script and never chases.
UPDATE `creature_template` SET `flags_extra` = `flags_extra` | 0x40000000 WHERE `entry` = 105906;

-- Raid warnings (14 = yell, 41 = raid boss emote, 42 = raid boss whisper)
DELETE FROM `creature_text` WHERE `CreatureID` IN (105393, 105906, 105304);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(105393, 0, 0, 'Come, little ones. Savor your fear as we feast upon it!', 14, 0, 100, 0, 0, 0, 0, 109119, 0, 'Il''gynoth - Aggro'),
(105393, 1, 0, 'Every blow struck at our heart will be repaid a thousand fold!', 14, 0, 100, 0, 0, 0, 0, 109141, 0, 'Il''gynoth - Heart phase'),
(105393, 2, 0, 'Out! Out! You are not yet worthy!', 14, 0, 100, 0, 0, 0, 0, 109142, 0, 'Il''gynoth - Reconstituted'),
(105393, 3, 0, 'Still you fight? Still you refuse the gift?', 14, 0, 100, 0, 0, 0, 0, 109143, 0, 'Il''gynoth - Low health'),
(105393, 4, 0, 'N''zoth... I journey... to Ny''alotha...', 14, 0, 100, 0, 0, 0, 0, 109144, 0, 'Il''gynoth - Death'),
(105393, 5, 0, '|TInterface\\Icons\\Ability_Rogue_EnvelopingShadows:20|t Il''gynoth has almost completed his |cFFFF0000|Hspell:210781|h[Dark Reconstitution]|h|r!', 41, 0, 100, 0, 0, 0, 0, 109470, 0, 'Il''gynoth - Dark Reconstitution almost completed'),
(105393, 6, 0, '|TInterface\\Icons\\sha_ability_rogue_envelopingshadows_nightmare:20|t Il''gynoth has almost completed his |cFFFF0000|Hspell:223121|h[Final Torpor]|h|r!', 41, 0, 100, 0, 0, 0, 0, 122666, 0, 'Il''gynoth - Final Torpor almost completed'),
(105906, 0, 0, 'The Eye of Il''gynoth is creating a Nightmare Horror!', 41, 0, 100, 0, 0, 0, 0, 111379, 0, 'Eye of Il''gynoth - Nightmare Horror'),
(105304, 0, 0, '|TInterface\\Icons\\Achievement_Boss_YoggSaron_01:20|t A Dominator Tentacle faces you and readies |cFFFF0000|Hspell:208689|h[Ground Slam]|h|r!', 42, 0, 100, 0, 0, 0, 0, 109189, 0, 'Dominator Tentacle - Ground Slam');

-- Spawn: the heart sits in the chamber (Normal only, 14). The Eye is not a DB spawn: the heart
-- summons it. The DELETE also cleans a database that applied an earlier draft with an Eye row.
DELETE FROM `creature` WHERE `guid` IN (1520102, 1520103);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(1520102, 105393, 1520, 0, 0, '14', 0, 0, 0, -1, 0, 0, -12793.001, 12352.525, -244.93068, 3.919, 604800, 0, 0, 100, 0, 0, 0, 0, 0, '', NULL, 0);

-- Eye of Il'gynoth's Forcefield (251555) at the hollow mouth, closed (state 1)
DELETE FROM `gameobject` WHERE `guid` = 1520003;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(1520003, 251555, 1520, 0, 0, '14', 0, 0, 0, -1, -12816.817, 12329.056, -246.30473, 3.730643, 0, 0, 0.956940, -0.290286, 604800, 255, 1, '', NULL, 0);
