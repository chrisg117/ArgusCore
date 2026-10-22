-- Doom Commander Beliash: Shadow Retreat's trail. 195402 creates area trigger 5177, which had no data. While inside it,
-- his enemies get Shadow Blaze (195401). The shape (a sphere of radius 3) is a guess; the 20 s lifetime is the spell's.
-- 1481001 is a custom template id (map id + n).
DELETE FROM `areatrigger_template` WHERE `Id` = 1481001 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1481001, 1, 0, 0, 0, 0);

DELETE FROM `areatrigger_template_actions` WHERE `AreaTriggerId` = 1481001 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template_actions` (`AreaTriggerId`, `IsCustom`, `ActionType`, `ActionParam`, `TargetType`) VALUES
(1481001, 1, 1, 195401, 2); -- add aura Shadow Blaze to enemies while inside

DELETE FROM `areatrigger_create_properties` WHERE `Id` = 5177 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `AnimId`, `Speed`, `DecalPropertiesId`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(5177, 0, 1481001, 1, 0, -1, 1, 0, 0, 3, 3, '', 0);
