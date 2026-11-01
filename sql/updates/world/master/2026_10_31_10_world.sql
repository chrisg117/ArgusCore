-- Legion Dalaran: the mailboxes and the portals. The 13 Legion mailboxes (241963-242231) stand on the curb nearest their
-- Wowhead pin centres. The lower room of the Chamber of the Guardian was Legion's portal hub (Karazhan, Dalaran Crater, Wyrmrest
-- Temple, Caverns of Time, Shattrath): one portal in front of each alcove. Greyfang Enclave's capital portals stand in the wall
-- niches, positions and order checked in game. A portal's disc lies along its facing, so it is turned 90 degrees from the viewer.
-- Karazhan uses template 258721 (type 22) because the type-10 246009 takes no click on this client. Team and class conditions
-- (source 32 group 5) gate the faction and order-hall portals. Portal to the Hall of the Guardian had no template: display and
-- spell 203976 (map 1513) are TrinityCore's for this entry (TDB 12.1), as a type-22 caster here.

DELETE FROM `gameobject_template` WHERE `entry`=247024;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data3`, `Data6`, `VerifiedBuild`) VALUES (247024, 22, 8111, 'Portal to the Hall of the Guardian', '', '', '', 1, 203976, -1, 1, 1, 27843);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 12203001 AND 12203034;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12203001, 241963, 1220, 7502, 7581, '0', 0, 0, 0, -1, -792.46, 4498.01, 732.05, 2.97, 0, 0, 0.996322, 0.085691, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203002, 241964, 1220, 7502, 7581, '0', 0, 0, 0, -1, -785.25, 4461.15, 735.32, 1.57, 0, 0, 0.706825, 0.707388, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203003, 241965, 1220, 7502, 7581, '0', 0, 0, 0, -1, -906.18, 4503.23, 732.00, 0.00, 0, 0, 0.000000, 1.000000, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203004, 241966, 1220, 7502, 7581, '0', 0, 0, 0, -1, -907.65, 4450.00, 736.26, 1.75, 0, 0, 0.767544, 0.640997, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203005, 241967, 1220, 7502, 7581, '0', 0, 0, 0, -1, -850.04, 4387.35, 738.33, 0.52, 0, 0, 0.257081, 0.966390, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203006, 242044, 1220, 7502, 7596, '0', 0, 0, 0, -1, -759.27, 4547.35, 728.28, 5.76, 0, 0, 0.258619, -0.965979, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203007, 242126, 1220, 7502, 7595, '0', 0, 0, 0, -1, -961.87, 4505.51, 734.17, 5.59, 0, 0, 0.339695, -0.940536, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203008, 242172, 1220, 7502, 7505, '0', 0, 0, 0, -1, -848.41, 4323.13, 744.82, 0.87, 0, 0, 0.421410, 0.906870, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203009, 242200, 1220, 7502, 7581, '0', 0, 0, 0, -1, -733.34, 4452.61, 733.90, 5.76, 0, 0, 0.258619, -0.965979, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203010, 242201, 1220, 7502, 7581, '0', 0, 0, 0, -1, -724.75, 4470.97, 733.22, 0.00, 0, 0, 0.000000, 1.000000, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203011, 242216, 1220, 7502, 7596, '0', 0, 0, 0, -1, -848.01, 4554.28, 728.22, 2.44, 0, 0, 0.939099, 0.343646, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203012, 242220, 1220, 7502, 7508, '0', 0, 0, 0, -1, -918.31, 4566.75, 729.27, 0.00, 0, 0, 0.000000, 1.000000, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203013, 242231, 1220, 7502, 7589, '0', 0, 0, 0, -1, -763.93, 4359.43, 729.07, 0.87, 0, 0, 0.421410, 0.906870, 300, 255, 1, '', NULL, 0), -- Mailbox
(12203014, 258721, 1220, 7502, 8045, '0', 0, 0, 0, -1, -871.44, 4501.61, 580.20, 5.11, 0, 0, 0.553526, -0.832832, 300, 255, 1, '', NULL, 0), -- Portal to Karazhan, Chamber of the Guardian
(12203015, 246008, 1220, 7502, 8045, '0', 0, 0, 0, -1, -880.16, 4508.31, 580.20, 6.15, 0, 0, 0.066543, -0.997784, 300, 255, 1, '', NULL, 0), -- Portal to Dalaran Crater, Chamber of the Guardian
(12203016, 246013, 1220, 7502, 8045, '0', 0, 0, 0, -1, -890.33, 4504.10, 580.20, 0.92, 0, 0, 0.443948, 0.896052, 300, 255, 1, '', NULL, 0), -- Portal to Wyrmrest Temple, Chamber of the Guardian
(12203017, 246005, 1220, 7502, 8045, '0', 0, 0, 0, -1, -891.76, 4493.19, 580.20, 1.96, 0, 0, 0.830497, 0.557023, 300, 255, 1, '', NULL, 0), -- Portal to Caverns of Time, Chamber of the Guardian
(12203018, 246010, 1220, 7502, 8045, '0', 0, 0, 0, -1, -883.04, 4486.49, 580.20, 3.01, 0, 0, 0.997836, 0.065749, 300, 255, 1, '', NULL, 0), -- Portal to Shattrath, Chamber of the Guardian, Alliance
(12203019, 246011, 1220, 7502, 8045, '0', 0, 0, 0, -1, -883.04, 4486.49, 580.20, 3.01, 0, 0, 0.997836, 0.065749, 300, 255, 1, '', NULL, 0), -- Portal to Shattrath, Chamber of the Guardian, Horde
(12203020, 246003, 1220, 7502, 7508, '0', 0, 0, 0, -1, -949.15, 4563.88, 729.28, 2.46, 0, 0, 0.942489, 0.334238, 300, 255, 1, '', NULL, 0), -- Portal to Exodar, Greyfang Enclave
(12203021, 245998, 1220, 7502, 7508, '0', 0, 0, 0, -1, -936.19, 4553.50, 729.22, 2.46, 0, 0, 0.942489, 0.334238, 300, 255, 1, '', NULL, 0), -- Portal to Ironforge, Greyfang Enclave
(12203022, 246002, 1220, 7502, 7508, '0', 0, 0, 0, -1, -929.76, 4548.35, 729.20, 2.46, 0, 0, 0.942489, 0.334238, 300, 255, 1, '', NULL, 0), -- Portal to Stormwind, Greyfang Enclave
(12203023, 245997, 1220, 7502, 7508, '0', 0, 0, 0, -1, -942.75, 4558.75, 729.26, 2.46, 0, 0, 0.942489, 0.334238, 300, 255, 1, '', NULL, 0), -- Portal to Darnassus, Greyfang Enclave
(12203024, 246006, 1220, 7502, 7508, '0', 0, 0, 0, -1, -950.95, 4572.16, 729.34, 1.78, 0, 0, 0.777072, 0.629412, 300, 255, 1, '', NULL, 0), -- Portal to Vale of Eternal Blossoms, Greyfang Enclave
(12203030, 251304, 1220, 7502, 7508, '0', 0, 0, 0, -1, -962.49, 4600.83, 735.25, 0.78, 0, 0, 0.380188, 0.924909, 300, 255, 1, '', NULL, 0), -- Portal to Sanctum of Light, Greyfang side (Wowhead pin)
(12203031, 251305, 1220, 7502, 7589, '0', 0, 0, 0, -1, -662.55, 4367.44, 747.99, 4.21, 0, 0, 0.860674, -0.509156, 300, 255, 1, '', NULL, 0), -- Portal to Sanctum of Light, Windrunner's side (Wowhead pin)
(12203032, 251311, 1220, 7502, 7505, '0', 0, 0, 0, -1, -844.82, 4316.32, 744.85, 5.98, 0, 0, 0.151013, -0.988532, 300, 255, 1, '', NULL, 0), -- Portal to the Maelstrom, Krasus' Landing (Wowhead pin)
(12203033, 251302, 1220, 8392, 7825, '0', 0, 0, 0, -1, -799.07, 4575.30, 700.58, 0.00, 0, 0, 0.000000, 1.000000, 300, 255, 1, '', NULL, 0), -- Portal to Dreadscar Rift, the Underbelly Descent (Wowhead pin)
(12203034, 247024, 1220, 7502, 7595, '0', 0, 0, 0, -1, -928.77, 4473.00, 733.75, 0.00, 0, 0, 0.000000, 1.000000, 300, 255, 1, '', NULL, 0); -- Portal to the Hall of the Guardian (Wowhead pin)

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=5 AND `SourceEntry` IN (258721, 246008, 246013, 246005, 246010, 246011, 246003, 245998, 246002, 245997, 246006, 251304, 251305, 251311, 251302, 247024, 245999, 246000, 246001, 246004, 246007);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 5, 246010, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Shattrath, Chamber of the Guardian, Alliance - Alliance only'),
(32, 5, 246011, 0, 0, 6, 0, 67, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Shattrath, Chamber of the Guardian, Horde - Horde only'),
(32, 5, 246003, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Exodar, Greyfang Enclave - Alliance only'),
(32, 5, 245998, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Ironforge, Greyfang Enclave - Alliance only'),
(32, 5, 246002, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Stormwind, Greyfang Enclave - Alliance only'),
(32, 5, 245997, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Darnassus, Greyfang Enclave - Alliance only'),
(32, 5, 246006, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Vale of Eternal Blossoms, Greyfang Enclave - Alliance only'),
(32, 5, 251304, 0, 0, 15, 0, 2, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Sanctum of Light, Greyfang side - paladins only'),
(32, 5, 251305, 0, 0, 15, 0, 2, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Sanctum of Light, Windrunner''s side - paladins only'),
(32, 5, 251311, 0, 0, 15, 0, 64, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to the Maelstrom, Krasus'' Landing - shamans only'),
(32, 5, 251302, 0, 0, 15, 0, 256, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to Dreadscar Rift, the Underbelly Descent - warlocks only'),
(32, 5, 247024, 0, 0, 15, 0, 128, 0, 0, '', 0, 0, 0, '', 'Dalaran - Portal to the Hall of the Guardian - mages only');
