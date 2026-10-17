-- Broken Shore scenario (map 1460), stage "Destroy the Portal", Alliance side.
-- There is no sniff of the scenario. The positions come from Wowhead map pins converted to map 1460 like the
-- spawns of the earlier stages, with heights from the extracted terrain, so they are approximate:
-- the four Shielded Anchors stand in two pairs either side of the portal site, an Eredar Chaos Guard stands
-- beside each on the side of Varian's camp, and a Mo'arg Painbringer stands at the edge of the camp.
-- A second Mo'arg Painbringer with pins here (101632) is left out: unlike every other demon of the scenario
-- it is typed as a critter, and it has no faction or level data, so it is taken to be a scripted actor.

-- A Shielded Anchor is a structure: it does not move or fight back
UPDATE `creature_template` SET `AIName`='NullCreatureAI' WHERE `entry`=101667;

-- The group is spawned by the instance script when the stage starts and despawned when it ends
DELETE FROM `spawn_group_template` WHERE `groupId`=14604;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14604, 'Broken Shore Scenario - Destroy the Portal (Alliance)', 4);

DELETE FROM `creature` WHERE `guid` BETWEEN 14600400 AND 14600408;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600400, 101667, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1205.50, 2471.60, 39.70, 0, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shielded Anchor
(14600401, 101667, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1215.80, 2467.80, 42.56, 0, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shielded Anchor
(14600402, 101667, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1200.40, 2429.30, 39.50, 0, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shielded Anchor
(14600403, 101667, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1226.10, 2425.40, 44.11, 0, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shielded Anchor
(14600404, 90525, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1201.50, 2471.60, 39.11, 2.9400, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Eredar Chaos Guard
(14600405, 90525, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1211.80, 2467.80, 41.23, 2.9400, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Eredar Chaos Guard
(14600406, 90525, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1196.40, 2429.30, 38.44, 2.4800, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Eredar Chaos Guard
(14600407, 90525, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1222.10, 2425.40, 43.56, 2.4800, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Eredar Chaos Guard
(14600408, 92564, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1137.10, 2469.00, 35.22, 2.2215, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- Mo'arg Painbringer

DELETE FROM `spawn_group` WHERE `groupId`=14604;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14604, 0, 14600400),
(14604, 0, 14600401),
(14604, 0, 14600402),
(14604, 0, 14600403),
(14604, 0, 14600404),
(14604, 0, 14600405),
(14604, 0, 14600406),
(14604, 0, 14600407),
(14604, 0, 14600408);
