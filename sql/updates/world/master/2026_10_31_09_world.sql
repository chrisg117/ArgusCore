-- Legion Dalaran: the inn, the banks and the services. None of these stood in the city; their Legion templates were
-- bare (faction 35, npcflag 0). Faction, flags, gossip links, equipment and addon rows are TrinityCore's values for the same
-- entries (TDB 12.1 build 69497); flag bits this core does not know (0x400000000 and above) are dropped. Positions are
-- Wowhead pin centres on Dalaran floor 10 with z on the walkable floor of the room (navmesh and the Dalaran2.wmo collision),
-- facings point into the room; the Alliance and Horde inn staff carry a team condition, like their retail phases.

UPDATE `creature_template` SET `faction`=2007, `npcflag`=66177, `unit_flags`=0, `subname`='Innkeeper' WHERE `entry`=96806;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=65536, `unit_flags`=0, `subname`='Assistant Innkeeper' WHERE `entry`=96807;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=642, `unit_flags`=256, `subname`='Bartender' WHERE `entry`=96809;
UPDATE `creature_template` SET `faction`=2027, `npcflag`=65536, `unit_flags`=768, `subname`='Assistant Innkeeper' WHERE `entry`=96789;
UPDATE `creature_template` SET `faction`=2027, `npcflag`=66179, `unit_flags`=768, `subname`='Innkeeper' WHERE `entry`=96790;
UPDATE `creature_template` SET `faction`=2123, `npcflag`=66179, `unit_flags`=768, `subname`='Innkeeper' WHERE `entry`=96796;
UPDATE `creature_template` SET `faction`=2123, `npcflag`=65536, `unit_flags`=768, `subname`='Assistant Innkeeper' WHERE `entry`=96799;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=131073, `unit_flags`=0, `subname`='Banker' WHERE `entry`=96817;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=131073, `unit_flags`=0, `subname`='Banker' WHERE `entry`=96818;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=131073, `unit_flags`=0, `subname`='Banker' WHERE `entry`=96819;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=131072, `unit_flags`=0, `subname`='Banker' WHERE `entry`=96820;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=131072, `unit_flags`=0, `subname`='Banker' WHERE `entry`=96821;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=131072, `unit_flags`=0, `subname`='Banker' WHERE `entry`=96822;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=131072, `unit_flags`=0, `subname`='Banker' WHERE `entry`=96823;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=131072, `unit_flags`=0, `subname`='Banker' WHERE `entry`=96825;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=131072, `unit_flags`=0, `subname`='Banker' WHERE `entry`=96826;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=786433, `unit_flags`=0, `subname`='Guild Master' WHERE `entry`=96811; -- gossip | petitioner | tabard designer
UPDATE `creature_template` SET `faction`=35, `npcflag`=536870912, `unit_flags`=0, `subname`='Void Storage' WHERE `entry`=85290; -- vaultkeeper
UPDATE `creature_template` SET `faction`=35, `npcflag`=268435585, `unit_flags`=0, `subname`='Transmogrifier' WHERE `entry`=99867;
UPDATE `creature_template` SET `faction`=2007, `npcflag`=1, `unit_flags`=0, `subname`='Barber' WHERE `entry`=96781; -- gossip only: this core has no barber flag, 7.3.5 barbers are their chairs
UPDATE `creature_template` SET `faction`=2007, `npcflag`=4194433, `unit_flags`=0, `subname`='Stable Master' WHERE `entry`=96507;

-- 99867 and 96507 from TDB 12.1; the innkeepers have no link there. Amisi takes the Wrath Lounge menu 10201 (the Lounge's greeting), the two others the plain inn menu 342.
DELETE FROM `creature_template_gossip` WHERE `CreatureID` IN (99867, 96507, 96806, 96790, 96796);
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES (99867, 21043, 48676), (96507, 9821, 48676), (96806, 10201, 0), (96790, 342, 0), (96796, 342, 0);
-- Option 1 of 10201 is a duplicate browse row whose broadcast text is the bind line, so the client showed the bind line twice.
DELETE FROM `gossip_menu_option` WHERE `MenuID`=10201 AND `OptionID`=1;
-- Aludane's other lines (Val'sharah, Faronaar, Stormheim, Suramar) belong to quest flights that are not built; hidden until they are (level = 255 never holds).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=18723 AND `SourceEntry` IN (1, 3, 4, 5);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 18723, 1, 0, 0, 27, 0, 255, 0, 0, '', 0, 0, 0, '', 'Aludane Whitecloud - quest flight line hidden until its quest is built'),
(15, 18723, 3, 0, 0, 27, 0, 255, 0, 0, '', 0, 0, 0, '', 'Aludane Whitecloud - quest flight line hidden until its quest is built'),
(15, 18723, 4, 0, 0, 27, 0, 255, 0, 0, '', 0, 0, 0, '', 'Aludane Whitecloud - quest flight line hidden until its quest is built'),
(15, 18723, 5, 0, 0, 27, 0, 255, 0, 0, '', 0, 0, 0, '', 'Aludane Whitecloud - quest flight line hidden until its quest is built');
DELETE FROM `creature_equip_template` WHERE `CreatureID`=96809;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES (96809, 1, 13861, 0, 0, 0, 0, 0, 0, 0, 0, 66337);
DELETE FROM `creature_template_addon` WHERE `entry`=96507;
INSERT INTO `creature_template_addon` (`entry`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES (96507, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, '');

DELETE FROM `creature` WHERE `guid` BETWEEN 12203001 AND 12203023;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12203001, 96806, 1220, 7502, 8140, '0', 0, 0, 0, -1, 0, 0, -800.03, 4465.82, 735.01, 1.96, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Amisi Azuregaze, Innkeeper, the Legerdemain Lounge
(12203002, 96807, 1220, 7502, 8140, '0', 0, 0, 0, -1, 0, 0, -810.24, 4476.52, 735.01, 5.83, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Afsaneh Asrar, Assistant Innkeeper
(12203003, 96809, 1220, 7502, 8140, '0', 0, 0, 0, -1, 0, 0, -794.87, 4483.77, 745.88, 4.07, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Arille Azuregaze, Bartender, upstairs
(12203004, 96790, 1220, 7502, 7508, '0', 0, 0, 0, -1, 0, 0, -930.66, 4512.75, 733.25, 0.00, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Isirami Fairwind, Innkeeper, Greyfang Enclave (Alliance)
(12203005, 96789, 1220, 7502, 7508, '0', 0, 0, 0, -1, 0, 0, -973.79, 4550.95, 740.96, 0.00, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Caliel Brightwillow, Assistant Innkeeper, Greyfang Enclave (Alliance)
(12203006, 96796, 1220, 7502, 7589, '0', 0, 0, 0, -1, 0, 0, -761.61, 4338.22, 729.07, 0.00, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Uda the Beast, Innkeeper, Windrunner's Sanctuary (Horde)
(12203007, 96799, 1220, 7502, 7589, '0', 0, 0, 0, -1, 0, 0, -774.02, 4317.80, 729.05, 0.00, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Abohba, Assistant Innkeeper, Windrunner's Sanctuary (Horde)
(12203008, 96817, 1220, 7502, 8532, '0', 0, 0, 0, -1, 0, 0, -664.69, 4442.61, 738.12, 3.53, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Teller Rames, the Bank of Dalaran
(12203009, 96818, 1220, 7502, 8532, '0', 0, 0, 0, -1, 0, 0, -670.32, 4437.56, 738.12, 3.06, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Teller Hanners
(12203010, 96819, 1220, 7502, 8532, '0', 0, 0, 0, -1, 0, 0, -672.15, 4431.24, 738.12, 2.09, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Teller Hoag
(12203011, 96820, 1220, 7502, 8532, '0', 0, 0, 0, -1, 0, 0, -664.91, 4440.63, 738.12, 3.37, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Paymaster Alstein
(12203012, 85290, 1220, 7502, 7502, '0', 0, 0, 0, -1, 0, 0, -997.07, 4522.63, 739.49, 3.65, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Vaultkeeper Sarniv, Void Storage
(12203013, 96821, 1220, 7502, 7502, '0', 0, 0, 0, -1, 0, 0, -1021.76, 4521.80, 739.49, 6.25, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Teller Gee, the western bank
(12203014, 96822, 1220, 7502, 7502, '0', 0, 0, 0, -1, 0, 0, -1023.85, 4515.05, 739.49, 0.24, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Teller Almeida
(12203015, 96823, 1220, 7502, 7502, '0', 0, 0, 0, -1, 0, 0, -1017.56, 4528.11, 739.49, 5.90, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Teller Plushner
(12203016, 96825, 1220, 7502, 7502, '0', 0, 0, 0, -1, 0, 0, -1024.78, 4517.69, 739.49, 0.13, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Paymaster Chang
(12203017, 96826, 1220, 7502, 7502, '0', 0, 0, 0, -1, 0, 0, -1021.21, 4525.37, 739.49, 6.08, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Paymaster Amadi
(12203018, 96811, 1220, 7502, 7502, '0', 0, 0, 0, -1, 0, 0, -886.20, 4451.83, 737.59, 5.45, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Andrew Matthews, Guild Master
(12203019, 99867, 1220, 7502, 7596, '0', 0, 0, 0, -1, 0, 0, -810.19, 4550.20, 729.50, 5.50, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Warpweaver Tuviss, Transmogrifier
(12203020, 96781, 1220, 7502, 7502, '0', 0, 0, 0, -1, 0, 0, -755.03, 4446.89, 736.88, 2.34, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kizi Copperclip, Barber
(12203021, 96507, 1220, 7502, 7581, '0', 0, 0, 0, -1, 0, 0, -792.35, 4389.26, 740.16, 2.50, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Tassia Whisperglen, Stable Master
(12203022, 35594, 1220, 7502, 7596, '0', 0, 0, 0, -1, 0, 0, -728.68, 4555.14, 729.64, 3.82, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Brassbolt Mechawrench, Steam-Powered Auctioneer
(12203023, 35607, 1220, 7502, 7596, '0', 0, 0, 0, -1, 0, 0, -726.50, 4553.50, 729.64, 3.53, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Reginald Arcfire, Steam-Powered Auctioneer

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (96789, 96790, 96796, 96799);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 96790, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Dalaran - Isirami Fairwind - Alliance only'),
(32, 3, 96789, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Dalaran - Caliel Brightwillow - Alliance only'),
(32, 3, 96796, 0, 0, 6, 0, 67, 0, 0, '', 0, 0, 0, '', 'Dalaran - Uda the Beast - Horde only'),
(32, 3, 96799, 0, 0, 6, 0, 67, 0, 0, '', 0, 0, 0, '', 'Dalaran - Abohba - Horde only');
