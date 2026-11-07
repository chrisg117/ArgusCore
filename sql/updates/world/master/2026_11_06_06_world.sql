-- Acherus, the hall opening (Task 4): Illanna Dreadmoore gives Keeping Your Edge.
--
-- Fork data: no creature_queststarter row for 39757 exists in the data, and the data names no giver. Its start POI
-- (objective 32, spawn tracking 1053061, -1523, 1054, floor 2) is the one Illanna's 43877 uses, and the user remembers
-- her ("Archivist Dreadmoore") giving it on retail. She stands at the user's ping on the lowest deck, facing Mograine.
-- Every death knight spec's artifact quest (38990, 40740, 40935) ends at Mograine with 218548 Order Formation, which
-- rewards tracking quest 39017; she offers 39757 to a death knight once that is done.
-- Illanna has no gossip menu or text in the data, so none is added.

DELETE FROM `creature` WHERE `guid`=12207002;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12207002, 97111, 1220, 0, 0, '0', 0, 0, 0, 0, -1520.18, 1042.34, 260.46, 0.40, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Illanna Dreadmoore

DELETE FROM `creature_queststarter` WHERE `id`=97111 AND `quest`=39757;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(97111, 39757, 0); -- Illanna Dreadmoore: Keeping Your Edge

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry`=39757;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 39757, 0, 0, 15, 0, 32, 0, 0, 0, 'Keeping Your Edge: death knights only'),
(19, 0, 39757, 0, 0, 47, 0, 39017, 64, 0, 0, 'Keeping Your Edge: Order Formation done (any spec''s artifact quest turned in)');
