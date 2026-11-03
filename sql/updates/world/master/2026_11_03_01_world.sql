-- Retake Faronaar (36811): the two Legion Portals stood on the same spots every time. Eight spots across Faronaar now
-- form one pool with two portals up at a time (the user's pass, 2026-11-03).
DELETE FROM `gameobject` WHERE `guid` IN (12201200, 12201202, 12201204, 12201205, 12201206, 12201207);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(12201200, 239676, 1220, 7334, 0, '0', 0, 0, -619.90, 7238.70, 40.20, 1.28, 0, 0, 0.597195, 0.802096, 120, 255, 1, 0),
(12201202, 239676, 1220, 7334, 0, '0', 0, 0, -500.20, 7240.90, 17.91, 5.93, 0, 0, 0.175676, -0.984448, 120, 255, 1, 0),
(12201204, 239676, 1220, 7334, 0, '0', 0, 0, -501.20, 7361.60, 44.08, 3.09, 0, 0, 0.999667, 0.025793, 120, 255, 1, 0),
(12201205, 239676, 1220, 7334, 0, '0', 0, 0, -378.20, 7243.90, 22.22, 1.34, 0, 0, 0.620986, 0.783822, 120, 255, 1, 0),
(12201206, 239676, 1220, 7334, 0, '0', 0, 0, -349.00, 7243.90, 26.15, 1.51, 0, 0, 0.685289, 0.728272, 120, 255, 1, 0),
(12201207, 239676, 1220, 7334, 0, '0', 0, 0, -651.30, 7240.20, 26.11, 3.32, 0, 0, 0.996024, -0.089085, 120, 255, 1, 0);

DELETE FROM `pool_template` WHERE `entry`=12201200;
INSERT INTO `pool_template` (`entry`, `max_limit`, `description`) VALUES
(12201200, 2, 'Retake Faronaar - Legion Portals');

DELETE FROM `pool_members` WHERE `type`=1 AND `spawnId` BETWEEN 12201200 AND 12201207;
INSERT INTO `pool_members` (`type`, `spawnId`, `poolSpawnId`, `chance`, `description`) VALUES
(1, 12201200, 12201200, 0, 'Retake Faronaar - Legion Portal'),
(1, 12201201, 12201200, 0, 'Retake Faronaar - Legion Portal'),
(1, 12201202, 12201200, 0, 'Retake Faronaar - Legion Portal'),
(1, 12201203, 12201200, 0, 'Retake Faronaar - Legion Portal'),
(1, 12201204, 12201200, 0, 'Retake Faronaar - Legion Portal'),
(1, 12201205, 12201200, 0, 'Retake Faronaar - Legion Portal'),
(1, 12201206, 12201200, 0, 'Retake Faronaar - Legion Portal'),
(1, 12201207, 12201200, 0, 'Retake Faronaar - Legion Portal');
