-- Azsuna slice 4a: Trailing the Tidestone (37497), Nar'thalas Still Suffers (37486), The Walk of Shame (37467),
-- Into the Academy (37468). Spawn positions from the quest POIs and Wowhead's pins (Azsuna map 630:
-- x = 1760.7 - 43.0 * pin_y, y = 9821.5 - 65.69 * pin_x); heights from the navmesh, checked in the client.

-- Givers: Farondis at the gate starts the walk, Farondis at the end of the second walk sends the player into the academy.
-- Each quest only after the one before it: the rows would otherwise offer it to anyone who sees the NPC.
DELETE FROM `creature_queststarter` WHERE (`id`, `quest`) IN ((88115, 37497), (88890, 37468));
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(88115, 37497, 0),
(88890, 37468, 0);

INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(37497, 37257),
(37486, 37497),
(37467, 37486),
(37468, 37467)
ON DUPLICATE KEY UPDATE `PrevQuestID` = VALUES(`PrevQuestID`);

-- The static Farondis at the gate and at the ruins summon the walking one for the player who accepts the walk (they
-- phase out as it starts: phases 4270 and 4290, as sniffed); the walking Farondis are personal summons
UPDATE `creature_template` SET `ScriptName` = 'npc_farondis_tidestone_giver' WHERE `entry` IN (88115, 88867);
UPDATE `creature_template` SET `ScriptName` = 'npc_farondis_escort' WHERE `entry` IN (88869, 88889);

-- 1220001: the gate (-11,6734) down the road to the Ruins of Nar'thalas, past Wowhead's two pins of 88869
-- 1220002: the ruins (-140,6419) around the edge of town to the Drowned Gardens' shore (-40,6261), past 88889's pins
DELETE FROM `waypoint_path` WHERE `PathId` IN (1220001, 1220002);
INSERT INTO `waypoint_path` (`PathId`, `MoveType`, `Flags`, `Velocity`, `Comment`) VALUES
(1220001, 0, 0, NULL, 'Prince Farondis 88869 - Trailing the Tidestone: gate to the Ruins of Nar''thalas'),
(1220002, 0, 0, NULL, 'Prince Farondis 88889 - The Walk of Shame: the ruins around the edge of town');

DELETE FROM `waypoint_path_node` WHERE `PathId` IN (1220001, 1220002);
INSERT INTO `waypoint_path_node` (`PathId`, `NodeId`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `Delay`) VALUES
(1220001,  0,  -10.8, 6723.5, 56.1, NULL, 0),
(1220001,  1,  -15.0, 6715.0, 54.0, NULL, 0),
(1220001,  2,  -20.0, 6700.0, 46.3, NULL, 0),
(1220001,  3,  -23.2, 6680.0, 38.1, NULL, 0),
(1220001,  4,  -24.9, 6657.2, 31.6, NULL, 0),
(1220001,  5,  -24.7, 6630.0, 27.2, NULL, 0),
(1220001,  6,  -37.0, 6616.0, 28.4, NULL, 0),
(1220001,  7,  -56.3, 6599.9, 32.2, NULL, 0),
(1220001,  8,  -81.9, 6584.0, 32.6, NULL, 0),
(1220001,  9,  -93.6, 6556.0, 31.6, NULL, 0),
(1220001, 10, -115.7, 6532.7, 29.3, NULL, 0),
(1220001, 11, -122.8, 6511.7, 28.8, NULL, 0),
(1220001, 12, -127.7, 6485.3, 33.6, NULL, 0),
(1220001, 13, -131.6, 6464.0, 30.2, NULL, 0),
(1220001, 14, -135.3, 6440.7, 27.7, NULL, 0),
(1220001, 15, -137.5, 6426.0, 28.5, NULL, 0),
(1220002,  0, -130.0, 6412.0, 24.9, NULL, 0),
(1220002,  1, -118.0, 6400.0,  7.0, NULL, 0),
(1220002,  2, -101.0, 6392.0,  6.9, NULL, 0),
(1220002,  3,  -80.4, 6390.8,  2.5, NULL, 0),
(1220002,  4,  -74.7, 6357.3,  2.2, NULL, 0),
(1220002,  5,  -76.1, 6330.0,  1.7, NULL, 0),
(1220002,  6,  -80.0, 6307.0,  2.6, NULL, 0),
(1220002,  7,  -78.4, 6283.2,  0.1, NULL, 0),
(1220002,  8,  -75.6, 6259.2,  3.6, NULL, 0),
(1220002,  9,  -50.0, 6259.0,  7.4, NULL, 0);

-- Spawns of slice 4a: 12204151 to 12204199
DELETE FROM `creature` WHERE `guid` BETWEEN 12204151 AND 12204199;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
-- Athissa in the Drowned Gardens (37486 POI -323,6159), two raiders and an enchantress from the shore camp beside her
(12204151, 88091, 1220, 7334, 0, '0', 0, 0, 0, 0, -323.0, 6159.0, 41.4, 0.60, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204152, 88084, 1220, 7334, 0, '0', 0, 0, 0, 0, -315.0, 6168.0, 41.4, 3.90, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204153, 88084, 1220, 7334, 0, '0', 0, 0, 0, 0, -331.0, 6166.0, 41.4, 5.50, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204154, 88089, 1220, 7334, 0, '0', 0, 0, 0, 0, -318.0, 6150.0, 41.4, 2.20, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
-- Farondis where the second walk ends (37467 turn-in POI -40,6261)
(12204155, 88890, 1220, 7334, 0, '0', 0, 0, 0, 0,  -40.0, 6261.0,  8.75, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
-- Thyrillion, Academy Curator, in the entrance hall (37468 turn-in POI 184,6341)
(12204156, 89660, 1220, 7334, 0, '0', 0, 0, 0, 0,  184.0, 6341.0, -30.2, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0);

-- Who sees them
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (88091, 88890, 89660);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 88091, 0, 0, 47, 0, 37486,  8, 0, '', 0, 0, 0, '', 'Athissa is visible while quest 37486 (Nar''thalas Still Suffers) is in progress'),
(32, 3, 88890, 0, 0, 47, 0, 37467, 66, 0, '', 0, 0, 0, '', 'Prince Farondis (Drowned Gardens) is visible once quest 37467 (The Walk of Shame) is complete or rewarded'),
(32, 3, 88890, 0, 0, 47, 0, 37468, 64, 0, '', 1, 0, 0, '', 'Prince Farondis (Drowned Gardens) is visible until quest 37468 (Into the Academy) is rewarded'),
(32, 3, 89660, 0, 0, 47, 0, 37468, 74, 0, '', 0, 0, 0, '', 'Thyrillion is visible once quest 37468 (Into the Academy) is taken');
