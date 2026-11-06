-- DK campaign: The Call of Icecrown, scenario 901 step 1810, the Spire's trash between the fragments (Task 3b-2b).
-- Design: dk-frost-task3b2b-design.md. From the user's retail recording: Phantom Mages and Swordsmen in pairs at the
-- landing, Spectral Knights before Halahk, Anguished Souls before Magroth and a pack travelling with Gavinrad. Kits are
-- the client's spells of the name and shape the recording shows (no sniff, a fidelity gap). Positions are points the
-- user walked on 1480 (GM.log), for the user to correct

UPDATE `creature_template` SET `faction`=14, `ScriptName`='npc_icc_scenario_spire_trash' WHERE `entry` IN (94907, 94913, 95433, 95434, 97659);
UPDATE `creature_template` SET `unit_class`=8 WHERE `entry`=94913; -- Phantom Mage: mana for its Frostbolt
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `Entry` IN (94907, 94913, 95433, 95434, 97659) AND `DifficultyID`=0;

DELETE FROM `creature` WHERE `guid` BETWEEN 14800007 AND 14800019;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14800007, 94913, 1480, 0, 0, '12', 0, 0, 0, 0, 4160.00, 2768.00, 351.00, 3.1416, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Phantom Mage, landing pair 1
(14800008, 95434, 1480, 0, 0, '12', 0, 0, 0, 0, 4160.00, 2775.00, 351.00, 3.1416, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Phantom Swordsman, landing pair 1
(14800009, 94913, 1480, 0, 0, '12', 0, 0, 0, 0, 4182.00, 2811.00, 353.20, 3.5000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Phantom Mage, landing pair 2 (north walkway)
(14800010, 95434, 1480, 0, 0, '12', 0, 0, 0, 0, 4186.00, 2808.00, 353.20, 3.5000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Phantom Swordsman, landing pair 2 (north walkway)
(14800011, 94907, 1480, 0, 0, '12', 0, 0, 0, 0, 4398.00, 2677.00, 345.60, 2.4000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Spectral Knight, before Halahk
(14800012, 97659, 1480, 0, 0, '12', 0, 0, 0, 0, 4402.00, 2681.00, 345.80, 2.4000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Spectral Knight, before Halahk
(14800013, 95433, 1480, 0, 0, '12', 0, 0, 0, 0, 4399.00, 2862.00, 344.30, 3.3000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Anguished Soul, before Magroth
(14800014, 95433, 1480, 0, 0, '12', 0, 0, 0, 0, 4394.00, 2863.00, 346.10, 3.3000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Anguished Soul, before Magroth
(14800015, 95433, 1480, 0, 0, '12', 0, 0, 0, 0, 4388.00, 2864.00, 350.40, 3.3000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Anguished Soul, before Magroth
(14800016, 94907, 1480, 0, 0, '12', 0, 0, 0, 0, 4378.00, 2769.00, 357.00, 0.0000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Spectral Knight, Gavinrad's pack
(14800017, 95434, 1480, 0, 0, '12', 0, 0, 0, 0, 4358.00, 2785.00, 356.00, 1.5708, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Phantom Swordsman, Gavinrad's pack
(14800018, 95434, 1480, 0, 0, '12', 0, 0, 0, 0, 4374.00, 2769.00, 356.40, 0.0000, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Phantom Swordsman, Gavinrad's pack

-- Gavinrad travels in a pack: they pull together (members assist the leader and the leader the members)
DELETE FROM `creature_formations` WHERE `leaderGUID`=14800004 OR `memberGUID` IN (14800004, 14800016, 14800017, 14800018);
INSERT INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`, `point_1`, `point_2`) VALUES
(14800004, 14800004, 0, 0, 3, 0, 0),
(14800004, 14800016, 0, 0, 3, 0, 0),
(14800004, 14800017, 0, 0, 3, 0, 0),
(14800004, 14800018, 0, 0, 3, 0, 0);
