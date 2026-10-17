-- Nythendra (creature 102672), the first boss of the Emerald Nightmare.
-- Positions were taken in game on 2026-10-17: her spot, and a circle fitted
-- through three points on the edge of the arena (instance script boundary).

-- Her template was unsniffed: faction 35 (friendly), no script
UPDATE `creature_template` SET `faction` = 14, `ScriptName` = 'boss_nythendra' WHERE `entry` = 102672;

-- Corrupted Vermin (Heart of the Swarm insects): hostile, cannot be targeted
-- (UNIT_FLAG_UNINTERACTIBLE). They are held in place by the boss script.
UPDATE `creature_template` SET `faction` = 14, `unit_flags` = 0x02000000, `speed_walk` = 1, `speed_run` = 1.14286 WHERE `entry` = 111005;

-- Raid warnings: 41 = raid boss emote, 42 = raid boss whisper to the target
DELETE FROM `creature_text` WHERE `CreatureID` = 102672;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(102672, 0, 0, '|TInterface\Icons\spell_nature_insect_swarm2:20|t Nythendra inhales for an |cFFFF0404|Hspell:202977|h[Infested Breath]|h|r!', 41, 0, 100, 0, 0, 0, 0, 106119, 0, 'Nythendra - Infested Breath'),
(102672, 1, 0, '|TInterface\Icons\Spell_Shadow_CreepingPlague:20|t Nythendra infests you with |cFFFF0404|Hspell:203096|h[Rot]|h|r!', 42, 0, 100, 0, 0, 0, 0, 105376, 0, 'Nythendra - Rot'),
(102672, 2, 0, '|TInterface\Icons\Ability_Rogue_DeviousPoisons:20|t Nythendra infests you with |cFFFF0404|Hspell:204463|h[Volatile Rot]|h|r!', 42, 0, 100, 0, 0, 0, 0, 105969, 0, 'Nythendra - Volatile Rot');

-- Spawn: Normal only (14); Heroic and Mythic extras are not built yet
DELETE FROM `creature` WHERE `guid` = 1520101;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(1520101, 102672, 1520, 0, 0, '14', 0, 0, 0, -1, 0, 0, 1886.4869, 1305.3795, 369.50168, 2.2541, 604800, 0, 0, 100, 0, 0, 0, 0, 0, '', NULL, 0);
