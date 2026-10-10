-- Broken Shore scenario (map 1460), "Raze the Black City": what the Alliance's leaders say.
--
-- Text and sound are those of each line's broadcast text; the lines are the scenario's next after those of
-- "Destroy the Portal". Two have a woman's voice only, which makes them Jaina's. Who says the others, and
-- which of them are yelled, is a guess: the one that tells what happened is given to Gelbin Mekkatorque,
-- as are the three fight yells that speak of blasters, Gnomeregan and the Nether, and the rest to Varian.
-- The Argent Dawnbringer's line has both voices.
DELETE FROM `creature_text` WHERE (`CreatureID`=90713 AND `GroupID` IN (3, 4, 5, 6, 7, 8)) OR (`CreatureID`=90714 AND `GroupID` IN (12, 13)) OR (`CreatureID`=90716 AND `GroupID` IN (0, 1)) OR (`CreatureID`=110615 AND `GroupID`=0);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(90713, 3, 0, 'Heal the wounded, we can''t rest here for long.', 12, 0, 100, 0, 0, 53396, 0, 99180, 0, 'King Varian Wrynn - where the forces fell back to'),
(90713, 4, 0, 'One demon at a time. With me, Alliance, leave nothing standing!', 14, 0, 100, 0, 0, 53399, 0, 99183, 0, 'King Varian Wrynn - sends the Alliance into the Black City'),
(90713, 5, 0, 'We''re halfway there, take heart Alliance!', 14, 0, 100, 0, 0, 53406, 0, 99190, 0, 'King Varian Wrynn - the Black City is half razed'),
(90713, 6, 0, 'Back where you came from!', 14, 0, 100, 0, 0, 53404, 0, 99188, 0, 'King Varian Wrynn - in a fight'),
(90713, 6, 1, 'We''re pushing them back! Don''t let up!', 14, 0, 100, 0, 0, 53405, 0, 99189, 0, 'King Varian Wrynn - in a fight'),
(90713, 6, 2, 'For the Alliance!', 14, 0, 100, 0, 0, 53407, 0, 99191, 0, 'King Varian Wrynn - in a fight'),
(90714, 12, 0, 'What happened here?', 12, 0, 100, 0, 0, 53397, 0, 99181, 0, 'Lady Jaina Proudmoore - where the forces fell back to'),
(90714, 13, 0, 'Keep your eyes open for survivors! Save as many as you can!', 14, 0, 100, 0, 0, 53403, 0, 99187, 0, 'Lady Jaina Proudmoore - the forces go into the Black City'),
(90716, 0, 0, 'Taste blaster!', 14, 0, 100, 0, 0, 53408, 0, 99192, 0, 'Gelbin Mekkatorque - in a fight'),
(90716, 0, 1, 'For Gnomeregan!', 14, 0, 100, 0, 0, 53409, 0, 99193, 0, 'Gelbin Mekkatorque - in a fight'),
(90716, 0, 2, 'Back to the Nether with you!', 14, 0, 100, 0, 0, 53410, 0, 99194, 0, 'Gelbin Mekkatorque - in a fight'),
(90716, 1, 0, 'It happened in seconds! We were fighting our way up the shore and then BOOM, explosions of fel energy all around us! And these buildings just grew up out of them, filled with demons! I don''t know how we''re going to be able to push them back at this rate!', 12, 0, 100, 0, 0, 53398, 0, 99182, 0, 'Gelbin Mekkatorque - where the forces fell back to'),
(90713, 7, 0, 'You''re from the Crusade! What happened? Where is Tirion?', 12, 0, 100, 0, 0, 53400, 0, 99184, 0, 'King Varian Wrynn - to the Argent Dawnbringer before the gate of the Black City'),
(90713, 8, 0, 'Save your strength, we will find him.', 12, 0, 100, 0, 0, 53402, 0, 99186, 0, 'King Varian Wrynn - to the Argent Dawnbringer before the gate of the Black City'),
(110615, 0, 0, 'Don''t know... they were on us in seconds... felfire everywhere...', 12, 0, 100, 0, 0, 0, 0, 99185, 0, 'Argent Dawnbringer - to Varian');

-- An Argent Dawnbringer (110615) is caged before the city's gate, where Varian talks with it. That is from a
-- video of the retail scenario; nothing of it was sniffed. The creature is numbered among the city's demons.
-- Two more lie dead on the fence behind the cage. The cage is "Legion Cage" (240535), which was picked by
-- eye against that video, as were the places, taken in game. A player who opens a cage frees the Dawnbringer
-- inside, which the cage's script sees to.
UPDATE `gameobject_template` SET `ScriptName`='go_broken_shore_legion_cage' WHERE `entry`=240535;
UPDATE `creature_template` SET `ScriptName`='npc_broken_shore_argent_dawnbringer' WHERE `entry`=110615;
DELETE FROM `creature` WHERE `guid` BETWEEN 14605060 AND 14605062;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14605060, 110615, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1104.0, 2325.1, 20.3, 1.04, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Argent Dawnbringer, caged before the gate
(14605061, 110615, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1102.2, 2324.6, 27.5, 0.5, 7200, 0, 0, 100, 0, NULL, 33555200, NULL, NULL, 0), -- dead on the fence
(14605062, 110615, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 1107.6, 2322.2, 25.7, 3.9, 7200, 0, 0, 100, 0, NULL, 33555200, NULL, NULL, 0); -- dead on the fence

-- The two on the fence lie dead (stand state 7) and can be neither picked nor fought (unit flags 0x02000300)
DELETE FROM `creature_addon` WHERE `guid` IN (14605061, 14605062);
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(14605061, 0, 0, 0, 7, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(14605062, 0, 0, 0, 7, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL);

DELETE FROM `gameobject` WHERE `guid`=14605060;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14605060, 240535, 1460, 7534, 8290, '12', 0, 0, 0, -1, 1104.0, 2325.1, 20.3, 1.04, 0.000000, 0.000000, 0.496880, 0.867819, 7200, 255, 1, '', NULL, 0); -- Legion Cage

DELETE FROM `spawn_group` WHERE `groupId`=14605 AND `spawnId` BETWEEN 14605060 AND 14605062;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14605, 0, 14605060),
(14605, 0, 14605061),
(14605, 0, 14605062),
(14605, 1, 14605060);
