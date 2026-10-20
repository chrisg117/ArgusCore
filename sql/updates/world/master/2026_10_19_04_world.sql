-- The Emerald Nightmare hub portals (map 1520): four walk-in portals above Nythendra's
-- room, one per wing, and a return trigger inside each wing.
-- The hub triggers carry the script at_en_hub_portal (open once Nythendra is dead, or for
-- a GM); the return triggers use the built-in teleport action (ActionType 2) and are
-- ungated. Wing order everywhere: Un'goro, Thunder Bluff, Grizzly Hills, Emerald Dreamway.
-- Positions are placeholders from the client's WorldSafeLocs (hub portals 8 yards outside
-- the ring spots 5677-5680, return triggers 5 yards behind the wing arrival points) and
-- are corrected with .gps in game. Which arrival point belongs to which wing is only
-- verified for Un'goro (5461).

-- Templates 152003-152006 hub, 152007-152010 return
DELETE FROM `areatrigger_template` WHERE `Id` BETWEEN 152003 AND 152010 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(152003, 1, 1, 0, 10, 0),
(152004, 1, 1, 0, 10, 0),
(152005, 1, 1, 0, 10, 0),
(152006, 1, 1, 0, 10, 0),
(152007, 1, 1, 0, 10, 0),
(152008, 1, 1, 0, 10, 0),
(152009, 1, 1, 0, 10, 0),
(152010, 1, 1, 0, 10, 0);

-- Return triggers teleport to the hub ring spots 5677-5680; the hub triggers have no
-- action, their script does the teleport
DELETE FROM `areatrigger_template_actions` WHERE `AreaTriggerId` BETWEEN 152003 AND 152010 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template_actions` (`AreaTriggerId`, `IsCustom`, `ActionType`, `ActionParam`, `TargetType`) VALUES
(152007, 1, 2, 5677, 5),
(152008, 1, 2, 5678, 5),
(152009, 1, 2, 5679, 5),
(152010, 1, 2, 5680, 5);

-- Spheres of radius 4
DELETE FROM `areatrigger_create_properties` WHERE `Id` BETWEEN 152003 AND 152010 AND `IsCustom` = 1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(152003, 1, 152003, 1, 0, 0, 4, 4, 'at_en_hub_portal', 0),
(152004, 1, 152004, 1, 0, 0, 4, 4, 'at_en_hub_portal', 0),
(152005, 1, 152005, 1, 0, 0, 4, 4, 'at_en_hub_portal', 0),
(152006, 1, 152006, 1, 0, 0, 4, 4, 'at_en_hub_portal', 0),
(152007, 1, 152007, 1, 0, 0, 4, 4, '', 0),
(152008, 1, 152008, 1, 0, 0, 4, 4, '', 0),
(152009, 1, 152009, 1, 0, 0, 4, 4, '', 0),
(152010, 1, 152010, 1, 0, 0, 4, 4, '', 0);

DELETE FROM `areatrigger` WHERE `SpawnId` BETWEEN 1520201 AND 1520208;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1520201, 152003, 1, 1520, '14,15,16,17', 1824.56, 1325.0, 597.5, 0, 0, 0, 0, '', 'The Emerald Nightmare - Hub portal to Un''goro Crater', 0),
(1520202, 152004, 1, 1520, '14,15,16,17', 1850.88, 1272.96, 598.5, 0, 0, 0, 0, '', 'The Emerald Nightmare - Hub portal to Thunder Bluff', 0),
(1520203, 152005, 1, 1520, '14,15,16,17', 1910.28, 1297.39, 601.9, 0, 0, 0, 0, '', 'The Emerald Nightmare - Hub portal to Grizzly Hills', 0),
(1520204, 152006, 1, 1520, '14,15,16,17', 1956.0, 1350.21, 581.0, 0, 0, 0, 0, '', 'The Emerald Nightmare - Hub portal to The Emerald Dreamway', 0),
(1520205, 152007, 1, 1520, '14,15,16,17', -13152.79, 12095.48, -256.3, 0, 0, 0, 0, '', 'The Emerald Nightmare - Return from Un''goro Crater', 0),
(1520206, 152008, 1, 1520, '14,15,16,17', 456.87, -12752.69, 27.1, 0, 0, 0, 0, '', 'The Emerald Nightmare - Return from Thunder Bluff', 0),
(1520207, 152009, 1, 1520, '14,15,16,17', -12332.35, -12756.89, 329.4, 0, 0, 0, 0, '', 'The Emerald Nightmare - Return from Grizzly Hills', 0),
(1520208, 152010, 1, 1520, '14,15,16,17', 11332.71, 11050.69, 13.7, 0, 0, 0, 0, '', 'The Emerald Nightmare - Return from The Emerald Dreamway', 0);

-- The portal objects (visual only), under the hub triggers and facing the ring centre
DELETE FROM `gameobject` WHERE `guid` BETWEEN 1520201 AND 1520204;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(1520201, 254270, 1520, 0, 0, '14,15,16,17', 0, 0, 0, -1, 1824.56, 1325.0, 597.5, 0.4956, 0, 0, 0.245274, 0.969454, 7200, 255, 1, '', NULL, 0),
(1520202, 254271, 1520, 0, 0, '14,15,16,17', 0, 0, 0, -1, 1850.88, 1272.96, 598.5, 1.3157, 0, 0, 0.611406, 0.791317, 7200, 255, 1, '', NULL, 0),
(1520203, 254272, 1520, 0, 0, '14,15,16,17', 0, 0, 0, -1, 1910.28, 1297.39, 601.9, 2.2112, 0, 0, 0.893729, 0.448607, 7200, 255, 1, '', NULL, 0),
(1520204, 254273, 1520, 0, 0, '14,15,16,17', 0, 0, 0, -1, 1956.0, 1350.21, 581.0, 3.1429, 0, 0, 1, -0.000649, 7200, 255, 1, '', NULL, 0);
