-- Legion Dalaran: the flight master. Krasus' Landing had the client's taxi node 1774 (-860.83, 4295.95, 745.24) but no
-- Aludane Whitecloud (96813): his Legion template was a bare placeholder (faction 35, npcflag 0). Faction, flags and the
-- gossip link (menu 18723, "I need a ride.", already in gossip_menu/gossip_menu_option) are TrinityCore's own values for
-- this entry (TDB 12.1 build 69497); nothing else of that schema is copied. Position: Wowhead pin centre (69.8, 50.6) on
-- Dalaran floor 10 -> (-861.0, 4298.7), 2.8 yd from the taxi node; z 745.19 is the landing's floor from Dalaran2.wmo.
UPDATE `creature_template` SET `faction`=2007, `npcflag`=8193, `unit_flags`=0, `subname`='Flight Master' WHERE `entry`=96813;

DELETE FROM `creature_template_gossip` WHERE `CreatureID`=96813;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES (96813, 18723, 48676);

DELETE FROM `creature` WHERE `guid`=12203000;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12203000, 96813, 1220, 7502, 7505, '0', 0, 0, 0, -1, 0, 0, -861.00, 4298.70, 745.19, 4.71, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Aludane Whitecloud, Flight Master, Krasus' Landing
