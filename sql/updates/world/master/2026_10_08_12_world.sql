-- Broken Shore scenario (map 1460), Alliance side: the forces that land with Jaina and Genn and fight
-- alongside players on the beach. Wowhead has Kirin Tor Battle-Mages, Gilnean Royal Guards, Gilnean Druids
-- and Darnassus Sentinels in the scenario, the first three all over the beach.
--
-- Their templates still had the placeholder faction 35 and level 1. They get the faction of the leaders
-- they follow, which the Stormwind Knights and Gnomeregan Tinkerers of the same scenario already have, and
-- the scenario's level range, which is that of their level scaling.
UPDATE `creature_template` SET `faction`=2879, `ScriptName`='npc_broken_shore_alliance_soldier' WHERE `entry` IN (97486, 114466);
UPDATE `creature_template` SET `faction`=2879, `ScriptName`='npc_broken_shore_alliance_caster' WHERE `entry` IN (91353, 97496, 110627);
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry` IN (91353, 97486, 97496, 110627, 114466);

-- They are a spawn group of their own, spawned by the instance script when "Storm the Beach" starts, because
-- they stay when that stage ends. How many there are and where they stand is a choice: twelve of them behind
-- the leaders, between them and the water. Retail lands far more. One that falls is back after 30 seconds, in
-- place of the reinforcements that should keep landing.
DELETE FROM `spawn_group_template` WHERE `groupId`=14612;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14612, 'Broken Shore Scenario - Alliance Landing Force', 4);

DELETE FROM `creature` WHERE `guid` BETWEEN 14601200 AND 14601211;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14601200, 91353, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 453.00, 2090.00, 0.25, 0.2842, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Kirin Tor Battle-Mage
(14601201, 97496, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 452.50, 2096.00, 0.62, 0.2232, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Kirin Tor Battle-Mage
(14601202, 91353, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 448.50, 2087.00, 0.25, 0.2999, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Kirin Tor Battle-Mage
(14601203, 97496, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 448.50, 2093.00, 0.25, 0.2433, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Kirin Tor Battle-Mage
(14601204, 114466, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 452.50, 2079.00, 0.25, 0.3867, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Darnassus Sentinel
(14601205, 114466, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 452.50, 2073.50, 0.25, 0.4357, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Darnassus Sentinel
(14601206, 110627, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 448.00, 2081.50, 0.25, 0.3485, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Gilnean Druid
(14601207, 110627, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 448.00, 2070.50, 0.25, 0.4432, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Gilnean Druid
(14601208, 97486, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 453.50, 2063.00, 0.25, 0.5279, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Gilnean Royal Guard
(14601209, 97486, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 455.00, 2057.00, 0.79, 0.5819, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Gilnean Royal Guard
(14601210, 97486, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 450.50, 2067.00, 0.25, 0.4822, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Gilnean Royal Guard
(14601211, 97486, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 459.50, 2061.50, 0.24, 0.5695, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- Gilnean Royal Guard

DELETE FROM `spawn_group` WHERE `groupId`=14612;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14612, 0, 14601200),
(14612, 0, 14601201),
(14612, 0, 14601202),
(14612, 0, 14601203),
(14612, 0, 14601204),
(14612, 0, 14601205),
(14612, 0, 14601206),
(14612, 0, 14601207),
(14612, 0, 14601208),
(14612, 0, 14601209),
(14612, 0, 14601210),
(14612, 0, 14601211);
