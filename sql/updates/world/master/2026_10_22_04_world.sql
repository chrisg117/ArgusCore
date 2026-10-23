-- The Imp Mother's Tome (40222): Prolifica had no loot, so The Imp Mother's Tome (129957) never dropped
UPDATE `creature_template_difficulty` SET `LootID` = 98986 WHERE `Entry` = 98986 AND `DifficultyID` = 0;
DELETE FROM `creature_loot_template` WHERE `Entry` = 98986 AND `Item` = 129957;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(98986, 0, 129957, 100, 1, 1, 0, 1, 1, 'Prolifica - The Imp Mother''s Tome');

-- The quest ends at Kor'vas Bloodthorn 99045, who was never spawned. The quest's turn-in POI is where Kor'vas 98711
-- (who gives it) stands, so 99045 takes his place once the tome quest is complete.
DELETE FROM `creature` WHERE `guid` = 14810005;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14810005, 99045, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1733.8, 1287.8, 84.4, 2.45, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 99045;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 98711 AND `ConditionValue1` = 40222;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 98711, 0, 0, 47, 0, 40222, 66, 0, '', 1, 0, 0, '', 'Kor''vas Bloodthorn (98711) is gone once The Imp Mother''s Tome is complete'),
(32, 3, 99045, 0, 0, 47, 0, 40222, 66, 0, '', 0, 0, 0, '', 'Kor''vas Bloodthorn (99045) is there once The Imp Mother''s Tome is complete');
