-- DK campaign: The Call of Icecrown, scenario 901 step 1812 "Power Overwhelming" (Task 3d). Design: dk-frost-task3d-design.md,
-- reworked in dk-frost-task3d-hilt-redesign.md after the user's account of retail (pass 7). On the Frozen Throne the Lich
-- King shows the player the hilt of Frostmourne; its click is the client's own: GO 242502 casts 186264 Forging, whose 4 s
-- 186253 forges the fragments and sends event 45692 (event_icc_scenario_fragments_forged): the blades are reforged (186203,
-- whose 186202 counts the step), a fresh hilt created in GO_STATE_ACTIVE plays the model's sequence (snow pile, Frostmourne,
-- the shatter, the hilt rising), the hanging twin blades (GO 247386, looted on the return in 3e) take its place and the
-- player is drawn inside Frostmourne for The Purge. The spot is the platform's centre (user-confirmed). Before the click the
-- hilt rests as the twin blades: no state of the model rests on the snow pile, an accepted gap

-- The Hilt of Frostmourne, the goober the player clicks
DELETE FROM `gameobject` WHERE `guid`=14800007;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14800007, 242502, 1480, 0, 0, '12', 0, 0, 0, -1, 503.60, -2124.60, 840.86, 0, 0, 0, 0, 1, 86400, 255, 1, '', NULL, 0);

-- 94102 Hilt of Frostmourne, the creature: the invisible target the Forging spells play on (both aim at a nearby entry and
-- fail without one). Of its template displays (an imp and an invisible stalker) only the stalker
DELETE FROM `creature_template_model` WHERE `CreatureID`=94102;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(94102, 0, 53300, 1, 0);
DELETE FROM `creature` WHERE `guid`=14800021;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14800021, 94102, 1480, 0, 0, '12', 0, 0, 0, 0, 503.60, -2124.60, 840.86, 0, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Hilt of Frostmourne (target)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceGroup`=1 AND `SourceEntry` IN (186253, 186267);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 186253, 0, 0, 31, 0, 3, 94102, 0, 0, 0, 0, '', 'Forging - target the Hilt of Frostmourne'),
(13, 1, 186267, 0, 0, 31, 0, 3, 94102, 0, 0, 0, 0, '', 'Forging - target the Hilt of Frostmourne');

DELETE FROM `event_script_names` WHERE `Id`=45692;
INSERT INTO `event_script_names` (`Id`, `ScriptName`) VALUES
(45692, 'event_icc_scenario_fragments_forged');

-- The Lich King: "Behold, the hilt of Frostmourne..." (line 1785, BroadcastText 96379), a talking head like 14800001-06
DELETE FROM `conversation_template` WHERE `Id`=14800007;
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(14800007, 1785, 12000, 0, '', 0);
DELETE FROM `conversation_actors` WHERE `ConversationId`=14800007;
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(14800007, 14800001, 0, 0, 99456, 64642, 0, 0, 0);
DELETE FROM `conversation_line_template` WHERE `Id`=1785;
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(1785, 0, 296, 0, 0, 0);

-- The blades taking form: package 1466 "7.0 Artifacts - Frost Death Knight - Acquisition - Looting the Blades (1. Loot
-- Scene)", which no client spell plays; flags 16 as 1076. It belongs to the return to the Frozen Throne after The Purge
-- (user, pass 7), which is 3e; nothing plays it yet
DELETE FROM `scene_template` WHERE `SceneId`=14800001;
INSERT INTO `scene_template` (`SceneId`, `Flags`, `ScriptPackageID`, `ScriptName`) VALUES
(14800001, 16, 1466, '');
