-- Broken Shore scenario (map 1460), stage "Raze the Black City", Alliance side.
-- There is no sniff of the scenario. The positions come from Wowhead map pins converted to map 1460 like the
-- spawns of the earlier stages, with heights from the extracted terrain, so they are approximate.
-- The pins are sightings rather than spawn points, so there are more of them than there were demons:
-- - an elite gets one spawn at the centre of its pins, which lie close together and are taken to be one
--   creature moving;
-- - the imps and Grinning Shadowstalkers, which have by far the most pins, get a spawn for every second pin;
-- - the other demons get a spawn for each pin, moved 4 yards apart where several share a pin.
-- All of them face the graveyard at the edge of the city, which is the side players come from.
-- The five Unattended Cannons that players fight from stand at their pins and point into the city. The one at
-- the shore is moved 15 yards inland, because its pin lies below sea level.
-- No demon stands within 50 yards of a cannon, which is the range of its Fel Cannonball: those whose pins are
-- closer are left out, or moved out if elite.
-- Everything respawns after 2 minutes: killing each demon once is worth 193 of the 300 points the stage needs.

-- The group is spawned by the instance script when the stage starts and despawned when it ends
DELETE FROM `spawn_group_template` WHERE `groupId`=14605;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14605, 'Broken Shore Scenario - Raze the Black City (Alliance)', 4);

DELETE FROM `creature` WHERE `guid` BETWEEN 14605000 AND 14605059;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14605000, 91967, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1179.86, 2252.15, 11.85, 2.2821, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Infernal Siegebreaker
(14605001, 91970, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1061.79, 2232.90, 7.49, 1.2969, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Invader
(14605002, 91970, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1040.12, 2209.80, 5.43, 1.2001, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felguard Invader
(14605003, 94189, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1128.52, 2256.00, 12.33, 1.9118, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Living Felblaze
(14605004, 94189, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1102.86, 2256.00, 10.68, 1.6546, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Living Felblaze
(14605005, 94189, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1138.79, 2248.30, 10.30, 1.9756, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Living Felblaze
(14605006, 94190, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1092.59, 2256.00, 10.84, 1.5464, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Burning Sentry
(14605007, 94190, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1102.86, 2232.90, 9.92, 1.6382, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Burning Sentry
(14605008, 94191, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1097.72, 2248.30, 10.82, 1.5983, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Burning Terrorhound
(14605009, 94191, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1123.39, 2240.60, 8.79, 1.8239, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Burning Terrorhound
(14605010, 94191, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1066.92, 2232.90, 7.52, 1.3377, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Burning Terrorhound
(14605011, 97510, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1025.86, 2240.60, 10.11, 1.0108, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Soulbound Destructor
(14605012, 97510, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 984.79, 2240.60, 15.41, 0.7855, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Soulbound Destructor
(14605013, 97510, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1188.99, 2217.50, 9.44, 2.1857, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Soulbound Destructor
(14605014, 97510, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1194.12, 2209.80, 10.61, 2.1842, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Soulbound Destructor
(14605015, 97510, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1005.32, 2209.80, 10.08, 1.0046, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Soulbound Destructor
(14605016, 100621, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1277.39, 2202.10, 13.84, 2.4581, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Mother Virila
(14605017, 102701, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1113.12, 2229.05, 9.18, 1.7194, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Mo'arg Painbringer
(14605018, 102702, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1066.92, 2248.30, 11.94, 1.3042, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Wrathguard Dreadblade
(14605019, 102702, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1061.79, 2217.50, 5.40, 1.3272, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Wrathguard Dreadblade
(14605020, 102702, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1032.12, 2209.80, 5.90, 1.1517, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Wrathguard Dreadblade
(14605021, 102703, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1016.32, 2242.80, 10.42, 0.9415, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fel Lord Dukaz
(14605022, 102704, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1045.11, 2229.05, 8.55, 1.1824, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fel Lord Zarnoz
(14605023, 102705, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 995.06, 2236.75, 12.96, 0.8514, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fel Lord Rakaz
(14605024, 102706, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 995.06, 2240.60, 13.28, 0.8344, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Grinning Shadowstalker
(14605025, 103896, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1133.66, 2279.10, 13.75, 2.0667, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felfire Imp
(14605026, 103896, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1133.66, 2256.00, 11.16, 1.9591, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felfire Imp
(14605027, 103896, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 989.92, 2248.30, 14.95, 0.7731, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felfire Imp
(14605028, 103896, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1087.46, 2209.80, 7.48, 1.5180, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felfire Imp
(14605029, 103896, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1215.79, 2179.00, 6.15, 2.1842, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felfire Imp
(14605030, 103896, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1040.12, 2171.30, 2.97, 1.2745, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Felfire Imp
(14605031, 103897, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1132.52, 2279.10, 13.57, 2.0544, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fiery Trickster
(14605032, 103897, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 974.52, 2256.00, 16.31, 0.6667, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fiery Trickster
(14605033, 103897, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1030.99, 2217.50, 6.42, 1.1235, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fiery Trickster
(14605034, 103897, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1210.66, 2194.40, 11.16, 2.2082, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fiery Trickster
(14605035, 103897, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1282.52, 2179.00, 14.19, 2.4004, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fiery Trickster
(14605036, 103897, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1036.12, 2179.00, 4.01, 1.2410, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Fiery Trickster
(14605037, 103899, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1124.52, 2279.10, 13.07, 1.9629, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowflame Imp
(14605038, 103899, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1030.99, 2256.00, 13.39, 0.9773, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowflame Imp
(14605039, 103899, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1231.19, 2240.60, 14.86, 2.4619, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowflame Imp
(14605040, 103899, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1183.86, 2209.80, 9.24, 2.1338, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowflame Imp
(14605041, 103899, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1051.52, 2202.10, 5.43, 1.2868, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowflame Imp
(14605042, 103899, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1061.79, 2186.70, 4.12, 1.3716, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowflame Imp
(14605043, 103899, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1032.12, 2171.30, 4.50, 1.2342, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowflame Imp
(14605044, 103899, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1046.39, 2155.90, 5.71, 1.3268, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowflame Imp
(14605045, 110614, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1047.96, 2260.60, 12.87, 1.0907, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Malificus
(14605046, 110616, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 974.52, 2232.90, 18.02, 0.7747, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Dark Worshipper
(14605047, 110616, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1200.39, 2209.80, 11.17, 2.2133, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Dark Worshipper
(14605048, 110616, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1175.86, 2209.80, 8.70, 2.0922, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Dark Worshipper
(14605049, 110616, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1000.19, 2209.80, 11.24, 0.9791, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Dark Worshipper
(14605050, 110617, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 984.79, 2248.30, 15.83, 0.7493, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowsworn Harbinger
(14605051, 110617, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1154.19, 2232.90, 10.42, 2.0370, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowsworn Harbinger
(14605052, 110617, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1180.99, 2217.50, 8.45, 2.1445, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowsworn Harbinger
(14605053, 110617, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1000.19, 2217.50, 11.31, 0.9528, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowsworn Harbinger
(14605054, 110617, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1186.12, 2209.80, 9.59, 2.1453, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Shadowsworn Harbinger
(14605055, 100959, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1041.26, 2325.30, 20.38, 3.5843, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Unattended Cannon
(14605056, 100959, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1061.79, 2317.60, 22.79, 3.9273, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Unattended Cannon
(14605057, 100959, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1066.92, 2309.90, 22.68, 4.1117, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Unattended Cannon
(14605058, 100959, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1005.32, 2302.20, 15.26, 3.6381, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Unattended Cannon
(14605059, 100959, 1460, 7534, 7547, '12', 0, 0, 0, -1, 0, 0, 1133.66, 2178.60, 5.76, 4.9338, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- Unattended Cannon

DELETE FROM `spawn_group` WHERE `groupId`=14605;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14605, 0, 14605000),
(14605, 0, 14605001),
(14605, 0, 14605002),
(14605, 0, 14605003),
(14605, 0, 14605004),
(14605, 0, 14605005),
(14605, 0, 14605006),
(14605, 0, 14605007),
(14605, 0, 14605008),
(14605, 0, 14605009),
(14605, 0, 14605010),
(14605, 0, 14605011),
(14605, 0, 14605012),
(14605, 0, 14605013),
(14605, 0, 14605014),
(14605, 0, 14605015),
(14605, 0, 14605016),
(14605, 0, 14605017),
(14605, 0, 14605018),
(14605, 0, 14605019),
(14605, 0, 14605020),
(14605, 0, 14605021),
(14605, 0, 14605022),
(14605, 0, 14605023),
(14605, 0, 14605024),
(14605, 0, 14605025),
(14605, 0, 14605026),
(14605, 0, 14605027),
(14605, 0, 14605028),
(14605, 0, 14605029),
(14605, 0, 14605030),
(14605, 0, 14605031),
(14605, 0, 14605032),
(14605, 0, 14605033),
(14605, 0, 14605034),
(14605, 0, 14605035),
(14605, 0, 14605036),
(14605, 0, 14605037),
(14605, 0, 14605038),
(14605, 0, 14605039),
(14605, 0, 14605040),
(14605, 0, 14605041),
(14605, 0, 14605042),
(14605, 0, 14605043),
(14605, 0, 14605044),
(14605, 0, 14605045),
(14605, 0, 14605046),
(14605, 0, 14605047),
(14605, 0, 14605048),
(14605, 0, 14605049),
(14605, 0, 14605050),
(14605, 0, 14605051),
(14605, 0, 14605052),
(14605, 0, 14605053),
(14605, 0, 14605054),
(14605, 0, 14605055),
(14605, 0, 14605056),
(14605, 0, 14605057),
(14605, 0, 14605058),
(14605, 0, 14605059);
