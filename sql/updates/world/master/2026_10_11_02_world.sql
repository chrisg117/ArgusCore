-- "Blade in Twilight" (scenario 991): its seventh stage, "Dark Passage", the way from the hall of the tomb
-- down to the prison chamber.
--
-- Nothing stood on that way. What does is taken from a video of the retail scenario; where it stands was
-- measured on the tomb's collision (the stairs under the broken seal, the passage, the round chamber halfway
-- down) and placed along it by eye. Slaghammer's line is Blizzard's (broadcast_text 106116).

-- Borgoth the Master Reaver comes up the stairs when the way has opened, brought there by the scenario's
-- script. His template was a placeholder: friendly to all, of level 1, without a script. The Void Tendrils
-- were hostile and of levels 98 to 110 already, each of a level of its own between the two.
UPDATE `creature_template` SET `faction`=16, `ScriptName`='npc_borgoth_the_master_reaver' WHERE `entry`=101897;
UPDATE `creature_template` SET `ScriptName`='npc_darkwalk_void_tendril' WHERE `entry`=102927;
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `DifficultyID`=0 AND `Entry` IN (101897, 102927);

-- He carries a polearm in the video. Which item it is is not known; this is one that other creatures of
-- the Twilight's Hammer carry (a Twilight Zealot's).
DELETE FROM `creature_equip_template` WHERE `CreatureID`=101897 AND `ID`=1;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(101897, 1, 41889, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- What Slaghammer says of the Void Tendrils (his other lines in the tomb are in 2026_10_11_01_world.sql)
DELETE FROM `creature_text` WHERE `CreatureID`=101899 AND `GroupID`=3;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(101899, 3, 0, 'More o'' the Deacon''s mind games. We could Mass Dispel ''em or just fight through, but I would''na get too close!', 12, 0, 100, 0, 0, 63440, 0, 106116, 0, 'Shadowlord Slaghammer - the player comes to the Void Tendrils');

-- In the passage below the stairs, the Darkwalk: a pack of Flesh Spawn and a Twilight Bladetwister behind
-- them. At the northern edge of the round chamber, along which the way goes on: eight Void Tendrils.
DELETE FROM `creature` WHERE `guid` BETWEEN 15390024 AND 15390037;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15390024, 101120, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1880, 2333, 51, 0.3, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Flesh Spawn
(15390025, 101120, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1878, 2329, 50.33, 5.9, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Flesh Spawn
(15390026, 101120, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1876, 2332, 49.94, 0.1, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Flesh Spawn
(15390027, 101120, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1874, 2333.5, 49.83, 6.1, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Flesh Spawn
(15390028, 101120, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1873, 2329, 48.97, 0.4, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Flesh Spawn
(15390029, 101127, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 1866, 2330, 48.81, 0, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Bladetwister
(15390030, 102927, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1851, 2318, 46.31, 1.2, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Void Tendril
(15390031, 102927, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1848, 2314, 46.69, 0.6, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Void Tendril
(15390032, 102927, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1852, 2311, 46.18, 0.2, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Void Tendril
(15390033, 102927, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1847, 2306, 47.3, 5.9, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Void Tendril
(15390034, 102927, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1850, 2305, 47.17, 0, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Void Tendril
(15390035, 102927, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1845, 2316, 47.16, 0.9, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Void Tendril
(15390036, 102927, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1849, 2300, 46.95, 5.6, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Void Tendril
(15390037, 102927, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1844, 2303, 47.37, 5.8, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Void Tendril
