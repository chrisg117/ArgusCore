-- What closes the shadow priest's way to an artifact: the temple as it is after the founding (map 1512) can
-- be left and come back to, and the weapon is made an artifact of at its forge, the Altar of Light and Shadow,
-- by Faol's "Artifacts Need Artificers" and Betild Deepanvil's "Empowering Your Artifact". The rest of the
-- temple (its other people, missions, research) is not there.

-- The way out: the temple's "Portal to Dalaran" (251677, which casts "Teleport: Dalaran" 218172). It stands
-- in the alcove behind where one arrives; players' positions at it on Wowhead are just before that alcove.
-- Where its spell takes the player is not in the client's data: Krasus' Landing, as the other spells of this
-- name that the database has a place for.
DELETE FROM `gameobject` WHERE `guid` IN (12200006, 15120001, 15120002);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15120001, 251677, 1512, 0, 0, '0', 0, 0, 0, -1, 1230, 1344.2, 185.71, 0, 0, 0, 0, 1, 300, 255, 1, '', NULL, 0), -- Portal to Dalaran
-- The way back: the Alliance's "Portal to Netherlight Temple" (251306, "Teleport to Priest Order Hall" 215790,
-- which has its place) in the Greyfang Enclave, on the upper floor between the two priests' banners that the
-- city's model has there. The Horde's (251307, in Windrunner's Sanctuary) is not spawned.
(12200006, 251306, 1220, 0, 0, '0', 0, 0, 0, -1, -898.1, 4554.9, 744.62, 4.12, 0, 0, 0.882701, -0.469936, 300, 255, 1, '', NULL, 0), -- Portal to Netherlight Temple
-- The forge, on the plate between the two tables at the front of the dais; which way it faces is a guess
(15120002, 246320, 1512, 0, 0, '0', 0, 0, 0, -1, 1412.4, 1344.2, 184.18, 0, 0, 0, 0, 1, 300, 255, 1, '', NULL, 0); -- Altar of Light and Shadow

DELETE FROM `spell_target_position` WHERE `ID`=218172;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(218172, 1, 0, 1220, -828.72, 4371.78, 738.64, NULL, 0);

-- The portal in Dalaran is a priest's alone by its template; it is there only for one whose Conclave is founded
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=5 AND `SourceEntry`=251306;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 5, 251306, 0, 0, 8, 0, 40938, 0, 0, '', 0, 0, 0, '', 'Portal to Netherlight Temple - is seen once "The Light and the Void" is rewarded');

-- Betild Deepanvil beside the altar, where players are found speaking with her on Wowhead, and Saa'ra, whom
-- the temple's main circle has above it (the same source; how high is from the video). A naaru floats.
DELETE FROM `creature` WHERE `guid` IN (15120002, 15120003);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15120002, 102709, 1512, 0, 0, '0', 0, 0, 0, -1, 0, 0, 1415.9, 1347.2, 184.19, 0.3, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Betild Deepanvil
(15120003, 111241, 1512, 0, 0, '0', 0, 0, 0, -1, 0, 0, 1335.5, 1344, 187, 3.14, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Saa'ra

UPDATE `creature_template_difficulty` SET `StaticFlags1`=`StaticFlags1`|0x20000000 WHERE `Entry`=111241 AND `DifficultyID`=0;

-- The forge gives the weapon what its first trait costs; see zone_orderhall_priest.cpp
UPDATE `gameobject_template` SET `ScriptName`='go_netherlight_temple_altar_of_light_and_shadow' WHERE `entry`=246320;

-- The two quests: who gives them, whose they are and what they follow, and what Betild says when they are
-- handed in to her (from the Warcraft Wiki; the database had no text)
DELETE FROM `creature_queststarter` WHERE (`id`=102655 AND `quest`=41015) OR (`id`=102709 AND `quest`=41017);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(102655, 41015, 0),
(102709, 41017, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (41015, 41017);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(41015, 16, 40938),
(41017, 16, 41015);

DELETE FROM `quest_offer_reward` WHERE `ID` IN (41015, 41017);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(41015, 0, 0, 0, 0, 0, 0, 0, 0, 'Ach, there ye are. Let me see that priceless artifact ye''re swinging around like a stick ye bought at the gift shop!', 0),
(41017, 0, 0, 0, 0, 0, 0, 0, 0, 'Well isn''t that nice and shiny! You''ll need ta get more used to using that thing first. Then we can mess with it more rather than, you know, putting it in a museum and respecting it for the piece o'' history that it is.', 0);

-- Their places on the map: Betild, and the altar for what the second asks
DELETE FROM `quest_poi` WHERE `QuestID` IN (41015, 41017);
INSERT INTO `quest_poi` (`QuestID`, `BlobIndex`, `Idx1`, `ObjectiveIndex`, `QuestObjectiveID`, `QuestObjectID`, `MapID`, `WorldMapAreaId`, `Floor`, `Priority`, `Flags`, `WorldEffectID`, `PlayerConditionID`, `SpawnTrackingID`, `AlwaysAllowMergingBlobs`, `VerifiedBuild`) VALUES
(41015, 0, 0, -1, 0, 0, 1512, 1040, 1, 0, 0, 0, 0, 0, 0, 0),
(41017, 0, 0, -1, 0, 0, 1512, 1040, 1, 0, 0, 0, 0, 0, 0, 0),
(41017, 0, 1, 0, 281671, 46798, 1512, 1040, 1, 0, 2, 0, 0, 0, 0, 0);

DELETE FROM `quest_poi_points` WHERE `QuestID` IN (41015, 41017);
INSERT INTO `quest_poi_points` (`QuestID`, `Idx1`, `Idx2`, `X`, `Y`, `VerifiedBuild`) VALUES
(41015, 0, 0, 1416, 1347, 0),
(41017, 0, 0, 1416, 1347, 0),
(41017, 1, 0, 1412, 1344, 0);

-- What Saa'ra and Betild say of the altar when a priest whom Faol has sent comes up to it (the video). Saa'ra
-- is far from the altar, so her line goes to that priest alone.
DELETE FROM `creature_text` WHERE `CreatureID` IN (102709, 111241);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(111241, 0, 0, 'The altar is complete, Betild.', 12, 0, 100, 0, 0, 63410, 0, 105144, 5, 'Saa''ra - a priest with "Artifacts Need Artificers" comes to the altar'),
(102709, 0, 0, 'Yer a miracle worker, Saa''ra. It came out better than I designed it!', 12, 0, 100, 0, 0, 63411, 0, 105145, 0, 'Betild Deepanvil - answers Saa''ra');

DELETE FROM `areatrigger_template` WHERE `Id`=105144 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(105144, 1, 1, 0, 0, 0);

DELETE FROM `areatrigger_create_properties` WHERE `Id`=105144 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(105144, 1, 105144, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 18, 18, 0, 0, 0, 0, '', 0);

DELETE FROM `areatrigger` WHERE `SpawnId`=1512002;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1512002, 105144, 1, 1512, '0', 1412.4, 1344.2, 184.18, 0, 0, 0, 0, 'at_netherlight_temple_altar_of_light_and_shadow', 'Netherlight Temple - the Altar of Light and Shadow - Saa''ra and Betild speak of it (Quest: "Artifacts Need Artificers")', 0);
