-- "Blade in Twilight" (scenario 991), in which a shadow priest gets Xal'atath: how a priest comes into it, and
-- its first two stages, "The Twilight Camp" and "Raiding the Tomb Raiders".
--
-- The scenario's map (1539) is a copy of western Tirisfal Glades with the same coordinates. The database had
-- nothing on it. What stands where in the camp is not known from any data: the three notes are where the
-- Warcraft Wiki has them, put on the wagon and the crates that the map has at those places, and the cultists
-- are placed by eye among the camp's tents.

-- The map has three scenarios, of which the `scenarios` table could name one. Its instance script gives an
-- instance the scenario of the quest the first player to enter is on.
DELETE FROM `instance_template` WHERE `map`=1539;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1539, 0, 'instance_tirisfal_artifact_scenarios');

-- A priest on the quest who comes near the camp at Tyr's Fall on Eastern Kingdoms is taken into the scenario.
-- The client has no area trigger there, so it is one of the server's own: a sphere of 120 yards around the
-- camp.
DELETE FROM `areatrigger_template` WHERE `Id`=40710 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(40710, 1, 1, 0, 0, 0);

DELETE FROM `areatrigger_create_properties` WHERE `Id`=40710 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(40710, 1, 40710, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 120, 120, 0, 0, 0, 0, '', 0);

DELETE FROM `areatrigger` WHERE `SpawnId`=1539001;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1539001, 40710, 1, 0, '0', 2135, 2420, 119, 0, 0, 0, 0, 'areatrigger_blade_in_twilight_entrance', 'Tirisfal Glades - Tyr''s Fall - into the scenario Blade in Twilight', 0);

-- The cultists of the camp are the Twilight's Hammer's and hostile like its others. Their templates had the
-- placeholder level 1 and a faction that is everyone's friend. They are of the level of the player who looks
-- at them, from 98 to 110, like the creatures of Legion's other scenarios: without the scaling levels each
-- would be of a level of its own between the two.
UPDATE `creature_template` SET `faction`=16, `ScriptName`='npc_twilight_shadowcaster' WHERE `entry`=101122;
UPDATE `creature_template` SET `faction`=16, `ScriptName`='npc_twilight_bladetwister' WHERE `entry`=101127;
UPDATE `creature_template` SET `faction`=16 WHERE `entry`=103692;
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `DifficultyID`=0 AND `Entry` IN (101122, 101127, 103692);

-- In the video of the retail scenario the Shadowcasters carry staves and the Bladetwisters a blade in each
-- hand. Which items they are is not known; these are what other creatures of the Twilight's Hammer carry
-- (the staff of the Twilight Darkcaller, the daggers of a Twilight Cultist).
DELETE FROM `creature_equip_template` WHERE `CreatureID` IN (101122, 101127) AND `ID`=1;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(101122, 1, 65102, 0, 0, 0, 0, 0, 0, 0, 0, 0), -- Twilight Shadowcaster
(101127, 1, 56067, 0, 0, 56067, 0, 0, 0, 0, 0, 0); -- Twilight Bladetwister

DELETE FROM `creature` WHERE `guid` BETWEEN 15390001 AND 15390012;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15390001, 101127, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2110, 2428, 119.3, 0.9, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Bladetwister, by the wagon with the first note
(15390002, 101127, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2118, 2418, 119.1, 5.5, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Bladetwister, by the southern tent
(15390003, 101127, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2140, 2400, 118.9, 4.7, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Bladetwister, at the camp's side to the lake
(15390004, 101127, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2150, 2442, 120.1, 3.9, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Bladetwister, between the western tents
(15390005, 101127, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2160, 2416, 119.5, 2.4, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Bladetwister, in the north of the camp
(15390006, 101127, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2124, 2438, 119.5, 4.2, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Bladetwister, before the tent with the table
(15390007, 101122, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2130, 2427, 119.3, 5.07, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Shadowcaster, at the brazier
(15390008, 101122, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2135, 2419, 119.3, 2.26, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Shadowcaster, at the brazier
(15390009, 101122, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2171, 2428, 119.6, 3.6, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Shadowcaster, at the smelting pot
(15390010, 101122, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2164, 2411, 119.4, 1.2, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Shadowcaster, by the tent with the third note
(15390011, 101122, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2106, 2400, 119.0, 1.6, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Shadowcaster, at the southern tent
(15390012, 103692, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 2143, 2422, 119.8, 4.5, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Greater Void Ravener, in the middle of the camp

-- Slaghammer's notes, the three clues of "The Twilight Camp"
DELETE FROM `gameobject` WHERE `guid` BETWEEN 15390001 AND 15390003;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15390001, 247332, 1539, 0, 0, '12', 0, 0, 0, -1, 2100.7, 2425.1, 119.9, 1.3, 0, 0, 0.605186, 0.796084, 7200, 255, 1, '', NULL, 0), -- Slaghammer's Notes: The New Leader, in the wagon at the camp's southern end
(15390002, 247334, 1539, 0, 0, '12', 0, 0, 0, -1, 2166.9, 2438.1, 121.0, 4.0, 0, 0, 0.909297, -0.416147, 7200, 255, 1, '', NULL, 0), -- Slaghammer's Notes: Tyr's Fall, on a crate in the north-western tent
(15390003, 247335, 1539, 0, 0, '12', 0, 0, 0, -1, 2169.3, 2404.6, 120.7, 5.2, 0, 0, 0.515501, -0.856889, 7200, 255, 1, '', NULL, 0); -- Slaghammer's Notes: Xal'atath, the Blade of the Black Empire, on the larger crate in the north-east
