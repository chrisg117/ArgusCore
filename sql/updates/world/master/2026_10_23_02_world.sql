-- On Felbat Wings (39663) could not be started or done: Kayn Sunfury (93127) did not give it, and neither Izal
-- Whitemoon (96653, who hands out the felbat) nor Cyana Nightglaive (97297, who ends it on the Fel Hammer) was spawned.
-- Izal's sniffed gossip option makes the player cast 192136 "On Felbat Wings: Unlearned Taxi Illidari Foothold --> The
-- Fel Hammer", which flies taxi path 5286 up to the Fel Hammer and gives the credit (96653).

-- Kayn offers it once the Illidari Foothold's three quest lines are done (Wowhead series: Stop the Bombardment, Their
-- Numbers Are Legion and either spec quest)
DELETE FROM `creature_queststarter` WHERE `id` = 93127 AND `quest` = 39663;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(93127, 39663, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` = 39663;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 39663, 0, 0, 8, 0, 38727, 0, 0, '', 0, 0, 0, '', 'On Felbat Wings needs Stop the Bombardment'),
(19, 0, 39663, 0, 0, 8, 0, 38819, 0, 0, '', 0, 0, 0, '', 'On Felbat Wings needs Their Numbers Are Legion'),
(19, 0, 39663, 0, 0, 8, 0, 39516, 0, 0, '', 0, 0, 0, '', 'On Felbat Wings needs Cry Havoc and Let Slip the Illidari!'),
(19, 0, 39663, 0, 1, 8, 0, 38727, 0, 0, '', 0, 0, 0, '', 'On Felbat Wings needs Stop the Bombardment'),
(19, 0, 39663, 0, 1, 8, 0, 38819, 0, 0, '', 0, 0, 0, '', 'On Felbat Wings needs Their Numbers Are Legion'),
(19, 0, 39663, 0, 1, 8, 0, 39515, 0, 0, '', 0, 0, 0, '', 'On Felbat Wings needs Vengeance Will Be Mine!');

-- Izal at the start of taxi path 5286 (his Wowhead pin, on the Foothold's landing), facing the Foothold; Cyana on the
-- Fel Hammer's deck where the path ends (her pin), facing the landing felbat. Heights from the navmesh.
DELETE FROM `creature` WHERE `guid` IN (14810007, 14810008);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14810007, 96653, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1456.2, 1692.5, 47.63, 1.571, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14810008, 97297, 1481, 7705, 0, '0', 0, 0, 0, -1, 0, 0, 1471.3, 1415.2, 243.86, 3.734, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0);

-- Izal's sniffed menu (18776: "I have captured a few felbats...") and its option, shown while the flight is still to do
DELETE FROM `creature_template_gossip` WHERE `CreatureID` = 96653;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(96653, 18776, 0); -- Izal Whitemoon

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 18776 AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 18776, 0, 0, 0, 47, 0, 39663, 8, 0, '', 0, 0, 0, '', 'Izal Whitemoon: felbat while On Felbat Wings is in progress'),
(15, 18776, 0, 0, 0, 48, 0, 279956, 0, 1, '', 1, 0, 0, '', 'Izal Whitemoon: not yet flown to the Fel Hammer');

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 96653;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 96653;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `Difficulties`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param_string`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `action_param7`, `action_param_string`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_param_string`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(96653, 0, 0, 0, '', 62, 0, 100, 0, 18776, 0, 0, 0, 0, '', 72, 0, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Izal Whitemoon - On gossip option 0 selected - Close gossip'),
(96653, 0, 1, 0, '', 62, 0, 100, 0, 18776, 0, 0, 0, 0, '', 85, 192136, 2, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Izal Whitemoon - On gossip option 0 selected - Invoker casts On Felbat Wings: Unlearned Taxi Illidari Foothold --> The Fel Hammer');
