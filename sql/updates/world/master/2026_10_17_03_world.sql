-- Nythendra phase one: Rot, Volatile Rot, Infested Breath and Infested Ground.

-- Class 4 so her bar is energy (class 1 creatures display rage). Warrior and
-- rogue creatures have the same base stats at level 110.
UPDATE `creature_template` SET `unit_class` = 4 WHERE `entry` = 102672;

-- Infested Ground: spell 203044 creates area trigger 5887, which the world
-- database did not have. The radius is a first guess, to be calibrated.
DELETE FROM `areatrigger_template` WHERE `Id` = 5887 AND `IsCustom` = 0;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(5887, 0, 0, 0, 0, 0);

DELETE FROM `areatrigger_create_properties` WHERE `Id` = 5887 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(5887, 0, 5887, 0, 0, 0, 5, 5, 'at_nythendra_infested_ground', 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_nythendra_infested_breath', 'spell_nythendra_rot', 'spell_nythendra_volatile_rot');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(202977, 'spell_nythendra_infested_breath'),
(203096, 'spell_nythendra_rot'),
(204463, 'spell_nythendra_volatile_rot');
