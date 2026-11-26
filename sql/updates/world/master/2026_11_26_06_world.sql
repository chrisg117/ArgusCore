-- Eye of Azshara trash: the abilities DBM 7.3.31's EoATrash.lua warns for (template rule T1).
-- Casters from LittleWigs' era trash module (addon:littlewigs@2018-07-10, Legion/EyeOfAzshara/Trash.lua):
--   Storm 196870: Hatecoil Stormweaver 91783 and Mystic Ssa'veh 98173
--   Rejuvenating Waters 195046: Hatecoil Oracle 95861
--   Spray Sand 196127: Mak'rana Siltwalker 91790
--   Undertow 195284: Restless Tides 97173
-- The trash Arcane Bomb (192706) has no caster in any source at hand: left unscripted.
-- Timers have no DBM bar: first cast and repeat are calibrate values (Storm's repeat is its 18 s category cooldown).

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (91783, 98173, 95861, 91790, 97173);

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (91783, 98173, 95861, 91790, 97173) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `Difficulties`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param_string`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `action_param7`, `action_param_string`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_param_string`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(91783, 0, 0, 0, '', 0, 0, 100, 0, 6000, 6000, 18000, 18000, 0, '', 11, 196870, 0, 0, 0, 0, 0, 0, '', 1, 0, 0, 0, 0, '', 0, 0, 0, 0, 'Hatecoil Stormweaver - In combat - Cast Storm'),
(98173, 0, 0, 0, '', 0, 0, 100, 0, 6000, 6000, 18000, 18000, 0, '', 11, 196870, 0, 0, 0, 0, 0, 0, '', 1, 0, 0, 0, 0, '', 0, 0, 0, 0, 'Mystic Ssa''veh - In combat - Cast Storm'),
(95861, 0, 0, 0, '', 0, 0, 100, 0, 8000, 8000, 20000, 20000, 0, '', 11, 195046, 0, 0, 0, 0, 0, 0, '', 1, 0, 0, 0, 0, '', 0, 0, 0, 0, 'Hatecoil Oracle - In combat - Cast Rejuvenating Waters'),
(91790, 0, 0, 0, '', 0, 0, 100, 0, 5000, 5000, 15000, 15000, 0, '', 11, 196127, 0, 0, 0, 0, 0, 0, '', 1, 0, 0, 0, 0, '', 0, 0, 0, 0, 'Mak''rana Siltwalker - In combat - Cast Spray Sand'),
(97173, 0, 0, 0, '', 0, 0, 100, 0, 8000, 8000, 20000, 20000, 0, '', 11, 195284, 0, 0, 0, 0, 0, 0, '', 1, 0, 0, 0, 0, '', 0, 0, 0, 0, 'Restless Tides - In combat - Cast Undertow');

-- Rejuvenating Waters' pool 5149: sphere, radius 5 (calibrate; the client has no shape).
-- Undertow's push (area trigger 5172 through 195288) is left out of the first pass; its damage ticks are data.
DELETE FROM `areatrigger_template` WHERE `Id` = 5149 AND `IsCustom` = 0;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(5149, 0, 0, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` = 5149 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(5149, 0, 5149, 0, 0, 0, 5, 5, 'at_eoa_rejuvenating_waters', 0);
