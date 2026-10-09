-- Broken Shore scenario (map 1460), "Destroy the Portal": the Eredar Chaos Guards shield the anchors, the
-- Mo'arg Painbringer comes from the portal, and what is said.
--
-- An Eredar Chaos Guard (90525) stands at each Shielded Anchor (101667) and channels "Chaos Shield" (181545)
-- on it, which its script sees to. The anchors do nothing themselves.
UPDATE `creature_template` SET `ScriptName`='npc_broken_shore_eredar_chaos_guard' WHERE `entry`=90525;
UPDATE `creature_template` SET `AIName`='NullCreatureAI' WHERE `entry`=101667;

-- The Mo'arg Painbringer (92564) was sniffed in front of King Varian Wrynn, which is where it fights. A video
-- of the retail scenario shows it appear in the demons' portal, behind the anchors, and come from there. No
-- data has the portal; the place is where it stands in that video, taken in game.
UPDATE `creature` SET `position_x`=1246.3, `position_y`=2438.8, `position_z`=44.0, `orientation`=2.98 WHERE `guid`=14600408 AND `id`=92564;

-- Text and sound are those of each line's broadcast text. Two of the four have a woman's voice only, which
-- makes them Jaina's. That the other two are Varian's, and which of the lines are yelled, is a guess.
DELETE FROM `creature_text` WHERE (`CreatureID`=90713 AND `GroupID` IN (1, 2)) OR (`CreatureID`=90714 AND `GroupID` IN (10, 11));
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(90713, 1, 0, 'We''ve got to take down this portal!', 12, 0, 100, 0, 0, 53392, 0, 99176, 0, 'King Varian Wrynn - at the portal'),
(90713, 2, 0, 'Form up, Alliance! We push them back to the portal! For Azeroth!', 14, 0, 100, 0, 0, 53394, 0, 99178, 0, 'King Varian Wrynn - sends the Alliance at the portal'),
(90714, 10, 0, 'The anchoring crystals are the key, just get us up there, and we''ll take care of it.', 12, 0, 100, 0, 0, 53393, 0, 99177, 0, 'Lady Jaina Proudmoore - at the portal'),
(90714, 11, 0, 'Focus on the crystals!', 14, 0, 100, 0, 0, 53395, 0, 99179, 0, 'Lady Jaina Proudmoore - at the portal');

-- The portal. Nothing was sniffed of it. A video of the retail scenario shows dark arches around a green
-- orb behind the anchors, gone when the stage is over. "Legion Gateway" (240211) is a template this database
-- already has, made next to the scenario's Spire of Woe (240194) and of the kind of its "WMO Bridge"; that
-- it is the portal is a guess. The place is where the portal stands in that video, taken in game.
DELETE FROM `gameobject` WHERE `guid`=14600409;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14600409, 240211, 1460, 7534, 8120, '12', 0, 0, 0, -1, 1244.5, 2440.8, 44.0, 2.965, 0.000000, 0.000000, 0.996103, 0.088197, 7200, 255, 1, '', NULL, 0);

DELETE FROM `spawn_group` WHERE `groupId`=14604 AND `spawnType`=1 AND `spawnId`=14600409;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14604, 1, 14600409);

-- The green orb in the portal is "Portal On Visual" (181308), which was picked by eye against that video.
-- Something has to wear it: a General Purpose Bunny JMF (54020), which players do not see.
DELETE FROM `creature` WHERE `guid`=14600409;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600409, 54020, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1244.5, 2440.8, 44.0, 2.965, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- General Purpose Bunny JMF, the portal's orb

DELETE FROM `creature_addon` WHERE `guid`=14600409;
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(14600409, 0, 0, 0, 0, 3, 0, 1, 0, 0, 0, 0, 0, 4, '181308'); -- Portal On Visual

DELETE FROM `spawn_group` WHERE `groupId`=14604 AND `spawnType`=0 AND `spawnId`=14600409;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14604, 0, 14600409);
