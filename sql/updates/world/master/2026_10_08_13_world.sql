-- Broken Shore scenario (map 1460), "Storm the Beach": the Spires of Woe themselves, and three Anchoring
-- Crystals for each of them instead of one.
--
-- A Spire of Woe is a gameobject. Five templates of that name share one model (7LG_Legion_Spire01, or its twin
-- with other animations); which of them Blizzard spawns here is not known, so 240194 is a guess: it is the only
-- one verified in a 7.0.3 build, the patch the scenario came with. Its flags are not known either. A goober
-- without a lock can be used by anyone, so the spawns are made unselectable, and they are not to despawn
-- (GO_FLAG_NOT_SELECTABLE | GO_FLAG_NODESPAWN). They stand where the single crystals stood, turned as those were.
--
-- They are a spawn group of their own, spawned by the instance script when the stage starts, because their
-- wreckage stays when the stage ends.
DELETE FROM `spawn_group_template` WHERE `groupId`=14613;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14613, 'Broken Shore Scenario - Spires of Woe', 4);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 14600140 AND 14600142;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14600140, 240194, 1460, 7534, 8290, '12', 0, 0, 0, -1, 512.5, 2155.9, 4.89, 4.0013, 0.000000, 0.000000, 0.909027, -0.416738, 7200, 255, 1, '', NULL, 0),
(14600141, 240194, 1460, 7534, 8290, '12', 0, 0, 0, -1, 546.8, 2117.4, 4.71, 3.5226, 0.000000, 0.000000, 0.981909, -0.189353, 7200, 255, 1, '', NULL, 0),
(14600142, 240194, 1460, 7534, 8290, '12', 0, 0, 0, -1, 569.0, 2059.7, 3.71, 3.0111, 0.000000, 0.000000, 0.997872, 0.065200, 7200, 255, 1, '', NULL, 0);

DELETE FROM `gameobject_overrides` WHERE `spawnId` BETWEEN 14600140 AND 14600142;
INSERT INTO `gameobject_overrides` (`spawnId`, `faction`, `flags`) VALUES
(14600140, 0, 48),
(14600141, 0, 48),
(14600142, 0, 48);

DELETE FROM `spawn_group` WHERE `spawnType`=1 AND `spawnId` BETWEEN 14600140 AND 14600142;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14613, 1, 14600140),
(14613, 1, 14600141),
(14613, 1, 14600142);

-- Wowhead has three Anchoring Crystals around each spire, about 15 yards out, and in a video of the retail
-- scenario they circle it. The three that stood where the spires now stand move out onto that circle and six
-- more join them. Their script makes them circle the spire at the distance and height they are spawned at, in
-- place of the NullCreatureAI they had.
--
-- They float (CREATURE_STATIC_FLAG_FLOATING), a spire's three at one height, just clear of the highest ground
-- under their circle: the beach is too uneven for the core to lay a circle over the ground. Their real static
-- flags and the height they float at on retail are not known.
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_broken_shore_anchoring_crystal' WHERE `entry`=91704;
UPDATE `creature_template_difficulty` SET `StaticFlags1`=`StaticFlags1`|0x20000000 WHERE `Entry`=91704;

UPDATE `creature` SET `position_x`=527.5, `position_y`=2155.9, `position_z`=7.00, `orientation`=1.5708 WHERE `guid`=14600128;
UPDATE `creature` SET `position_x`=558.3, `position_y`=2127.0, `position_z`=7.60, `orientation`=2.2689 WHERE `guid`=14600129;
UPDATE `creature` SET `position_x`=571.6, `position_y`=2074.5, `position_z`=6.90, `orientation`=2.9671 WHERE `guid`=14600130;

DELETE FROM `creature` WHERE `guid` BETWEEN 14600131 AND 14600136;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600131, 91704, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 505.0, 2168.9, 7.00, 3.6652, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0),
(14600132, 91704, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 505.0, 2142.9, 7.00, 5.7596, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0),
(14600133, 91704, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 532.7, 2122.5, 7.60, 4.3633, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0),
(14600134, 91704, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 549.4, 2102.6, 7.60, 0.1745, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0),
(14600135, 91704, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 554.9, 2054.6, 6.90, 5.0615, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0),
(14600136, 91704, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 580.5, 2050.1, 6.90, 0.8727, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0);

DELETE FROM `spawn_group` WHERE `spawnType`=0 AND `spawnId` BETWEEN 14600131 AND 14600136;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14601, 0, 14600131),
(14601, 0, 14600132),
(14601, 0, 14600133),
(14601, 0, 14600134),
(14601, 0, 14600135),
(14601, 0, 14600136);
