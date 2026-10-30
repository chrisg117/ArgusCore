-- The Fallen Lion (40517): "(Optional) Ride a gryphon to Stormwind Keep" has no optional flag, and the letter's
-- objective is sequenced after it, so the quest could not be finished without the ride, which nothing offered. As in
-- the retail video: a white Gilnean Gryphon waits at the Skyfire's rail between Tess and Crowley on their armored
-- gryphons; clicking it (198209, Mount Gilnean Gryphon) brings the player's own, which flies them to the Keep's gate
-- with Tess and Crowley alongside, talking (lines 2643-6470, 2650-2651: custom conversations 1519003, 1519004);
-- landing counts (npc_the_fallen_lion_gilnean_gryphon). The way is Wowhead's pins of the flying gryphon (101677).
-- The gryphon's vehicle is the Vampiric Felbat's (1907): the retail one is not known.

UPDATE `creature_template` SET `VehicleId` = 1907, `ScriptName` = 'npc_the_fallen_lion_gilnean_gryphon' WHERE `entry` = 100696;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 100696;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(100696, 198209, 3, 0);

-- On the Skyfire's deck (Wowhead 21.2, 33.4 for the gryphon; Tess and Crowley beside it), on the bow, in the ship's phase
DELETE FROM `creature` WHERE `guid` BETWEEN 15190029 AND 15190031;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(15190029, 100696, 0, 1519, 4411, '0', 6666, 0, 0, 0, -8382.70, 1354.60, 126.80, 4.30, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Gilnean Gryphon
(15190030, 101708, 0, 1519, 4411, '0', 6666, 0, 0, 0, -8385.00, 1358.00, 126.50, 4.30, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Princess Tess Greymane
(15190031, 101713, 0, 1519, 4411, '0', 6666, 0, 0, 0, -8380.40, 1351.20, 126.80, 4.30, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Commander Lorna Crowley

DELETE FROM `creature_addon` WHERE `guid` IN (15190030, 15190031);
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(15190030, 0, 34881, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''), -- on her armored gryphon
(15190031, 0, 34881, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''); -- on her armored gryphon

-- Who sees them, and who may click the gryphon: while the ride is still to do
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (100696, 101708, 101713);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 100696 AND `SourceEntry` = 198209;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 100696, 0, 0, 47, 0, 40517, 8, 0, '', 0, 0, 0, '', 'Gilnean Gryphon (Skyfire): The Fallen Lion in progress'),
(32, 3, 100696, 0, 0, 48, 0, 281187, 0, 1, '', 1, 0, 0, '', 'Gilnean Gryphon (Skyfire): not yet ridden to the Keep'),
(32, 3, 101708, 0, 0, 47, 0, 40517, 8, 0, '', 0, 0, 0, '', 'Tess (Skyfire): The Fallen Lion in progress'),
(32, 3, 101708, 0, 0, 48, 0, 281187, 0, 1, '', 1, 0, 0, '', 'Tess (Skyfire): not yet ridden to the Keep'),
(32, 3, 101713, 0, 0, 47, 0, 40517, 8, 0, '', 0, 0, 0, '', 'Crowley (Skyfire): The Fallen Lion in progress'),
(32, 3, 101713, 0, 0, 48, 0, 281187, 0, 1, '', 1, 0, 0, '', 'Crowley (Skyfire): not yet ridden to the Keep'),
(18, 100696, 198209, 0, 0, 47, 0, 40517, 8, 0, '', 0, 0, 0, '', 'Mount Gilnean Gryphon: The Fallen Lion in progress'),
(18, 100696, 198209, 0, 0, 48, 0, 281187, 0, 1, '', 1, 0, 0, '', 'Mount Gilnean Gryphon: not yet ridden to the Keep');

-- Tess and Crowley on the way (their actors are added by the script: they are the rider's own)
DELETE FROM `conversation_template` WHERE `Id` IN (1519003, 1519004);
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(1519003, 2643, 16000, 0, '', 0),
(1519004, 2650, 6000, 0, '', 0);

DELETE FROM `conversation_line_template` WHERE `Id` IN (2643, 2644, 2645, 6470, 2650, 2651);
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(2643, 0, 0, 0, 0, 0), -- Tess: I've seen my father angry before, but never like this...
(2644, 6000, 0, 1, 0, 0), -- Crowley: All Gilneans have a rage inside us, Tess...
(2645, 11000, 0, 0, 0, 0), -- Tess: Indeed. It's sometimes hard to know where the rage ends and the man begins.
(6470, 15000, 0, 0, 0, 0), -- (no text)
(2650, 0, 0, 0, 0, 0), -- Tess: We're almost at the keep. Please tell King Anduin he's in our thoughts, $n.
(2651, 2500, 0, 1, 0, 0); -- Crowley: Quickly, Tess! The gunship will be ready to leave soon!
