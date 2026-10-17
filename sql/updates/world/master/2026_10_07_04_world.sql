-- Broken Shore scenario (map 1460), stage "Storm the Beach", Alliance side.
-- There is no sniff of the scenario. The positions are Wowhead map pins for each creature, converted from
-- the Broken Shore zone map to map 1460 (WorldMapArea 1021 and WorldMapTransforms 69), with the height taken
-- from the extracted terrain. A pin is where a player stood, on a grid of about 5 x 8 yards, so these show
-- what was roughly where; they are not exact spawn points. Everything faces the Alliance landing point.
-- Spawn ids 146001xx and spawn group 14601 are picked well away from the ids the base database uses.

-- An Anchoring Crystal is a structure: it neither moves nor fights back
UPDATE `creature_template` SET `AIName`='NullCreatureAI' WHERE `entry`=91704;

-- The group is spawned by the instance script while the stage is in progress
DELETE FROM `spawn_group_template` WHERE `groupId`=14601;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14601, 'Broken Shore Scenario - Storm the Beach (Alliance)', 4);

-- The beach demons respawn after 30 seconds: the stage asks for 33 kills, more than were ever seen standing
-- there at once, and nothing reinforces them yet. Fel Lords and crystals do not come back during the stage.
DELETE FROM `creature` WHERE `guid` BETWEEN 14600100 AND 14600130;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600100, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 523.68, 2127.88, 3.67, 3.7163, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600101, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 506.49, 2125.73, 3.33, 3.8108, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600102, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 573.04, 2112.39, 7.52, 3.4151, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600103, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 531.13, 2100.56, 1.96, 3.4144, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600104, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 551.95, 2083.52, 3.44, 3.2098, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600105, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 518.06, 2088.67, 1.07, 3.3088, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600106, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 511.66, 2060.85, 0.83, 2.9202, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600107, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 549.27, 2056.15, 1.19, 2.9544, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600108, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 525.61, 2050.24, 0.83, 2.8352, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600109, 90686, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 484.79, 2046.33, 0.82, 2.5132, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felstalker Dreadhound
(14600110, 109591, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 547.56, 2099.41, 4.13, 3.3623, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600111, 109591, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 533.39, 2103.01, 2.28, 3.4330, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600112, 109591, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 517.81, 2086.89, 1.09, 3.2860, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600113, 109591, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 521.33, 2078.30, 0.88, 3.1695, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600114, 109591, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 502.21, 2079.11, 0.82, 3.1925, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600115, 109592, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 531.78, 2097.14, 2.06, 3.3760, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600116, 109592, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 549.54, 2084.59, 3.23, 3.2214, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600117, 109592, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 560.59, 2080.60, 4.47, 3.1799, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600118, 109592, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 505.02, 2065.90, 0.82, 2.9761, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600119, 109592, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 526.83, 2048.97, 0.83, 2.8255, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600120, 109592, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 490.81, 2047.71, 0.82, 2.5980, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600121, 109604, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 497.68, 2132.39, 3.92, 3.9484, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600122, 109604, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 520.62, 2126.98, 3.54, 3.7263, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600123, 109604, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 530.08, 2116.22, 2.82, 3.5764, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600124, 109604, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 515.53, 2114.29, 2.25, 3.6304, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Legionnaire
(14600125, 109586, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 506.40, 2122.00, 2.99, 3.7738, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fel Lord Rakkan
(14600126, 91588, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 528.00, 2088.00, 1.52, 3.2816, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fel Lord Kurduz
(14600127, 109587, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 530.50, 2050.00, 0.84, 2.8489, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fel Lord Zardak
(14600128, 91704, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 512.50, 2155.90, 4.89, 4.0013, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Anchoring Crystal
(14600129, 91704, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 546.80, 2117.40, 4.71, 3.5226, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Anchoring Crystal
(14600130, 91704, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 569.00, 2059.70, 3.71, 3.0111, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- Anchoring Crystal

DELETE FROM `spawn_group` WHERE `groupId`=14601;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14601, 0, 14600100),
(14601, 0, 14600101),
(14601, 0, 14600102),
(14601, 0, 14600103),
(14601, 0, 14600104),
(14601, 0, 14600105),
(14601, 0, 14600106),
(14601, 0, 14600107),
(14601, 0, 14600108),
(14601, 0, 14600109),
(14601, 0, 14600110),
(14601, 0, 14600111),
(14601, 0, 14600112),
(14601, 0, 14600113),
(14601, 0, 14600114),
(14601, 0, 14600115),
(14601, 0, 14600116),
(14601, 0, 14600117),
(14601, 0, 14600118),
(14601, 0, 14600119),
(14601, 0, 14600120),
(14601, 0, 14600121),
(14601, 0, 14600122),
(14601, 0, 14600123),
(14601, 0, 14600124),
(14601, 0, 14600125),
(14601, 0, 14600126),
(14601, 0, 14600127),
(14601, 0, 14600128),
(14601, 0, 14600129),
(14601, 0, 14600130);
