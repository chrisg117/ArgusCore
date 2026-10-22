-- Before We're Overrun (38766): Doom Commander Beliash (93221) was never spawned, so "Beliash slain & power taken"
-- could not be done. Position from Wowhead's map pins (Mardum 35.2, 39.4), converted with a fit through four spawned
-- Mardum NPCs; z from the map files. Phase 0 like the Doomguard Eradicators around him.
DELETE FROM `creature` WHERE `guid` = 14810001;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14810001, 93221, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1592.4, 2711.7, 19.3, 4.57, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0);

-- On death his attackers channel Taking Power (210093) on his corpse; finishing the channel gives the credit
UPDATE `creature_template` SET `ScriptName` = 'npc_doom_commander_beliash' WHERE `entry` = 93221;

DELETE FROM `spell_script_names` WHERE `spell_id` = 210093 AND `ScriptName` = 'spell_before_were_overrun_taking_power';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210093, 'spell_before_were_overrun_taking_power');

-- The channel's periodic dummy (210091) pulses at his corpse
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 210091;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 210091, 0, 0, 31, 0, 3, 93221, 0, '', 0, 0, 0, '','Before We''re Overrun: Taking Power 02 - target Doom Commander Beliash');
