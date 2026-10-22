-- Into the Foul Creche (38725) ends at Kor'vas Bloodthorn (98711), who was never spawned, so the quest could not be
-- turned in. Position from his Wowhead map pin (Mardum 73.2, 33.8) through the same fit used for Beliash; z from the
-- map files. He is only visible once the player has taken the quest.
DELETE FROM `creature` WHERE `guid` = 14810002;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14810002, 98711, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1733.8, 1287.8, 84.4, 2.45, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 98711;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 98711, 0, 0, 47, 0, 38725, 74, 0, '', 0, 0, 0, '', 'Kor''vas Bloodthorn is visible if quest 38725 (Into the Foul Creche) is in progress, complete or rewarded');

-- He also gives The Imp Mother's Tome (40222), which had no giver; it follows Into the Foul Creche (Wowhead series)
DELETE FROM `creature_queststarter` WHERE `id` = 98711 AND `quest` = 40222;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(98711, 40222, 0);

DELETE FROM `quest_template_addon` WHERE `ID` = 40222;
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(40222, 38725);
