-- The enders of the zone quests, at the turn-in points of their quest POIs on Krasus' Landing and over the Dalaran
-- flight path. The Tranquil Forest has no POI: its text puts Khadgar on Krasus' Landing, so he stands by the other one.
DELETE FROM `creature` WHERE `guid` BETWEEN 12200004 AND 12200008;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12200004, 86563, 1220, 0, 0, '0', 0, 0, 0, -1, 0, 0, -835, 4276, 746.25, 1.57, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Archmage Khadgar: Paradise Lost
(12200005, 91172, 1220, 0, 0, '0', 0, 0, 0, -1, 0, 0, -840, 4272, 746.25, 1.57, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Archmage Khadgar: The Tranquil Forest
(12200006, 96644, 1220, 0, 0, '0', 0, 0, 0, -1, 0, 0, -842, 4635, 749.35, 4.71, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Sky Admiral Rogers: Stormheim (Alliance)
(12200007, 96683, 1220, 0, 0, '0', 0, 0, 0, -1, 0, 0, -847, 4635, 749.35, 4.71, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Nathanos Blightcaller: Stormheim (Horde)
(12200008, 97666, 1220, 0, 0, '0', 0, 0, 0, -1, 0, 0, -828, 4294, 746.28, 3.14, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Warbrave Oro: The Lone Mountain
