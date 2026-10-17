-- The Emerald Nightmare (map 1520): walk-in entrance, exit and meeting stone.
-- The client has no trigger volume for the raid portal, so both triggers are
-- server-side static areatriggers using the built-in teleport action
-- (ActionType 2, ActionParam = WorldSafeLocs id). A static spawn uses the
-- player who walks in as the caster, so no script is needed. The pattern is
-- the one "The Seat of the Triumvirate - Enter" (template 110) uses.
-- Positions were taken in game on 2026-10-17 at the retail spots.

-- Templates: 152001 entrance (to 5533, inside), 152002 exit (to 5687, Val'sharah)
DELETE FROM `areatrigger_template` WHERE `Id` IN (152001, 152002) AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(152001, 1, 1, 0, 10, 0),
(152002, 1, 1, 0, 10, 0);

DELETE FROM `areatrigger_template_actions` WHERE `AreaTriggerId` IN (152001, 152002) AND `IsCustom` = 1;
INSERT INTO `areatrigger_template_actions` (`AreaTriggerId`, `IsCustom`, `ActionType`, `ActionParam`, `TargetType`) VALUES
(152001, 1, 2, 5533, 5),
(152002, 1, 2, 5687, 5);

-- Spheres of radius 4
DELETE FROM `areatrigger_create_properties` WHERE `Id` IN (152001, 152002) AND `IsCustom` = 1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(152001, 1, 152001, 1, 0, 0, 4, 4, '', 0),
(152002, 1, 152002, 1, 0, 0, 4, 4, '', 0);

-- Entrance at the foot of Shaladrassil, Val'sharah (34.7 yards from where
-- players leaving the raid land); exit 18.3 yards behind the arrival point inside.
DELETE FROM `areatrigger` WHERE `SpawnId` IN (1220005, 1520001);
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1220005, 152001, 1, 1220, '0', 3614.3474, 6506.3433, 183.47836, 0, 0, 0, 0, '', 'The Emerald Nightmare - Entrance', 0),
(1520001, 152002, 1, 1520, '14,15,16,17', 1791.7775, 1424.3619, 348.34763, 0, 0, 0, 0, '', 'The Emerald Nightmare - Exit', 0);

-- Emerald Nightmare Meeting Stone (252516), beside the entrance (spot and facing set in game)
DELETE FROM `gameobject` WHERE `guid` = 12200007;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12200007, 252516, 1220, 7558, 8179, '0', 0, 0, 0, -1, 3576.19, 6548.6963, 180.90729, 0.8049, 0, 0, 0.391674, 0.920104, 120, 255, 1, '', NULL, 0);
