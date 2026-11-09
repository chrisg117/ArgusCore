-- Warlock S6: the hall opening. The Heart of the Dreadscar (40731, scenario 998 on map 1522), Power Overwhelming
-- (40821), Rebuilding the Council (40823) and The Path of the Dreadscar (40824), in the Dreadscar Rift (map 1107).
--
-- Givers (fork data; the data has none): Calydus 101097 offers 40731 after Ritual Ruination, beside the Circle's fel
-- gateway (Wowhead's acquisition video, 44:10), and 40821 (RewardNextQuest of 40731, offered only by a starter of it)
-- and 40823; Ritssyn 101921 offers 40824 once raised. The way into 998: a fel gateway in the Circle seen with 40731 in the
-- log (go_dreadscar_heart_gateway, to WorldSafeLocs 5333's Finale front entrance). In 998: Jagganoth 101892 (962's
-- 98831 and the prison's spawns hidden from a warlock on 40731), the heart on his death, the altar in the Seat of the
-- Overlord (heart, then the armies), and Order Hall Transition 201205 to the hall by Calydus. In the hall: Calydus, the
-- Felblood Altar (an artifact forge), Ritssyn's soul flame and Ritssyn, the Command Map, and a portal to Dalaran; in the
-- Circle, the Portal to Dreadscar Rift once 40731 is rewarded. Looks (lineups local-only): the altar 27636, the flame
-- 16656, the gateway 30007 (the S2 pick).
--
-- lint: texts pending 40731 offer/request texts from the user's texts file (advisor ruling 2026-11-08)
-- lint: texts pending 40821 offer/request texts from the user's texts file (advisor ruling 2026-11-08)
-- lint: texts pending 40823 offer/request texts from the user's texts file (advisor ruling 2026-11-08)
-- lint: texts pending 40824 offer/request texts from the user's texts file (advisor ruling 2026-11-08)
-- lint: z 11070002 the Command Map stands on the hall's main level (the portal arrival is 243-244; layers 223.50 237.58 248.21), to confirm in the final pass

DELETE FROM `quest_template_addon` WHERE `ID` IN (40731, 40821, 40823, 40824);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(40731, 256, 43254),
(40821, 256, 40731),
(40823, 256, 40821),
(40824, 256, 40823);

DELETE FROM `creature_queststarter` WHERE (`id`=101097 AND `quest` IN (40731, 40821, 40823)) OR (`id`=101921 AND `quest`=40824);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(101097, 40731, 0), -- Calydus: The Heart of the Dreadscar
(101097, 40821, 0), -- Calydus: Power Overwhelming
(101097, 40823, 0), -- Calydus: Rebuilding the Council
(101921, 40824, 0); -- Ritssyn Flamescowl: The Path of the Dreadscar

-- Scripts
UPDATE `creature_template` SET `ScriptName`='boss_jagganoth_heart_of_the_dreadscar' WHERE `entry`=101892;

-- Fork gameobject templates: the Circle's gateway into 998 (1220015), the Seat's altar on 1522 (1522001), the Felblood
-- Altar (1107001, an artifact forge: no condition, large AOI, the camera mode left unset) and the soul flame (1107002)
DELETE FROM `gameobject_template` WHERE `entry` IN (1220015, 1522001, 1107001, 1107002);
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `ScriptName`, `VerifiedBuild`) VALUES
(1220015, 10, 30007, 'Ritual Gateway', '', '', '', 1, 0, 0, 0, 0, 0, 0, 'go_dreadscar_heart_gateway', 0),
(1522001, 10, 27636, 'Altar of the Overlord', '', '', '', 1, 0, 0, 0, 0, 0, 0, 'go_seat_of_the_overlord_altar', 0),
(1107001, 47, 27636, 'Felblood Altar', '', '', '', 1, 0, 1, 0, 0, 15, 0, 'go_felblood_altar', 0),
(1107002, 10, 16656, 'Soul Flame', '', '', '', 1, 0, 0, 0, 0, 0, 0, 'go_ritssyn_soul_flame', 0);

-- 201205 Order Hall Transition: the hall by Calydus (the 40731 turn-in POI)
DELETE FROM `spell_target_position` WHERE `ID`=201205;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(201205, 0, 0, 1107, 3116.00, 1102.00, 286.16, 0.67, 0);

-- The hall, map 1107 (series 11070000 + n)
DELETE FROM `creature` WHERE `guid` IN (11070000, 11070001, 11070002);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(11070000, 101097, 1107, 0, 0, '0', 0, 0, 0, 0, 3121.00, 1106.00, 286.59, 3.81, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Calydus (the 40731/40821 turn-in POI)
(11070001, 101921, 1107, 0, 0, '0', 0, 0, 0, 0, 2984.00, 1158.00, 195.62, 5.50, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Ritssyn Flamescowl by his soul flame (the 40823 turn-in POI)
(11070002, 102589, 1107, 0, 0, '0', 0, 0, 64062, 0, 3033.00, 884.00, 248.21, 0, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Command Map (the 40824 objective POI; display 64062 as in the other halls)
UPDATE `creature` SET `ScriptName`='npc_acherus_command_map' WHERE `guid`=11070002; -- scale 2, not the template's 8
DELETE FROM `gameobject` WHERE `guid` IN (11070000, 11070001, 11070002);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(11070000, 1107001, 1107, 0, 0, '0', 0, 0, 0, -1, 3128.00, 1112.00, 286.54, 0, 0, 0, 0, 1, 120, 255, 1, '', NULL, 0), -- Felblood Altar, by Calydus
(11070001, 1107002, 1107, 0, 0, '0', 0, 0, 0, -1, 2992.00, 1150.00, 194.80, 0, 0, 0, 0, 1, 120, 255, 1, '', NULL, 0), -- Ritssyn's soul flame (the 40823 objective POI)
(11070002, 251313, 1107, 0, 0, '0', 0, 0, 0, -1, 3093.00, 846.00, 243.81, 0, 0, 0, 0, 1, 120, 255, 1, '', NULL, 0); -- Portal to Dalaran (192241, to Krasus' Landing), by the hall's arrival

-- Scenario 998 on map 1522 (series 15220000 + n): Jagganoth where 962's stands, and the Seat's altar
DELETE FROM `creature` WHERE `guid`=15220010;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(15220010, 101892, 1522, 0, 0, '12', 0, 0, 0, 0, 3100.00, 880.00, 248.94, 4.47, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Jagganoth, facing the Finale's front entrance
DELETE FROM `gameobject` WHERE `guid`=15220000;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15220000, 1522001, 1522, 0, 0, '12', 0, 0, 0, -1, 3121.00, 1106.00, 286.47, 0, 0, 0, 0, 1, 86400, 255, 1, '', NULL, 0); -- the altar in the Seat of the Overlord

-- Dalaran, the Circle of Wills (series 12205000 + n)
DELETE FROM `gameobject` WHERE `guid` IN (12205005, 12205006);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12205005, 1220015, 1220, 0, 0, '0', 0, 0, 0, -1, -862.00, 4470.00, 696.75, 5.30, 0, 0, 0.472031, -0.881582, 120, 255, 1, '', NULL, 0), -- Ritual Gateway, where the S2 ritual opens it
(12205006, 251302, 1220, 0, 0, '0', 0, 0, 0, -1, -866.00, 4442.00, 696.88, 0, 0, 0, 0, 1, 120, 255, 1, '', NULL, 0); -- Portal to Dreadscar Rift (215793)

-- Visibility by entry
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup` IN (3, 5) AND `SourceEntry` IN (101892, 101921, 1220015, 251302, 98771, 98836, 99147, 99145, 99146, 101460, 98873, 98834, 98831);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 101892, 0, 0, 47, 0, 40731, 8, 0, 0, 'Jagganoth (998): The Heart of the Dreadscar incomplete'),
(32, 3, 101921, 0, 0, 47, 0, 40823, 66, 0, 0, 'Ritssyn Flamescowl (hall): raised, Rebuilding the Council complete or rewarded'),
(32, 5, 1220015, 0, 0, 47, 0, 40731, 8, 0, 0, 'Ritual Gateway (Circle): The Heart of the Dreadscar incomplete'),
(32, 5, 251302, 0, 0, 47, 0, 40731, 64, 0, 0, 'Portal to Dreadscar Rift (Circle): The Heart of the Dreadscar rewarded'),
(32, 3, 98771, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Ritssyn prisoner: not in 998'),
(32, 3, 98836, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Beshtal: not in 998'),
(32, 3, 99147, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Jubeka captive: not in 998'),
(32, 3, 99145, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Zinnin captive: not in 998'),
(32, 3, 99146, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Shinfel captive: not in 998'),
(32, 3, 101460, 0, 0, 47, 0, 40731, 8, 0, 1, '962 spawn 101460: not in 998'),
(32, 3, 98873, 0, 0, 47, 0, 40731, 8, 0, 1, '962 spawn 98873: not in 998'),
(32, 3, 98834, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Xel''toth: not in 998'),
(32, 3, 98831, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Jagganoth: not in 998 (101892 is its boss)');
