-- Azsuna, Leyline Abuse (37960), fix round 2026-11-27: Ishkaneth (90389) and Tyndrissen (90390) had no spawn here or in
-- the retail TrinityCore data (TDB 1210), so they are hand-placed on the quest's objective POIs (996,6087 and 1164,6005),
-- moved to the nearest point at least 4 yd from walls (navplace). Faction 2635, the retail value for both (hostile
-- Nightborne; FactionTemplate 2635 exists in the 7.3.5 client data); here they were 35, friendly. Their kills credit the
-- quest through KillCredit1 90468 (Nightfallen Elf: Lieutenant).
-- Not covered: the hidden, optional Ael'yith Approach Credit (100377, spell 197522); nothing in either data set casts it.
UPDATE `creature_template` SET `faction` = 2635 WHERE `entry` IN (90389, 90390);

DELETE FROM `creature` WHERE `guid` BETWEEN 12205320 AND 12205321;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12205320, 90389, 1220, 7334, 0, '0', 0, 0, 0, 0,  994.27, 6088.00, 100.37, 0, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12205321, 90390, 1220, 7334, 0, '0', 0, 0, 0, 0, 1157.46, 6007.50,  80.50, 0, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0);
