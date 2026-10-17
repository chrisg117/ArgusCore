-- Broken Shore scenario (map 1460), Alliance side: the two gaps that Jaina Proudmoore bridges with ice.
-- The first lies between the Black City and the crevasse, on the way to Tirion; the second between the ledge
-- where Krosus is fought and the path to the Tomb of Sargeras. Neither can be crossed on foot without a bridge.
--
-- A bridge is two gameobjects in the same place:
--
-- What is seen is the client's model world/expansion06/doodads/brokenshore/7bs_brokenshore_icebridge01.m2,
-- display 35871 in GameObjectDisplayInfo.db2. It has no collision. No gameobject using that display is in this
-- database, and none was found on Wowhead by name, so the object Blizzard spawns is unknown and the template
-- below is custom: its entry is not Blizzard's.
--
-- What is walked on is "WMO Bridge" (242549), a template this database already has. Its display, 36419, is
-- world/wmo/brokenisles/brokenshore/7bs_brokenshore_bridgecollision01.wmo, which draws nothing and has the shape
-- of the ice bridge in the model's own coordinates, so it takes the model's position and rotation. A second
-- template with that name and display exists (254234). Which of the two belongs to which bridge, or to which
-- faction's version of the scenario, is not known.
--
-- The deck rises 15.9 yards over 56, which is the shape of both gaps, so the bridges are placed without any
-- tilt. The line of each bridge is between two points marked in game. Where along that line the model sits, and
-- at what height, was fitted to the terrain at both rims of the gap.

DELETE FROM `gameobject_template` WHERE `entry`=1460001;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `VerifiedBuild`) VALUES
(1460001, 0, 35871, 'Ice Bridge', 1, 0); -- a door, so that its state picks the model's animation

-- flags: GO_FLAG_NOT_SELECTABLE and GO_FLAG_NODESPAWN, so that nobody can click the bridge shut
DELETE FROM `gameobject_template_addon` WHERE `entry`=1460001;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`) VALUES
(1460001, 0, 0x30);

-- Each bridge is its own group, spawned by the instance script a few seconds into a stage: the first in
-- "The Highlord", the second in "Stop Gul'dan". They are not part of those stages' groups because they stay.
DELETE FROM `spawn_group_template` WHERE `groupId` IN (14609, 14610);
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14609, 'Broken Shore Scenario - Bridge to the Highlord (Alliance)', 4),
(14610, 'Broken Shore Scenario - Bridge to Gul''dan (Alliance)', 4);

-- The bridge is spawned as a closed door (state 1). The model's animation of the ice forming is the one a
-- door plays when it opens, and a client only plays that for a door it already has, so the instance script
-- opens the bridge a moment after it appears. An open door then shows the finished bridge.
DELETE FROM `gameobject` WHERE `guid` IN (14600600, 14600601, 14600800, 14600801);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14600600, 1460001, 1460, 7534, 8120, '12', 0, 0, 0, -1, 1417.1372, 2110.0315, 21.0000, 1.6699, 0.000000, 0.000000, 0.741277, 0.671199, 7200, 255, 1, '', NULL, 0), -- bridge to the Highlord
(14600601, 242549, 1460, 7534, 8120, '12', 0, 0, 0, -1, 1417.1372, 2110.0315, 21.0000, 1.6699, 0.000000, 0.000000, 0.741277, 0.671199, 7200, 255, 1, '', NULL, 0), -- its collision
(14600800, 1460001, 1460, 7534, 8120, '12', 0, 0, 0, -1, 1538.4834, 1778.2597, 36.7300, 1.9304, 0.000000, 0.000000, 0.822171, 0.569241, 7200, 255, 1, '', NULL, 0), -- bridge to Gul'dan
(14600801, 242549, 1460, 7534, 8120, '12', 0, 0, 0, -1, 1538.4834, 1778.2597, 36.7300, 1.9304, 0.000000, 0.000000, 0.822171, 0.569241, 7200, 255, 1, '', NULL, 0); -- its collision

DELETE FROM `spawn_group` WHERE `spawnType`=1 AND `spawnId` IN (14600600, 14600601, 14600800, 14600801);
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14609, 1, 14600600),
(14609, 1, 14600601),
(14610, 1, 14600800),
(14610, 1, 14600801);
