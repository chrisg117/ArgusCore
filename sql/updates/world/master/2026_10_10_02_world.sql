-- Broken Shore scenario (map 1460): "Stop Gul'dan" - what Varian and Sylvanas say when Krosus has fallen, what
-- Varian and Gul'dan say when the Alliance has crossed Jaina's second bridge of ice, and the demons Gul'dan
-- brings before him.
--
-- The lines are Blizzard's (broadcast_text 99276 to 99306). Who says them, and when, is plain from what they say
-- or from a video of the retail scenario. Those of Sylvanas and Gul'dan are heard all over the map (TextRange 3),
-- as their others are.
DELETE FROM `creature_text` WHERE (`CreatureID`=90713 AND `GroupID` IN (17, 18, 19, 20, 21, 22, 23, 24)) OR (`CreatureID`=90709 AND `GroupID` IN (5, 6, 7)) OR (`CreatureID`=94276 AND `GroupID` IN (4, 5, 6, 7)) OR (`CreatureID` IN (90688, 91902, 93719) AND `GroupID`=0);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(90713, 17, 0, 'Ha, the bigger they are...', 14, 0, 100, 0, 0, 53489, 0, 99276, 0, 'King Varian Wrynn - Krosus has fallen'),
(90709, 5, 0, 'No time to gloat, Wrynn. We''ve got to stop Gul''dan before he summons another one of those things!', 14, 0, 100, 0, 0, 53490, 0, 99277, 3, 'Lady Sylvanas Windrunner - Krosus has fallen'),
(90713, 18, 0, 'Everyone, across the chasm!', 14, 0, 100, 0, 0, 53491, 0, 99278, 0, 'King Varian Wrynn - sends the Alliance across'),
(90709, 6, 0, 'We''ll take the ridge and cover your flank.', 14, 0, 100, 0, 0, 53492, 0, 99279, 3, 'Lady Sylvanas Windrunner - to Varian'),
(90713, 19, 0, 'Sylvanas, thank you.', 14, 0, 100, 0, 0, 53493, 0, 99280, 0, 'King Varian Wrynn - to Sylvanas'),
(90709, 7, 0, 'Good luck, Varian.', 14, 0, 100, 0, 0, 53494, 0, 99281, 3, 'Lady Sylvanas Windrunner - leaves for the ridge'),
(90713, 20, 0, 'And you.', 14, 0, 100, 0, 0, 53495, 0, 99282, 0, 'King Varian Wrynn - to Sylvanas'),
(90713, 21, 0, 'There is nowhere to run, Gul''dan. Give up now, and I will grant you a swift death.', 14, 0, 100, 0, 0, 53496, 0, 99283, 0, 'King Varian Wrynn - before Gul''dan'),
(94276, 4, 0, 'Run? Ha. You are right, human. There is nowhere to run from my master.  Now, witness the true might of Lord Sargeras!', 14, 0, 100, 0, 0, 53497, 0, 99284, 3, 'Gul''dan - to Varian'),
(90713, 22, 0, 'Hold steady, we''ve broken their lines before, we will again. Charge!', 14, 0, 100, 0, 0, 53498, 0, 99285, 0, 'King Varian Wrynn - has his forces charge'),
(90713, 23, 0, 'We will never fall to you, Gul''dan. The combined forces of Azeroth will triumph today.', 14, 0, 100, 0, 0, 53504, 0, 99291, 0, 'King Varian Wrynn - the first row of demons has fallen'),
(94276, 5, 0, 'I have seen the end of your pitiful world, Wrynn. You will perish in felfire, like ten thousand worlds before your own. The legion is ENDLESS.', 14, 0, 100, 0, 0, 53506, 0, 99293, 3, 'Gul''dan - the Legion appears'),
(90713, 24, 0, 'No matter how many demons you throw at us, we will cut you down, monster.', 14, 0, 100, 0, 0, 53513, 0, 99300, 0, 'King Varian Wrynn - the second row of demons has fallen'),
(91902, 0, 0, 'We return.', 14, 0, 100, 0, 0, 53516, 0, 99303, 3, 'Brutallus - before Gul''dan'),
(90688, 0, 0, 'To conquer this puny world.', 14, 0, 100, 0, 0, 53517, 0, 99304, 3, 'Tichondrius the Darkener - before Gul''dan'),
(93719, 0, 0, 'And burn it to a husk.', 14, 0, 100, 0, 0, 53518, 0, 99305, 3, 'Fel Commander Azgalor - before Gul''dan'),
(94276, 6, 0, 'Do you see now, human? The fate of this world is sealed. The legion cannot be stopped.', 14, 0, 100, 0, 0, 53519, 0, 99306, 3, 'Gul''dan - the Legion has come'),
(94276, 7, 0, 'And now you will fall.', 14, 0, 100, 0, 0, 53514, 0, 99301, 3, 'Gul''dan - his last before the cinematic');

-- Gul'dan before the tomb can be clicked on and not fought, like the one on his ledge above the pool
UPDATE `creature` SET `unit_flags`=768 WHERE `guid`=14600800;

-- The Mo'arg Spinebreaker that stood before him was a placeholder: the script has Gul'dan bring his demons now
DELETE FROM `creature` WHERE `guid`=14600801;
DELETE FROM `spawn_group` WHERE `groupId`=14608 AND `spawnType`=0 AND `spawnId`=14600801;

-- The demons he brings to be fought, and the lords of his Legion, are demons like the scenario's others and of
-- their levels. Their templates had the placeholder level 1 and a faction that is everyone's friend.
UPDATE `creature_template` SET `faction`=2780 WHERE `entry` IN (105199, 105203, 105206, 90662, 90688, 91672, 91902, 93719);
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry` IN (105199, 105203, 105206, 90688, 91672, 91902, 93719);

-- Where players are taken when the scenario has ended: the Skyfire over Stormwind Harbor, with Genn Greymane,
-- who the quest is handed in to, on its deck. Blizzard's data has where players arrive (the spell after "Broken
-- Shore Complete", 208514) and not where the ship is: it is put so that the middle of its main deck is under
-- that place, with its bow towards the city. Both are seen in the phase of the harbor's other preparations for
-- the Broken Shore.
DELETE FROM `gameobject_template_addon` WHERE `entry`=244823;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`, `mingold`, `maxgold`, `artkit0`, `artkit1`, `artkit2`, `artkit3`, `artkit4`, `WorldEffectID`, `AIAnimKitID`) VALUES
(244823, 0, 1048608, 0, 0, 0, 0, 0, 0, 0, 0, 0); -- The Skyfire: a map object that does not despawn, like the harbor's battleships

DELETE FROM `gameobject` WHERE `guid`=14600900;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14600900, 244823, 0, 1519, 4411, '0', 0, 6666, 0, -1, -8400.18, 1411.11, 114.4, 1.5708, 0, 0, 0.707107, 0.707107, 120, 255, 1, '', NULL, 0); -- The Skyfire

DELETE FROM `creature` WHERE `guid`=14600900;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14600900, 100395, 0, 1519, 4411, '0', 0, 6666, 0, -1, 0, 0, -8406.18, 1389.11, 135.0, 5.5, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Genn Greymane

-- His template had no level
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry`=100395;
