-- Broken Shore scenario (map 1460): "The Highlord" and the start of "Krosus" - Jaina's bridge of ice, Gul'dan
-- and the Horde's leaders at the pool that Tirion hangs over, and what is said there before Krosus rises.
--
-- The lines are Blizzard's (broadcast_text 99233 to 99248) and in their order. Who says them is from videos of
-- the retail scenario. Those of Tirion, Gul'dan and the Horde's leaders are heard all over the map (TextRange 3),
-- because they stand far from the ledge the Alliance is on.
DELETE FROM `creature_text` WHERE (`CreatureID`=90713 AND `GroupID` IN (9, 10, 11, 12, 13, 14, 15, 16)) OR (`CreatureID`=90714 AND `GroupID` IN (14, 15, 16, 17, 18)) OR (`CreatureID`=90716 AND `GroupID`=2) OR (`CreatureID`=90717 AND `GroupID`=12) OR (`CreatureID`=91951 AND `GroupID` IN (0, 1)) OR (`CreatureID`=94276 AND `GroupID` IN (0, 1, 2, 3)) OR (`CreatureID`=90709 AND `GroupID` IN (0, 1, 2, 3, 4)) OR (`CreatureID`=90711 AND `GroupID`=0) OR (`CreatureID`=90544 AND `GroupID`=0);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(90716, 2, 0, 'How are we going to get across?', 12, 0, 100, 0, 0, 53448, 0, 99233, 0, 'Gelbin Mekkatorque - at the gap to the crevasse'),
(90714, 14, 0, 'I''ve got this.', 12, 0, 100, 0, 0, 53449, 0, 99234, 0, 'Lady Jaina Proudmoore - makes the bridge of ice'),
(90713, 9, 0, 'Tirion!', 14, 0, 100, 0, 0, 53450, 0, 99235, 0, 'King Varian Wrynn - on the ledge before Tirion'),
(90709, 0, 0, 'Gul''dan! You will pay for this!', 14, 0, 100, 0, 0, 53451, 0, 99236, 3, 'Lady Sylvanas Windrunner - to Gul''dan'),
(91951, 0, 0, 'Stay back... it''s a trap... the Light will protect me...', 12, 0, 100, 0, 0, 53452, 0, 99237, 3, 'Highlord Tirion Fordring - held over the pool'),
(94276, 0, 0, 'Ha, you fool! You stand before the temple of a GOD. Your pitiful Light cannot reach you here. How fortunate that your friends have arrived just in time to watch you die.', 14, 0, 100, 0, 0, 53453, 0, 99238, 3, 'Gul''dan - to Tirion'),
(94276, 1, 0, 'Destroy him.', 14, 0, 100, 0, 0, 53454, 0, 99239, 3, 'Gul''dan - drops Tirion into the pool'),
(91951, 1, 0, 'The Light... will... ahh! Aahhhh!!', 14, 0, 100, 0, 0, 53455, 0, 99240, 3, 'Highlord Tirion Fordring - falls into the pool'),
(90711, 0, 0, 'Fordring!', 14, 0, 100, 0, 0, 53456, 0, 99241, 3, 'Thrall - Tirion falls'),
(90714, 15, 0, 'No!', 14, 0, 100, 0, 0, 53457, 0, 99242, 0, 'Lady Jaina Proudmoore - Tirion falls'),
(94276, 2, 0, 'All you have worked for, all you have sacrificed, just to see your champions fall to ash one by one.', 14, 0, 100, 0, 0, 53458, 0, 99243, 3, 'Gul''dan - to the leaders'),
(90713, 10, 0, 'We will never give in to hopelessness!', 14, 0, 100, 0, 0, 53459, 0, 99244, 0, 'King Varian Wrynn - to Gul''dan'),
(90709, 1, 0, 'The armies of Azeroth will triumph this day, together.', 14, 0, 100, 0, 0, 53460, 0, 99245, 3, 'Lady Sylvanas Windrunner - to Gul''dan'),
(90713, 11, 0, 'Together, for Azeroth!', 14, 0, 100, 0, 0, 53461, 0, 99246, 0, 'King Varian Wrynn - to Sylvanas'),
(94276, 3, 0, 'Destroy them!', 14, 0, 100, 0, 0, 53462, 0, 99247, 3, 'Gul''dan - sends Krosus'),
(90544, 0, 0, 'You will know fear.', 14, 0, 100, 0, 0, 53463, 0, 99248, 0, 'Krosus - rises from the pool'),
(90713, 12, 0, 'Brace yourselves, Alliance, it''s coming this way!', 14, 0, 100, 0, 0, 53464, 0, 99249, 0, 'King Varian Wrynn - Krosus comes at the Alliance'),
(90713, 13, 0, 'Focus your fire, bring him down!', 14, 0, 100, 0, 0, 53466, 0, 99251, 0, 'King Varian Wrynn - Krosus is at the Alliance''s ledge'),
(90713, 14, 0, 'He''s heading your way, Windrunner!', 14, 0, 100, 0, 0, 53474, 0, 99261, 0, 'King Varian Wrynn - Krosus leaves for the Horde'),
(90709, 2, 0, 'Let''s go, Horde, it''s time to show the Alliance how we slay demons.', 14, 0, 100, 0, 0, 53475, 0, 99262, 3, 'Lady Sylvanas Windrunner - Krosus is at the Horde''s ledge'),
(90709, 3, 0, 'All yours, Alliance.', 14, 0, 100, 0, 0, 53487, 0, 99274, 3, 'Lady Sylvanas Windrunner - Krosus goes back to the Alliance'),
(90713, 15, 0, 'We''ve got him this time! All forces, concentrate your fire!', 14, 0, 100, 0, 0, 53488, 0, 99275, 0, 'King Varian Wrynn - Krosus is back at the Alliance''s ledge'),
(90713, 16, 0, 'You''ve got another shot at the kill, Sylvanas!', 14, 0, 100, 0, 0, 53485, 0, 99272, 0, 'King Varian Wrynn - Krosus leaves for the Horde again'),
(90709, 4, 0, 'Consider your job done then, Wrynn.', 14, 0, 100, 0, 0, 53486, 0, 99273, 3, 'Lady Sylvanas Windrunner - Krosus is at the Horde''s ledge again'),
(90714, 16, 0, 'Beasts from the felfire emerge, be careful!', 14, 0, 100, 0, 0, 53476, 0, 99263, 0, 'Lady Jaina Proudmoore - demons come with Krosus''s first Fel Storm'),
(90714, 17, 0, 'They''re coming again, let''s put them on ice!', 14, 0, 100, 0, 0, 53478, 0, 99265, 0, 'Lady Jaina Proudmoore - demons come with Krosus''s second Fel Storm'),
(90714, 18, 0, 'Another wave incoming!', 14, 0, 100, 0, 0, 53471, 0, 99257, 0, 'Lady Jaina Proudmoore - demons come with Krosus''s third Fel Storm');

-- Gul'dan, on a ledge over the pool, from which he holds Tirion. He goes when Krosus comes, with the rest of the
-- stage's group. Nobody can fight him there (unit flags 0x300), but he can be picked, as on retail.
-- The Horde's leaders stand on a ledge of their own across the pool, in a group that stays. Nobody fights them or
-- is fought by them (unit flags 0x300). Where Gul'dan and Sylvanas are is from a video of the retail scenario and
-- was taken in game; the places of the other three around her are guesses.
DELETE FROM `creature` WHERE `guid`=14600601 OR `guid` BETWEEN 14601400 AND 14601403;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600601, 94276, 1460, 7534, 8120, '12', 0, 0, 0, -1, 0, 0, 1531.8, 1745.6, 52.6, 2.35, 7200, 0, 0, 100, 0, NULL, 768, NULL, NULL, 0), -- Gul'dan
(14601400, 90709, 1460, 7534, 8453, '12', 0, 0, 0, -1, 0, 0, 1424.1, 1757.3, 34.2, 0.34, 7200, 0, 0, 100, 0, NULL, 768, NULL, NULL, 0), -- Lady Sylvanas Windrunner
(14601401, 90711, 1460, 7534, 8453, '12', 0, 0, 0, -1, 0, 0, 1422.8, 1761.1, 34.2, 0.34, 7200, 0, 0, 100, 0, NULL, 768, NULL, NULL, 0), -- Thrall
(14601402, 90710, 1460, 7534, 8453, '12', 0, 0, 0, -1, 0, 0, 1425.4, 1753.5, 34.2, 0.34, 7200, 0, 0, 100, 0, NULL, 768, NULL, NULL, 0), -- Baine Bloodhoof
(14601403, 90708, 1460, 7534, 8453, '12', 0, 0, 0, -1, 0, 0, 1420.6, 1756.2, 34.2, 0.34, 7200, 0, 0, 100, 0, NULL, 768, NULL, NULL, 0); -- Vol'jin

DELETE FROM `spawn_group_template` WHERE `groupId`=14614;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14614, 'Broken Shore Scenario - Horde Leaders', 4);

DELETE FROM `spawn_group` WHERE (`groupId`=14606 AND `spawnType`=0 AND `spawnId`=14600601) OR `groupId`=14614;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14606, 0, 14600601),
(14614, 0, 14601400),
(14614, 0, 14601401),
(14614, 0, 14601402),
(14614, 0, 14601403);

-- Krosus rises from the pool further from the ledges, facing Tirion: the place is from a video of the retail
-- scenario and was taken in game, at the height he had.
UPDATE `creature` SET `position_x`=1485.2, `position_y`=1719.7, `orientation`=1.41 WHERE `guid`=14600700;

-- Tirion is strangled as he hangs (emote state 473), facing Gul'dan
UPDATE `creature` SET `orientation`=5.49 WHERE `guid`=14600600;
DELETE FROM `creature_addon` WHERE `guid`=14600600;
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(14600600, 0, 0, 0, 0, 0, 0, 1, 0, 473, 0, 0, 0, 0, NULL);

-- The Alliance Priest (92074) was never sniffed and kept the placeholder level 1, at which the demons that scale
-- with a player's level take many times the damage he does: he killed Krosus in two hits. Give him the level of
-- the scenario's other Alliance forces, which is also the level scaling range stored for him.
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry`=92074;

-- Krosus goes between the Alliance's and the Horde's ledge as his health falls
UPDATE `creature_template` SET `ScriptName`='npc_broken_shore_krosus' WHERE `entry`=90544;

-- His Fel Storm's missiles are sent by his script
DELETE FROM `spell_script_names` WHERE `spell_id`=221080 AND `ScriptName`='spell_broken_shore_krosus_fel_storm';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(221080, 'spell_broken_shore_krosus_fel_storm');

-- The Burning Sentry casts Fel Eruption and the Living Felblaze Mass Incineration
UPDATE `creature_template` SET `ScriptName`='npc_broken_shore_burning_sentry' WHERE `entry`=94190;
UPDATE `creature_template` SET `ScriptName`='npc_broken_shore_living_felblaze' WHERE `entry`=94189;
