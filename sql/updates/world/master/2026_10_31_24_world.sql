-- DH campaign, The Hunt: stage 3 of scenario 900, "Wrath of the Illidari". From the client and the Warcraft Wiki: the three
-- Felshield Wards, whose goober sends the stage's event; Lyana's corpse at the first, Igrius at the second, Varedis and his
-- crusher after it, Illysanna's body at the third. Ours: every position (no source gives any), which kill is worth how much
-- of the soul bar, and the respawn of two minutes that keeps the bar reachable.
UPDATE `creature_template` SET `faction`=16 WHERE `entry` IN (95313, 95423, 95285, 95452);
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `DifficultyID`=0 AND `Entry` IN (94375, 95313, 95423, 95285, 95452);

DELETE FROM `creature` WHERE `guid` BETWEEN 12204109 AND 12204127;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204109, 94375, 1498, 0, 0, '12', 0, 0, 0, 0, 1185.00, 5140.00, 56.70, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204110, 94375, 1498, 0, 0, '12', 0, 0, 0, 0, 1170.00, 5140.00, 62.82, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204111, 94375, 1498, 0, 0, '12', 0, 0, 0, 0, 1160.00, 5070.00, 54.16, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204112, 94375, 1498, 0, 0, '12', 0, 0, 0, 0, 1168.00, 5052.00, 56.46, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204113, 94375, 1498, 0, 0, '12', 0, 0, 0, 0, 1150.00, 5060.00, 57.10, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204114, 94375, 1498, 0, 0, '12', 0, 0, 0, 0, 1085.00, 4860.00, 56.44, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204115, 95313, 1498, 0, 0, '12', 0, 0, 0, 0, 1140.00, 5025.00, 50.58, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204116, 95313, 1498, 0, 0, '12', 0, 0, 0, 0, 1150.00, 5030.00, 52.65, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204117, 95313, 1498, 0, 0, '12', 0, 0, 0, 0, 1150.00, 5000.00, 53.27, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204118, 95313, 1498, 0, 0, '12', 0, 0, 0, 0, 1120.00, 5000.00, 52.99, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204119, 95313, 1498, 0, 0, '12', 0, 0, 0, 0, 1150.00, 5090.00, 54.89, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204120, 95313, 1498, 0, 0, '12', 0, 0, 0, 0, 1100.00, 4930.00, 52.56, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204121, 95423, 1498, 0, 0, '12', 0, 0, 0, 0, 1110.00, 4960.00, 49.93, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204122, 95423, 1498, 0, 0, '12', 0, 0, 0, 0, 1095.00, 4945.00, 49.85, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204123, 95423, 1498, 0, 0, '12', 0, 0, 0, 0, 1120.00, 4940.00, 52.68, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204124, 95423, 1498, 0, 0, '12', 0, 0, 0, 0, 1120.00, 4910.00, 53.82, 3.97, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204125, 95285, 1498, 0, 0, '12', 0, 0, 0, 0, 1120.00, 5060.00, 50.22, 3.97, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Fel Commander Igrius
(12204126, 97620, 1498, 0, 0, '12', 0, 0, 0, 0, 1185.00, 5180.00, 55.72, 3.97, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Lyana Darksorrow
(12204127, 100436, 1498, 0, 0, '12', 0, 0, 0, 0, 1090.00, 4870.00, 60.05, 3.97, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Illysanna Ravencrest

-- Lyana and Illysanna lie where they fell
DELETE FROM `creature_addon` WHERE `guid` IN (12204126, 12204127);
INSERT INTO `creature_addon` (`guid`, `StandState`) VALUES
(12204126, 7),
(12204127, 7);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 12204001 AND 12204003;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(12204001, 243428, 1498, 0, 0, '12', 0, 0, 1180.00, 5180.00, 58.32, 0, 0, 0, 0, 1, 86400, 255, 1, 0),
(12204002, 243472, 1498, 0, 0, '12', 0, 0, 1120.00, 5030.00, 49.80, 0, 0, 0, 0, 1, 86400, 255, 1, 0),
(12204003, 243473, 1498, 0, 0, '12', 0, 0, 1090.00, 4880.00, 59.89, 0, 0, 0, 0, 1, 86400, 255, 1, 0);

DELETE FROM `creature_text` WHERE `CreatureID`=94310;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(94310, 0, 0, 'So, Illidari, you seek my head, that you might take the Twinblades as your own? Allow me to introduce you to their power... personally.', 14, 0, 100, 0, 0, 0, 0, 97082, 0, 'Varedis Felsoul: after the second ward');
