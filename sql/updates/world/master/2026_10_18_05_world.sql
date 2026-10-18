-- Il'gynoth outside phase: the adds (hostile, scripted) and the Nightmare Corruption pool.

-- (The Corruptor script is named npc_ilgynoth_corruptor_tentacle: Ulduar already has npc_corruptor_tentacle.)
-- The add templates were unsniffed: faction 35 (friendly), no script. The Ichor (105721) only
-- gets its faction here; its ScriptName comes with its AI.
UPDATE `creature_template` SET `faction` = 14, `ScriptName` = 'npc_dominator_tentacle' WHERE `entry` = 105304;
UPDATE `creature_template` SET `faction` = 14, `ScriptName` = 'npc_deathglare_tentacle' WHERE `entry` = 105322;
UPDATE `creature_template` SET `faction` = 14, `ScriptName` = 'npc_ilgynoth_corruptor_tentacle' WHERE `entry` = 105383;
UPDATE `creature_template` SET `faction` = 14, `ScriptName` = 'npc_nightmare_horror' WHERE `entry` = 105591;
UPDATE `creature_template` SET `faction` = 14 WHERE `entry` = 105721;

-- Spew Corruption's periodic dummy drops the pools
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_ilgynoth_spew_corruption';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(208929, 'spell_ilgynoth_spew_corruption');

-- Nightmare Corruption: spell 208931 creates area trigger 6514, which the client data does not
-- have. Sphere of radius 5 and Nythendra's green decal 31 are first guesses, to be calibrated.
DELETE FROM `areatrigger_template` WHERE `Id` = 6514 AND `IsCustom` = 0;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(6514, 0, 0, 0, 0, 0);

DELETE FROM `areatrigger_create_properties` WHERE `Id` = 6514 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `DecalPropertiesId`, `ScriptName`, `VerifiedBuild`) VALUES
(6514, 0, 6514, 0, 0, 0, 5, 5, 31, 'at_ilgynoth_nightmare_corruption', 0);
