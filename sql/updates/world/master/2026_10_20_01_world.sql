-- Elerethe Renferal Spider form: Necrotic Venom scripts and the Venomous Pool area trigger.

-- 215443 filters the targets; 215449 (the 10 s mark) starts the damage debuff when it expires.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_en_necrotic_venom', 'spell_en_necrotic_venom_mark');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(215443, 'spell_en_necrotic_venom'),
(215449, 'spell_en_necrotic_venom_mark');

-- Venomous Pool: spells 213123 (spiderling death) and 225519 (Necrotic Venom) create area trigger
-- 7078, which the client data does not have. Sphere of radius 3 and Nythendra's green decal 31.
DELETE FROM `areatrigger_template` WHERE `Id` = 7078 AND `IsCustom` = 0;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(7078, 0, 0, 0, 0, 0);

DELETE FROM `areatrigger_create_properties` WHERE `Id` = 7078 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `DecalPropertiesId`, `ScriptName`, `VerifiedBuild`) VALUES
(7078, 0, 7078, 0, 0, 0, 3, 3, 31, 'at_en_venomous_pool', 0);
