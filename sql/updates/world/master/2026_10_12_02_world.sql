-- After playing the priest's return to Dalaran: the teleport pads can be seen, "Blade in Twilight" has places
-- on the map, and "The Light and the Void" goes on from the portal to its end, the founding of the Conclave
-- in Netherlight Temple.

-- The two teleport pads worked but were not to be seen: the city's model has only the floor they lie on. They
-- are the Kirin Tor's pad (display 27762, 6kt_kirintor_teleportpad01) with a column of light over it (display
-- 16972), as in the video. The game has two sets of "Teleport Pad" templates with these displays, of which
-- the first is taken for the pad in the middle of the city and the second for the one in Aegwynn's Gallery;
-- where the retail game has them is not known. Each set has a third object (display 9064, a battleground's
-- teleporter), which is left out: nothing like it is in the video.
DELETE FROM `gameobject` WHERE `guid` BETWEEN 12200002 AND 12200005;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12200002, 244534, 1220, 0, 0, '0', 0, 0, 0, -1, -844.65, 4467.65, 735.88, 0, 0, 0, 0, 1, 300, 255, 1, '', NULL, 0), -- Teleport Pad, in the middle of the city
(12200003, 244537, 1220, 0, 0, '0', 0, 0, 0, -1, -844.65, 4467.65, 735.88, 0, 0, 0, 0, 1, 300, 255, 1, '', NULL, 0), -- its light
(12200004, 244560, 1220, 0, 0, '0', 0, 0, 0, -1, -780, 4415.2, 602.63, 0, 0, 0, 0, 1, 300, 255, 1, '', NULL, 0), -- Teleport Pad, in Aegwynn's Gallery
(12200005, 244562, 1220, 0, 0, '0', 0, 0, 0, -1, -780, 4415.2, 602.63, 0, 0, 0, 0, 1, 300, 255, 1, '', NULL, 0); -- its light

-- "Blade in Twilight" had no places on the map. Its first objective asks for "the marked location in Tirisfal
-- Glades", which is the camp at Tyr's Fall; its last is the three at the Antonidas Memorial, where it is also
-- handed in. The second is inside the scenario and has none.
DELETE FROM `quest_poi` WHERE `QuestID`=40710;
INSERT INTO `quest_poi` (`QuestID`, `BlobIndex`, `Idx1`, `ObjectiveIndex`, `QuestObjectiveID`, `QuestObjectID`, `MapID`, `WorldMapAreaId`, `Floor`, `Priority`, `Flags`, `WorldEffectID`, `PlayerConditionID`, `SpawnTrackingID`, `AlwaysAllowMergingBlobs`, `VerifiedBuild`) VALUES
(40710, 0, 0, -1, 0, 0, 1220, 1014, 10, 0, 0, 0, 0, 0, 0, 0),
(40710, 0, 1, 0, 281106, 101365, 0, 20, 0, 0, 2, 0, 0, 0, 0, 0),
(40710, 0, 2, 2, 281356, 102285, 1220, 1014, 10, 0, 2, 0, 0, 0, 0, 0);

DELETE FROM `quest_poi_points` WHERE `QuestID`=40710;
INSERT INTO `quest_poi_points` (`QuestID`, `Idx1`, `Idx2`, `X`, `Y`, `VerifiedBuild`) VALUES
(40710, 0, 0, -701, 4494, 0),
(40710, 1, 0, 2135, 2420, 0),
(40710, 2, 0, -699, 4492, 0);

-- The founding of the Conclave. The portal's spell takes the player to the temple while it is Saraka's prison
-- (map 1629). What happens there is the client's scene 1191 with the script package "Naaru Reborn" (1556),
-- which "Priest Order Formation" (202457) plays; it cannot be left by the escape key (flags as most of the
-- quest scenes that the database has from the retail game). See zone_orderhall_priest.cpp.
DELETE FROM `scene_template` WHERE `SceneId`=1191;
INSERT INTO `scene_template` (`SceneId`, `Flags`, `ScriptPackageID`, `ScriptName`) VALUES
(1191, 20, 1556, 'scene_priest_order_formation');

-- The scene begins where the portal puts one down, and in the temple as it is afterwards (map 1512) Faol
-- calls to who arrives there
DELETE FROM `areatrigger_template` WHERE `Id` IN (105027, 202457) AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(105027, 1, 1, 0, 0, 0),
(202457, 1, 1, 0, 0, 0);

DELETE FROM `areatrigger_create_properties` WHERE `Id` IN (105027, 202457) AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(105027, 1, 105027, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 12, 12, 0, 0, 0, 0, '', 0),
(202457, 1, 202457, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 6, 6, 0, 0, 0, 0, '', 0);

DELETE FROM `areatrigger` WHERE `SpawnId` IN (1512001, 1629001);
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1629001, 202457, 1, 1629, '12', 1249.25, 1344.14, 185.02, 0, 0, 0, 0, 'at_netherlight_temple_prison_order_formation', 'Netherlight Temple (prison) - where the portal arrives - the scene of the Conclave''s founding begins (Quest: "The Light and the Void")', 0),
(1512001, 105027, 1, 1512, '0', 1247.93, 1344.84, 185.02, 0, 0, 0, 0, 'at_netherlight_temple_conclave_founded', 'Netherlight Temple - where one arrives - Faol calls (Quest: "The Light and the Void")', 0);

-- The void god that the player's "Dark Drain" (202747) is cast at belongs to the scene. The scene leaves one
-- kind of real creature to be seen, the Effects Bunny (102528), so that is what stands in the void god's place
-- (the scene's point 16137) for the spell to find. It has no body of its own; it is put at the height of the
-- void god's middle rather than at its feet. "Faol's Life Grip" (202982) looks for a creature in the same way.
DELETE FROM `creature` WHERE `guid`=16290001;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(16290001, 102528, 1629, 0, 0, '12', 0, 0, 0, -1, 11686, 0, 1417.68, 1343.95, 190, 3.14, 300, 0, 0, 100, 0, NULL, 33555200, NULL, NULL, '', NULL, 0); -- Effects Bunny, in the void god's place

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (202747, 202982);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 202747, 0, 0, 31, 0, 3, 102528, 0, '', 0, 0, 0, '', 'Dark Drain - is cast at an Effects Bunny'),
(13, 1, 202982, 0, 0, 31, 0, 3, 102528, 0, '', 0, 0, 0, '', 'Faol''s Life Grip - is cast at an Effects Bunny');

DELETE FROM `spell_script_names` WHERE `spell_id`=202747;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(202747, 'spell_orderhall_priest_dark_drain');

-- Where two of the quest's spells take the player is not in the client's data. "Faol's Life Grip" pulls to
-- Faol, who stands at the end of the scene's path 16239 before the dais: three yards short of him. "Teleport:
-- Post Order Formation" takes into the temple as it is afterwards, to where the spells of its portals arrive.
DELETE FROM `spell_target_position` WHERE `ID` IN (202491, 202982);
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(202491, 0, 0, 1512, 1247.93, 1344.84, 185, 0, 0),
(202982, 1, 0, 1629, 1381, 1344.2, 176.74, 0, 0);

-- Alonsus Faol at the main circle of the temple as it is afterwards, where the quest is handed in to him
-- (the place the quest's map data has for it), and what he says then (from the Warcraft Wiki; the database
-- had no text). The rest of the temple is not there yet.
DELETE FROM `creature` WHERE `guid`=15120001;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15120001, 102655, 1512, 0, 0, '0', 0, 0, 0, -1, 0, 0, 1332, 1349, 177.14, 3.2, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Alonsus Faol

DELETE FROM `creature_text` WHERE `CreatureID`=102655;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(102655, 0, 0, 'Over here! I think you will approve of the changes we''ve made!', 14, 0, 100, 0, 0, 63425, 0, 105027, 0, 'Alonsus Faol - a priest arrives in the temple after the Conclave''s founding');

DELETE FROM `quest_offer_reward` WHERE `ID`=40938;
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(40938, 0, 0, 0, 0, 0, 0, 0, 0, 'Welcome back, Cardinal. I think you''ll find we''ve done quite a job getting this place prepared for you. We should get started immediately.', 0);

DELETE FROM `quest_request_items` WHERE `ID`=40938;
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(40938, 0, 0, 0, 0, 'We have important work to do, $n.', 0);
