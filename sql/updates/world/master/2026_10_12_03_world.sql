-- The founding of the Conclave, after playing it: the main circle was not lit while the ritual waited for the
-- player's spell. The scene plays that light on a bunny of its own, which was not to be seen in play, so the
-- server plays the same on a real Effects Bunny in the middle of the circle (the scene's point 16138); see
-- zone_orderhall_priest.cpp, which finds it by its StringId.
DELETE FROM `creature` WHERE `guid`=16290002;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(16290002, 102528, 1629, 0, 0, '12', 0, 0, 0, -1, 11686, 0, 1335.03, 1344.22, 177.14, 0, 300, 0, 0, 100, 0, NULL, 33555200, NULL, NULL, '', 'PriestOrderFormationRitualSite', 0); -- Effects Bunny, in the middle of the main circle

-- With a second bunny on the map the two spells that look for one have to find the one in the void god's
-- place, not the nearest
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (202747, 202982);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 202747, 0, 0, 31, 0, 3, 102528, 16290001, '', 0, 0, 0, '', 'Dark Drain - is cast at the Effects Bunny in the void god''s place'),
(13, 1, 202982, 0, 0, 31, 0, 3, 102528, 16290001, '', 0, 0, 0, '', 'Faol''s Life Grip - is cast at the Effects Bunny in the void god''s place');
