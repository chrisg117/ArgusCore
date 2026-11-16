-- The New Blood, scenario 962 on map 1522: the layout of the live realm recording (2026-11-15). Places come from the
-- recording's map-coordinate readout, calibrated against our client on this map, and from view-matching at each stop.

-- The landing: inside the Jailer Cage on the pit's rim (video 2:52, the player wakes in the cell)
UPDATE `spell_target_position` SET `MapID` = 1522, `PositionX` = 2961.2, `PositionY` = 1126.8, `PositionZ` = 206.75, `Orientation` = 0 WHERE `ID` = 199359 AND `EffectIndex` = 0;

-- The cage stands closed for the whole scenario (the script roots the player in it until Beshtal is enslaved)
DELETE FROM `gameobject` WHERE `guid` IN (15220003, 15229900);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15220003, 242987, 1522, 0, 0, '12', 0, 0, 0, -1, 2961.2, 1126.8, 206.6, 0, 0, 0, 0, 1, 86400, 255, 1, '', NULL, 0);

-- Ritssyn in the cage beside the player; Beshtal on the dais, enslaved from the cage through the extra action button
-- (aura 199338), so his click goes
UPDATE `creature` SET `position_x` = 2962.5, `position_y` = 1127.9, `position_z` = 206.7, `orientation` = 5.6 WHERE `guid` = 15220000;
UPDATE `creature` SET `position_x` = 2979.5, `position_y` = 1142.5, `position_z` = 207.25, `orientation` = 3.92, `npcflag` = NULL WHERE `guid` = 15220001;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 98836 AND `spell_id` = 199336;

-- The shattered council on their spike pillars (video 5:22, 6:06, 6:26), released by a click: 192217 Releasing. The
-- click flag is on these spawns only, not the templates, so the followers that replace them are not clickable
UPDATE `creature` SET `position_x` = 2978.0, `position_y` = 1206.6, `position_z` = 224.1, `orientation` = 3.14, `npcflag` = 16777216 WHERE `guid` = 15220002;
UPDATE `creature` SET `position_x` = 2899.3, `position_y` = 1101.5, `position_z` = 223.6, `orientation` = 0.31, `npcflag` = 16777216 WHERE `guid` = 15220003;
UPDATE `creature` SET `position_x` = 2966.0, `position_y` = 1065.9, `position_z` = 224.0, `orientation` = 1.98, `npcflag` = 16777216 WHERE `guid` = 15220004;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (99147, 99145, 99146);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(99147, 192217, 1, 0),
(99145, 192217, 1, 0),
(99146, 192217, 1, 0);

-- Hostiles along the way: the Torturers beside Zinnin's and Shinfel's pillars (6:08, 6:26), a Dreadscar Jailer at
-- Zinnin's stop (6:06). The Sunderer on the approach and the felhound at Jubeka's pillar are rough places
UPDATE `creature` SET `position_x` = 2955.5, `position_y` = 1158.5, `position_z` = 216.3, `orientation` = 0.1 WHERE `guid` = 15220011;
UPDATE `creature` SET `position_x` = 2975.5, `position_y` = 1202.8, `position_z` = 221.3, `orientation` = 4.4 WHERE `guid` = 15220012;
UPDATE `creature` SET `position_x` = 2901.0, `position_y` = 1104.0, `position_z` = 220.9, `orientation` = 3.93 WHERE `guid` = 15220013;
UPDATE `creature` SET `position_x` = 2968.6, `position_y` = 1067.0, `position_z` = 221.3, `orientation` = 3.85 WHERE `guid` = 15220014;
UPDATE `creature` SET `position_x` = 2906.0, `position_y` = 1108.0, `position_z` = 220.81, `orientation` = 3.9 WHERE `guid` = 15220005;

-- The archive is the walled rune circle on the upper level: Nelach before it (8:04), the Tome on its centre (8:31)
UPDATE `creature` SET `position_x` = 3093.5, `position_y` = 987.0, `position_z` = 257.4, `orientation` = 1.57 WHERE `guid` = 15220008;
UPDATE `gameobject` SET `position_x` = 3095.7, `position_y` = 966.6, `position_z` = 257.8 WHERE `guid` = 15220002;

-- Jailbreak ends where the player stops before Calydus past the fel barrier (7:12)
UPDATE `areatrigger` SET `PosX` = 3071.8, `PosY` = 1070.7, `PosZ` = 238.7 WHERE `SpawnId` = 15220000 AND `IsCustom` = 1;

-- The click's hit releases the captive
DELETE FROM `spell_script_names` WHERE `spell_id` = 192217 AND `ScriptName` = 'spell_dreadscar_releasing';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(192217, 'spell_dreadscar_releasing');
