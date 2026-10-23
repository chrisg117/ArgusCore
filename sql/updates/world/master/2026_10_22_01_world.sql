-- Stop the Bombardment (38727) was not scripted: the Illidari Banners were never spawned and their use spells
-- (191480, 191519, 191520) are serverside and missing, and two of the three Legion Devastators had no spawn.
-- go_stop_the_bombardment_illidari_banner now gives the banner credit, sends the captain's troops at the
-- devastator and gives its credit; a devastator is hidden from a player once its objective is done.
-- Banners from their Wowhead pins (site per the Wowhead guide). The Doom Fortress
-- and Forge of Corruption devastators stand where the user placed them in game (.gps). Soul Engine uses the
-- existing phase-0 devastator (guid 6000552).
DELETE FROM `gameobject` WHERE `guid` IN (14810001, 14810002, 14810003);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14810001, 243965, 1481, 7705, 0, '0', 0, 0, 0, -1, 1352.7, 1433.9, 37.95, 1.2666, 0, 0, 0.591808, 0.806079, 120, 255, 1, '', NULL, 0), -- Doom Fortress
(14810002, 243967, 1481, 7705, 0, '0', 0, 0, 0, -1, 1552.1, 1216.6, 73.9, 1.7449, 0, 0, 0.765906, 0.642952, 120, 255, 1, '', NULL, 0), -- Forge of Corruption
(14810003, 243968, 1481, 7705, 0, '0', 0, 0, 0, -1, 1824.7, 1540.8, 87.09, 2.5992, 0, 0, 0.963451, 0.267884, 120, 255, 1, '', NULL, 0); -- Soul Engine

UPDATE `gameobject_template` SET `ScriptName` = 'go_stop_the_bombardment_illidari_banner' WHERE `entry` IN (243965, 243967, 243968);

-- The banners are there while the quest is in the log
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 5 AND `SourceEntry` IN (243965, 243967, 243968);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 5, 243965, 0, 0, 47, 0, 38727, 10, 0, '', 0, 0, 0, '', 'Illidari Banner (Doom Fortress) is visible while quest 38727 is in progress or complete'),
(32, 5, 243967, 0, 0, 47, 0, 38727, 10, 0, '', 0, 0, 0, '', 'Illidari Banner (Forge of Corruption) is visible while quest 38727 is in progress or complete'),
(32, 5, 243968, 0, 0, 47, 0, 38727, 10, 0, '', 0, 0, 0, '', 'Illidari Banner (Soul Engine) is visible while quest 38727 is in progress or complete');

DELETE FROM `creature` WHERE `guid` IN (14810003, 14810004);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14810003, 96732, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1380.09, 1450.79, 36.6, 1.3323, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14810004, 96731, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1533.84, 1241.26, 71.05, 1.7189, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0);

-- A devastator is gone for a player once its objective is done or the quest is rewarded (TypeID 3 = unit)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (93762, 96732, 96731);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 93762, 0, 0, 48, 0, 279857, 0, 1, '', 1, 0, 0, '', 'Legion Devastator (Soul Engine) is visible if its objective is not done'),
(32, 3, 93762, 0, 0, 47, 0, 38727, 64, 0, '', 1, 0, 0, '', 'Legion Devastator (Soul Engine) is visible if quest 38727 is not rewarded'),
(32, 3, 96732, 0, 0, 48, 0, 279862, 0, 1, '', 1, 0, 0, '', 'Legion Devastator (Doom Fortress) is visible if its objective is not done'),
(32, 3, 96732, 0, 0, 47, 0, 38727, 64, 0, '', 1, 0, 0, '', 'Legion Devastator (Doom Fortress) is visible if quest 38727 is not rewarded'),
(32, 3, 96731, 0, 0, 48, 0, 279864, 0, 1, '', 1, 0, 0, '', 'Legion Devastator (Forge of Corruption) is visible if its objective is not done'),
(32, 3, 96731, 0, 0, 47, 0, 38727, 64, 0, '', 1, 0, 0, '', 'Legion Devastator (Forge of Corruption) is visible if quest 38727 is not rewarded');
