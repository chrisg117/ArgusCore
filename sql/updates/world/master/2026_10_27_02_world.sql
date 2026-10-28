-- The Alliance demon hunter's arrival in Stormwind, after Illidari, We Are Leaving: The Call of War (39691), Second
-- Sight (44471), Demons Among Them (44463) and A Weapon of the Alliance (44473) had no one to give them and nothing to do
-- them with. As in the Wowhead guides and the retail videos:
-- - Teleport to Stormwind lands the demon hunter outside Stormwind Keep, before Archmage Khadgar (Wowhead 72.6, 47.0);
--   the spell's own facing points at the throne room. This replaces the landing before Anduin from 2026_10_27_01.
--   Khadgar gives The Call of War there and is gone once it is turned in ("I am needed in Dalaran").
-- - Entering the throne room counts (at_the_call_of_war_throne_room); Jace Darkweaver (84.4, 33.7) ends it.
-- - Second Sight: Jace gives it; using Spectral Sight in the Keep counts (player_second_sight).
-- - Demons Among Them: Jace gives it. The player warns Anduin through his gossip with the retail option text, which casts
--   Speak with Jace Credit and shows the player five Felblade Assassins spread through the Keep (Wowhead: "located
--   throughout Stormwind Keep"). The one in the throne room turns on the player with the retail yell; the others turn on
--   them up close (npc_demons_among_them_anduin, npc_demons_among_them_felblade_assassin). Anduin ends it.
-- - A Weapon of the Alliance: Anduin gives it; Elerion Bladedancer ends it at the Illidari camp in the Mage Quarter
--   (40.3, 77.8).
-- Anduin is the quest version (100973) while the player is between Second Sight and A Weapon of the Alliance, in place of
-- the throne's usual Anduin (107574). The funeral of Varian that retail stages in the throne room (the bier, the Alliance
-- leaders, Malfurion's line) is not built, and the Felblade Assassins are not disguised as guards before Spectral Sight.
-- Positions are converted from the Wowhead coordinates with the client's Stormwind map bounds; heights from the guards
-- standing there.

-- Lands outside the Keep, facing the throne room
DELETE FROM `spell_target_position` WHERE `ID` = 192757;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(192757, 0, 0, 0, -8544.20, 467.10, 104.80, NULL, 0); -- Teleport to Stormwind: outside Stormwind Keep, before Khadgar

DELETE FROM `creature_queststarter` WHERE `quest` IN (39691, 44471, 44463, 44473);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(97296, 39691, 0),
(102585, 44471, 0),
(102585, 44463, 0),
(100973, 44473, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (39691, 44471, 44463, 44473);
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(39691, 39689),
(44471, 39691),
(44463, 44471),
(44473, 44463);

UPDATE `creature_template` SET `npcflag` = 3, `ScriptName` = 'npc_demons_among_them_anduin' WHERE `entry` = 100973;
UPDATE `creature_template` SET `ScriptName` = 'npc_demons_among_them_felblade_assassin' WHERE `entry` = 100993;

DELETE FROM `creature_text` WHERE `CreatureID` = 100993;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(100993, 0, 0, 'You discovered us too late, Illidari! FELBLADE, ATTACK!', 14, 0, 100, 0, 0, 0, 0, 120161, 0, 'Felblade Assassin - Anduin warned');

-- Who sees whom along the way
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (97296, 102585, 100973, 107574, 100993, 101004);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 97296, 0, 0, 47, 0, 39689, 64, 0, '', 0, 0, 0, '', 'Khadgar (Stormwind Keep): arrived from the Vault'),
(32, 3, 97296, 0, 0, 47, 0, 39691, 64, 0, '', 1, 0, 0, '', 'Khadgar (Stormwind Keep): until The Call of War is turned in'),
(32, 3, 102585, 0, 0, 47, 0, 39691, 74, 0, '', 0, 0, 0, '', 'Jace (throne room): from The Call of War on'),
(32, 3, 102585, 0, 0, 47, 0, 44463, 64, 0, '', 1, 0, 0, '', 'Jace (throne room): until Demons Among Them is turned in'),
(32, 3, 100973, 0, 0, 47, 0, 44471, 64, 0, '', 0, 0, 0, '', 'Anduin (quest version): from Second Sight on'),
(32, 3, 100973, 0, 0, 47, 0, 44473, 64, 0, '', 1, 0, 0, '', 'Anduin (quest version): until A Weapon of the Alliance is turned in'),
(32, 3, 107574, 0, 0, 47, 0, 44471, 64, 0, '', 1, 0, 0, '', 'Anduin (usual): not while the quest version is there'),
(32, 3, 107574, 0, 1, 47, 0, 44473, 64, 0, '', 0, 0, 0, '', 'Anduin (usual): back once A Weapon of the Alliance is turned in'),
(32, 3, 100993, 0, 0, 48, 0, 286650, 0, 1, '', 0, 0, 0, '', 'Felblade Assassin (throne room): once Anduin is warned'),
(32, 3, 101004, 0, 0, 47, 0, 44463, 64, 0, '', 0, 0, 0, '', 'Elerion (Mage Quarter): from Demons Among Them on');

DELETE FROM `creature` WHERE `guid` BETWEEN 15190001 AND 15190009;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15190001, 97296, 0, 1519, 0, '0', 0, 0, 0, -1, 0, 0, -8540.00, 461.50, 104.46, 2.2100, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Archmage Khadgar, outside Stormwind Keep
(15190002, 102585, 0, 1519, 0, '0', 0, 0, 0, -1, 0, 0, -8384.50, 258.50, 155.43, 2.2100, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Jace Darkweaver, throne room
(15190003, 100973, 0, 1519, 0, '0', 0, 0, 0, -1, 0, 0, -8363.29, 232.55, 157.07, 2.2500, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Anduin Wrynn, throne
(15190004, 100993, 0, 1519, 0, '0', 0, 0, 0, -1, 0, 0, -8376.00, 247.00, 155.53, 3.9000, 30, 0, 0, 100, 0, NULL, 512, NULL, NULL, '', NULL, 0), -- Felblade Assassin, throne room
(15190005, 100993, 0, 1519, 0, '0', 0, 0, 0, -1, 0, 0, -8410.00, 207.00, 155.53, 0.8000, 30, 0, 0, 100, 0, NULL, 512, NULL, NULL, '', NULL, 0), -- Felblade Assassin, west wing
(15190006, 100993, 0, 1519, 0, '0', 0, 0, 0, -1, 0, 0, -8303.00, 242.00, 155.53, 3.5000, 30, 0, 0, 100, 0, NULL, 512, NULL, NULL, '', NULL, 0), -- Felblade Assassin, east wing
(15190007, 100993, 0, 1519, 0, '0', 0, 0, 0, -1, 0, 0, -8398.00, 316.00, 147.41, 5.5000, 30, 0, 0, 100, 0, NULL, 512, NULL, NULL, '', NULL, 0), -- Felblade Assassin, entrance hall
(15190008, 100993, 0, 1519, 0, '0', 0, 0, 0, -1, 0, 0, -8420.00, 310.00, 146.80, 0.7000, 30, 0, 0, 100, 0, NULL, 512, NULL, NULL, '', NULL, 0), -- Felblade Assassin, entrance hall
(15190009, 101004, 0, 1519, 0, '0', 0, 0, 0, -1, 0, 0, -8897.00, 1022.70, 124.50, 2.4000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Elerion Bladedancer, Mage Quarter

-- Entering the throne room (server-side)
DELETE FROM `areatrigger_template` WHERE `Id` = 1519001 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1519001, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` = 1519001 AND `IsCustom` = 1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(1519001, 1, 1519001, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 18, 18, 0, 0, 0, 0, 'at_the_call_of_war_throne_room', 0);
DELETE FROM `areatrigger` WHERE `SpawnId` = 1519001;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1519001, 1519001, 1, 0, '0', -8390.00, 262.00, 157.50, 0, 0, 0, 0, '', 'Stormwind Keep - throne room (Quest: "The Call of War" - Enter the Throne Room)', 0);
