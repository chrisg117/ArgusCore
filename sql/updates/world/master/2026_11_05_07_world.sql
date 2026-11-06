-- DK campaign: The Call of Icecrown, scenario 901 step 1810, the Spire re-placed from the user's eight .gps pings on
-- pass 4 (2026-11-05 23:52 to 00:03, read from GM.log). The user marked areas, not exact spots. The route: the gate,
-- Baelgun, a knight pair, Halahk, the souls, Magroth, a knight with souls, Gavinrad, then the teleporter (ping 8,
-- 4357.08, 2770.32, 356.05, kept for 3c). Each soul faces the way the player arrives and its fragment sits 4 yd behind it.
-- Chest rotations come from the facing at full precision (the old 0.9108 / 0.4128 pair failed the unit check)

-- Ping 1: Baelgun Flamebeard and his fragment
UPDATE `creature` SET `position_x`=4260.78, `position_y`=2768.81, `position_z`=349.38, `orientation`=3.14159 WHERE `guid`=14800001;
UPDATE `gameobject` SET `position_x`=4264.78, `position_y`=2768.81, `position_z`=349.38, `orientation`=3.14159, `rotation2`=1, `rotation3`=0 WHERE `guid`=14800001;

-- Ping 3: Halahk the Deathbringer and his fragment, facing the knights' floor to the west
UPDATE `creature` SET `position_x`=4356.78, `position_y`=2674.54, `position_z`=349.34, `orientation`=2.76 WHERE `guid`=14800002;
UPDATE `gameobject` SET `position_x`=4360.49, `position_y`=2673.05, `position_z`=349.34, `orientation`=2.76, `rotation2`=0.981854, `rotation3`=0.189641 WHERE `guid`=14800002;

-- Ping 5: Magroth the Defender and his fragment, facing the souls to the south-west
UPDATE `creature` SET `position_x`=4452.57, `position_y`=2769.71, `position_z`=349.35, `orientation`=4.33 WHERE `guid`=14800003;
UPDATE `gameobject` SET `position_x`=4454.06, `position_y`=2773.42, `position_z`=349.35, `orientation`=4.33, `rotation2`=0.828595, `rotation3`=-0.559849 WHERE `guid`=14800003;

-- Ping 7: Gavinrad the Cruel and his fragment on the walkway, facing east toward the walkway's knight
UPDATE `creature` SET `position_x`=4384.44, `position_y`=2769.61, `position_z`=357.66, `orientation`=0 WHERE `guid`=14800004;
UPDATE `gameobject` SET `position_x`=4380.44, `position_y`=2769.61, `position_z`=357.66, `orientation`=0, `rotation2`=0, `rotation3`=1 WHERE `guid`=14800004;

-- Ping 2: two Spectral Knights, each on its own short patrol over the open floor (walked points), not linked to Halahk
UPDATE `creature` SET `position_x`=4286.00, `position_y`=2702.00, `position_z`=344.11, `orientation`=2.2, `MovementType`=2 WHERE `guid`=14800011;
UPDATE `creature` SET `position_x`=4291.00, `position_y`=2697.00, `position_z`=344.11, `orientation`=5.4, `MovementType`=2 WHERE `guid`=14800012;

-- Ping 4: Magroth's three Anguished Souls
UPDATE `creature` SET `position_x`=4422.00, `position_y`=2697.00, `position_z`=344.11, `orientation`=3.6 WHERE `guid`=14800013;
UPDATE `creature` SET `position_x`=4427.00, `position_y`=2701.00, `position_z`=344.11, `orientation`=3.6 WHERE `guid`=14800014;
UPDATE `creature` SET `position_x`=4425.00, `position_y`=2705.00, `position_z`=344.11, `orientation`=3.6 WHERE `guid`=14800015;

-- Ping 6: a Spectral Knight patrolling the walkway, with two Anguished Souls. The walkway is narrow, so the Swordsman
-- becomes a soul. Everything stays 25 yd or more from Gavinrad so the pull stays separate
UPDATE `creature` SET `position_x`=4410.00, `position_y`=2769.00, `position_z`=357.70, `orientation`=0, `MovementType`=2 WHERE `guid`=14800016;
UPDATE `creature` SET `id`=95433, `position_x`=4419.00, `position_y`=2768.00, `position_z`=356.45, `orientation`=3.1416 WHERE `guid`=14800017;
DELETE FROM `creature` WHERE `guid`=14800018;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14800018, 95433, 1480, 0, 0, '12', 0, 0, 0, 0, 4422.00, 2769.00, 356.00, 3.1416, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Anguished Soul, the walkway

-- The patrols (PathId = spawn guid)
DELETE FROM `waypoint_path` WHERE `PathId` IN (14800011, 14800012, 14800016);
INSERT INTO `waypoint_path` (`PathId`, `MoveType`, `Flags`, `Velocity`, `Comment`) VALUES
(14800011, 0, 0, NULL, 'Icecrown Citadel (DK Frost scenario) - Spectral Knight, the floor before Halahk'),
(14800012, 0, 0, NULL, 'Icecrown Citadel (DK Frost scenario) - Spectral Knight, the floor before Halahk'),
(14800016, 0, 0, NULL, 'Icecrown Citadel (DK Frost scenario) - Spectral Knight, the walkway before Gavinrad');
DELETE FROM `waypoint_path_node` WHERE `PathId` IN (14800011, 14800012, 14800016);
INSERT INTO `waypoint_path_node` (`PathId`, `NodeId`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `Delay`) VALUES
(14800011, 0, 4286.00, 2702.00, 344.11, NULL, 0),
(14800011, 1, 4281.00, 2711.00, 344.11, NULL, 3000),
(14800011, 2, 4278.00, 2715.00, 344.11, NULL, 0),
(14800011, 3, 4283.00, 2705.00, 344.11, NULL, 3000),
(14800012, 0, 4291.00, 2697.00, 344.11, NULL, 0),
(14800012, 1, 4295.00, 2693.00, 344.11, NULL, 4000),
(14800012, 2, 4297.00, 2698.00, 344.11, NULL, 0),
(14800012, 3, 4288.00, 2701.00, 344.11, NULL, 4000),
(14800016, 0, 4410.00, 2769.00, 357.70, NULL, 2000),
(14800016, 1, 4428.00, 2769.00, 354.70, NULL, 2000);
DELETE FROM `creature_addon` WHERE `guid` IN (14800011, 14800012, 14800016);
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(14800011, 14800011, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(14800012, 14800012, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(14800016, 14800016, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL);
