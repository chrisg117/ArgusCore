-- "Blade in Twilight" (scenario 991): its eighth stage, "Death to the Deacon", in the prison chamber under
-- the tomb.
--
-- The chamber had nothing in it. What happens there is taken from a video of the retail scenario and from
-- the client's data (the script of the cutscene, the conversation, the spells); where the Deacon, his two
-- guards and the dome over Zakajz are was marked in game against the video. The lines are Blizzard's
-- (broadcast_text 104430, 104477, 104508 to 104511, 104575 and 105388).

-- The cutscene that shows the Deacon at his work, before the door of the chamber: scene 1182, which the
-- spell "Boss Reveal Scene: Shadow Priest" (201321) plays. A player can skip it, as on retail (the flags
-- are those of the Broken Shore's cutscenes).
DELETE FROM `scene_template` WHERE `SceneId`=1182;
INSERT INTO `scene_template` (`SceneId`, `Flags`, `ScriptPackageID`, `ScriptName`) VALUES
(1182, 58, 1551, 'scene_blade_in_twilight_boss_reveal');

-- The Deacon and his Shadow Images were hostile and of levels 98 to 110 already, each of a level of its own
-- between the two; the Faceless were placeholders: friendly to all, of level 1. Zakajz lies dead.
UPDATE `creature_template` SET `ScriptName`='npc_twilight_deacon_farthing' WHERE `entry`=101148;
UPDATE `creature_template` SET `faction`=16, `ScriptName`='npc_faceless_guardian' WHERE `entry`=102015;
UPDATE `creature_template` SET `faction`=16, `ScriptName`='npc_faceless_corrupter' WHERE `entry`=101118;
UPDATE `creature_template` SET `ScriptName`='npc_zakajz_the_corruptor' WHERE `entry`=102039;
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `DifficultyID`=0 AND `Entry` IN (101148, 102015, 101118, 102989);

-- The Deacon holds the blade, Xal'atath (the client's own data for him says so)
DELETE FROM `creature_equip_template` WHERE `CreatureID`=101148 AND `ID`=1;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(101148, 1, 128827, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- What Slaghammer says when the Deacon has been seen (his other lines in the tomb are in
-- 2026_10_11_01_world.sql and 2026_10_11_02_world.sql), and the Deacon in his fight
DELETE FROM `creature_text` WHERE (`CreatureID`=101899 AND `GroupID`=4) OR (`CreatureID`=101148 AND `GroupID` IN (0, 1, 2));
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(101899, 4, 0, 'This is nae good! I kin hear more comin'' from above. I''ll handle ''em. Ye take care o'' the Deacon!', 12, 0, 100, 0, 0, 63443, 0, 104477, 0, 'Shadowlord Slaghammer - the Deacon has been seen'),
(101148, 0, 0, 'Defend me, Blade! You said this would work!', 14, 0, 100, 0, 0, 63444, 0, 104430, 0, 'Twilight Deacon Farthing - his guards have fallen'),
(101148, 1, 0, 'You think to stop me? With this weapon I can do anything! ANYTHING!', 14, 0, 100, 0, 0, 63445, 0, 104575, 0, 'Twilight Deacon Farthing - is attacked'),
(101148, 2, 0, 'You cannot trust your own mind!', 12, 0, 100, 0, 0, 63446, 0, 105388, 0, 'Twilight Deacon Farthing - calls up Shadow Images and Void Tendrils');

-- What the Deacon and the blade say when the blade fails him: conversation 1126, which the spell
-- "Conversation - Death of the Deacon" (201791) starts. Its four lines are the client's (ConversationLine
-- 2741 to 2744). Who speaks which, with which view of the speaker, and when each begins is not in the
-- client's data: the times are from the video, the views are the client's for a human man and for this
-- blade ("Default-ShadowPriestArtifactBlade"), and the ids of the two speakers are not known.
DELETE FROM `conversation_template` WHERE `Id`=1126;
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(1126, 2741, 35250, 0, '', 0);

DELETE FROM `conversation_actors` WHERE `ConversationId`=1126;
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(1126, 0, 0, 0, 101148, 67003, 0, 0, 0), -- Twilight Deacon Farthing
(1126, 0, 0, 1, 102255, 67546, 0, 0, 0); -- Xal'atath, Blade of the Black Empire

DELETE FROM `conversation_line_template` WHERE `Id` BETWEEN 2741 AND 2744;
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(2741, 0, 82, 0, 0, 0), -- This... this shouldn't be happening! Why aren't you protecting me?!
(2742, 7500, 748, 1, 0, 0), -- Weak and pitiful! The Twilight Father did not dare wield me...
(2743, 16250, 82, 0, 0, 0), -- Give me another chance! No! Noooo!
(2744, 23250, 748, 1, 0, 0); -- You, on the other hand... Yes, I foresee us doing great things together...

-- The beam from the blade that undoes the Deacon. The spell is "Chained", one of two alike that follow
-- "Conversation - Death of the Deacon", and is cast at a creature that the client's data does not name.
-- It is a stand-in: by its name it is the chains on Zakajz, and what the beam is on retail is not known.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=201792;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 201792, 0, 0, 31, 0, 3, 101148, 0, '', 0, 0, 0, '', 'Xal''atath''s beam - is cast at Twilight Deacon Farthing');

-- The Deacon before the dome, turned to it; his two guards between him and the door, turned to the door;
-- and Zakajz in the middle of the dome, who cannot be fought
DELETE FROM `creature` WHERE `guid` BETWEEN 15390038 AND 15390041;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15390038, 101148, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 1872.1, 2188.45, 36.83, 5.06, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Deacon Farthing
(15390039, 101118, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1887.29, 2195.72, 37.54, 1.75, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Faceless Corrupter, on the left as seen from the door
(15390040, 102015, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1859.54, 2185.98, 37.62, 1.29, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Faceless Guardian, on the right
(15390041, 102039, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 0, 1878.37, 2171.22, 35.84, 1.57, 7200, 0, 0, 100, 0, NULL, 33555200, NULL, NULL, '', NULL, 0); -- Zakajz the Corruptor

-- The dome is what "Zakajz Rebirth State" looks like: of the scenario's spells it is the one whose visual
-- has a model that large, and the cutscene's script keeps Zakajz in view
DELETE FROM `creature_addon` WHERE `guid`=15390041;
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(15390041, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '206390');
