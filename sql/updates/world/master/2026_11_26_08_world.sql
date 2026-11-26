-- Azsuna, The Head of the Snake (37470). After Save Yourself, Farondis (89009) at the slave pen sends the player to kill
-- Athissa (88855) on the Hatecoil ledge (POI 12,5684). Nightwatcher Thaldrys (97841) and four Nar'thalas Nightwatchers
-- (97845), Farondis's people, fight beside the player. Farondis (91403) and Thaldrys (97886) wait in front of the Eye of
-- Azshara portal (POI 1,5765). The reward spell 181404 plays conversation 346 between them, and Farondis then offers 38286
-- (owned by the Eye of Azshara work; only the starter row is added here). The scripts are in zone_azsuna.cpp.
-- Positions: navplace on the navmesh, at least 4 yd from walls (Athissa 6 yd). Her POI point is on a lower level, cut
-- off from the rest of the mesh, so she stands on the platform above it at z 10.3.

-- Athissa and her two Hatecoil Myrmidons on the ledge; the Nightwatchers on the ground below, facing her
DELETE FROM `creature` WHERE `guid` BETWEEN 12205310 AND 12205319;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12205310, 88855, 1220, 7334, 0, '0', 0, 0, 0, 0, 11.29, 5683.29, 10.30, 2.06, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12205311, 89006, 1220, 7334, 0, '0', 0, 0, 0, 0,  8.52, 5685.05, 10.30, 2.06, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12205312, 89006, 1220, 7334, 0, '0', 0, 0, 0, 0,  8.71, 5679.71, 10.30, 2.06, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12205313, 97841, 1220, 7334, 0, '0', 0, 0, 0, 0,  3.00, 5699.00,  8.46, 5.20, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12205314, 97845, 1220, 7334, 0, '0', 0, 0, 0, 0,  1.00, 5696.00,  8.43, 5.20, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12205315, 97845, 1220, 7334, 0, '0', 0, 0, 0, 0,  2.00, 5703.00,  8.51, 5.20, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12205316, 97845, 1220, 7334, 0, '0', 0, 0, 0, 0,  6.00, 5707.00,  8.56, 5.20, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12205317, 97845, 1220, 7334, 0, '0', 0, 0, 0, 0,  8.00, 5702.00,  8.49, 5.20, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
-- Farondis and Thaldrys before the portal (GO 252247 at 1.4,5809), facing it
(12205318, 91403, 1220, 7334, 0, '0', 0, 0, 0, 0,  0.29, 5765.71,  3.60, 1.55, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12205319, 97886, 1220, 7334, 0, '0', 0, 0, 0, 0,  1.50, 5770.00,  3.85, 4.40, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0);

-- Athissa can be fought now (she was friendly); the Nightwatchers are friendly to players and hostile to monsters, and
-- passive until Thaldrys rallies them (the script)
UPDATE `creature_template` SET `faction` = 16, `ScriptName` = 'npc_head_of_the_snake_athissa' WHERE `entry` = 88855;
UPDATE `creature_template` SET `faction` = 290, `ScriptName` = 'npc_head_of_the_snake_nightwatcher' WHERE `entry` IN (97841, 97845);

-- levels (all but Athissa were 1 or 98): 110 with the zone's 98..110 scaling; health modifiers stay the data's
UPDATE `creature_template_difficulty` SET `MinLevel` = 110, `MaxLevel` = 110, `LevelScalingMin` = 98, `LevelScalingMax` = 110
WHERE `DifficultyID` = 0 AND `Entry` IN (89006, 91403, 97841, 97845, 97886);

-- Farondis ends 37470 and starts Wrath of Azshara (38286)
DELETE FROM `creature_questender` WHERE `id` = 91403 AND `quest` = 37470;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(91403, 37470, 0);
DELETE FROM `creature_queststarter` WHERE `id` = 91403 AND `quest` = 38286;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(91403, 38286, 0);

-- Who sees them: the ledge while 37470 is in progress, the portal pair once it is complete or rewarded. Each entry also
-- has an OR group for any seer that is not a player: the core runs these conditions for creature seers too, and without
-- it Athissa, her guards and the Nightwatchers could not see (or fight) one another
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (88855, 89006, 97841, 97845, 91403, 97886);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 88855, 0, 0, 47, 0, 37470,  8, 0, '', 0, 0, 0, '', 'Athissa is visible while quest 37470 (The Head of the Snake) is in progress'),
(32, 3, 89006, 0, 0, 47, 0, 37470,  8, 0, '', 0, 0, 0, '', 'Hatecoil Myrmidon is visible while quest 37470 (The Head of the Snake) is in progress'),
(32, 3, 97841, 0, 0, 47, 0, 37470,  8, 0, '', 0, 0, 0, '', 'Nightwatcher Thaldrys (ledge) is visible while quest 37470 (The Head of the Snake) is in progress'),
(32, 3, 97845, 0, 0, 47, 0, 37470,  8, 0, '', 0, 0, 0, '', 'Nar''thalas Nightwatcher is visible while quest 37470 (The Head of the Snake) is in progress'),
(32, 3, 91403, 0, 0, 47, 0, 37470, 66, 0, '', 0, 0, 0, '', 'Prince Farondis (portal) is visible once quest 37470 (The Head of the Snake) is complete or rewarded'),
(32, 3, 97886, 0, 0, 47, 0, 37470, 66, 0, '', 0, 0, 0, '', 'Nightwatcher Thaldrys (portal) is visible once quest 37470 (The Head of the Snake) is complete or rewarded'),
(32, 3, 88855, 0, 1, 32, 0, 16, 0, 0, '', 1, 0, 0, '', 'Athissa is visible to any seer that is not a player'),
(32, 3, 89006, 0, 1, 32, 0, 16, 0, 0, '', 1, 0, 0, '', 'Hatecoil Myrmidon is visible to any seer that is not a player'),
(32, 3, 97841, 0, 1, 32, 0, 16, 0, 0, '', 1, 0, 0, '', 'Nightwatcher Thaldrys (ledge) is visible to any seer that is not a player'),
(32, 3, 97845, 0, 1, 32, 0, 16, 0, 0, '', 1, 0, 0, '', 'Nar''thalas Nightwatcher is visible to any seer that is not a player'),
(32, 3, 91403, 0, 1, 32, 0, 16, 0, 0, '', 1, 0, 0, '', 'Prince Farondis (portal) is visible to any seer that is not a player'),
(32, 3, 97886, 0, 1, 32, 0, 16, 0, 0, '', 1, 0, 0, '', 'Nightwatcher Thaldrys (portal) is visible to any seer that is not a player');

-- Conversation 346, "Head of the Snake: Farondis Finale" (spell 181404): ConversationLine 1001-1006, the one client chain
-- with these voiced lines (broadcast 93518-93523). Speakers from the text: Farondis lines 1-2 and 6, Thaldrys 3-5.
-- StartTime is an estimate from line length (about 2.6 words a second plus a short gap), to re-time by ear.
DELETE FROM `conversation_template` WHERE `Id` = 346;
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(346, 1001, 24600, 0, '', 0);
DELETE FROM `conversation_line_template` WHERE `Id` BETWEEN 1001 AND 1006;
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(1001,     0, 0, 0, 0, 0),
(1002,  4500, 0, 0, 0, 0),
(1003,  6800, 0, 1, 0, 0),
(1004,  9100, 0, 1, 0, 0),
(1005, 15100, 0, 1, 0, 0),
(1006, 19600, 0, 0, 0, 0);
DELETE FROM `conversation_actors` WHERE `ConversationId` = 346;
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(346, 0, 12205318, 0, 0, 0, 0, 0, 0),
(346, 0, 12205319, 1, 0, 0, 0, 0, 0);
