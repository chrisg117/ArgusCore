-- Prolifica (98986), from a retail video of The Imp Mother's Tome: she is hidden in her fel pool until a player comes
-- near, rises out of it yelling, and fights from above it. Her open tome lies on the floor in front of the pool and is
-- gone during the fight.
--
-- The client does not draw her while her origin is below the cave floor, so her spawn (2026_10_22_02) sits below the
-- pool and npc_prolifica keeps her unattackable there, then flies her up into view. CREATURE_STATIC_FLAG_FLOATING keeps
-- gravity off: without it the core takes a creature below the floor for one that fell through it and teleports it up.
-- In the video she hovers over her pool for the whole fight; the script roots her there once she has risen.
DELETE FROM `creature_static_flags_override` WHERE `SpawnId` = 14811144;
INSERT INTO `creature_static_flags_override` (`SpawnId`, `DifficultyId`, `StaticFlags1`) VALUES
(14811144, 0, 0x20000000);

-- AnimTier Fly: with gravity off she floats, but on the ground tier the client gives her no stand or attack animations
-- in the air (her melee landed without a swing)
DELETE FROM `creature_template_addon` WHERE `entry` = 98986;
INSERT INTO `creature_template_addon` (`entry`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(98986, 0, 0, 0, 0, 3, 0, 1, 0, 0, 0, 0, 0, 0, '');

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_prolifica' WHERE `entry` = 98986;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 98986 AND `source_type` = 0;

DELETE FROM `creature_text` WHERE `CreatureID` = 98986;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(98986, 0, 0, 'Get away. You cannot have my tome!', 14, 0, 100, 0, 0, 55338, 0, 100840, 0, 'Prolifica - rises from her pool'),
(98986, 1, 0, 'Recover that tome at all costs, my minions!', 14, 0, 100, 0, 0, 55066, 0, 100842, 0, 'Prolifica - first Gaseous Breath, which spawns her imps');

-- Her fight (Wowhead abilities; timers are a guess): Gaseous Breath (197240) hits where it lands, leaves a Gaseous
-- Cloud and spawns Lesser Minions (100286) there. The cloud, area trigger 5319 from 197245, had no data: while inside it
-- her enemies get Gaseous Cloud (197243). The shape (a sphere of radius 6) is a guess; the 15 s lifetime is the
-- spell's. 1481002 is a custom template id (map id + n). Fel Geyser is scripted in npc_prolifica.
DELETE FROM `areatrigger_template` WHERE `Id` = 1481002 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1481002, 1, 0, 0, 0, 0);

DELETE FROM `areatrigger_template_actions` WHERE `AreaTriggerId` = 1481002 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template_actions` (`AreaTriggerId`, `IsCustom`, `ActionType`, `ActionParam`, `TargetType`) VALUES
(1481002, 1, 1, 197243, 2); -- add aura Gaseous Cloud to enemies while inside

DELETE FROM `areatrigger_create_properties` WHERE `Id` = 5319 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `AnimId`, `Speed`, `DecalPropertiesId`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(5319, 0, 1481002, 1, 0, -1, 1, 0, 0, 6, 6, '', 0);

-- The tome: 245169 (7LG_Legion_Book01 at double scale, the Tome of Fel Secrets model), never spawned, lying open on
-- the floor in front of the pool where the user placed it in game (.gps)
DELETE FROM `gameobject` WHERE `guid` = 14810004;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14810004, 245169, 1481, 7705, 0, '0', 0, 0, 0, -1, 1864.70, 1149.16, 79.6, 5.7271, 0, 0, -0.274376, 0.961623, 120, 255, 1, '', NULL, 0);
