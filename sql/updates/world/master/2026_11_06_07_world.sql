-- Acherus, the hall opening (Task 4): Keeping Your Edge, Advanced Runecarving, Plans and Preparations, Our Next Move.
--
-- Floors by the quest POIs (WorldMapArea 1021) and the navigation mesh:
--   - The lowest deck (z about 260): Mograine, Illanna, Siouxsie, the Command Map.
--   - The upper level (z about 313.5; the 299 layer below it reads as underground in the client): Corvus and the Soul
--     Forge. Heights from Claudk standing there (2026-11-06).
--   - The teleporter's deck end is 39757's objective POI (-1464, 1124, z 270.5).
-- Order (user): Illanna gives 39757; the teleporter up credits it; Corvus ends it and offers 39761; the forge; Corvus
-- ends 39761 and offers 39832 straight away (its RewardNextQuest; 39832's start POI at Mograine is not used); Siouxsie
-- ends 39832 and gives 39799 at the Command Map.

-- Chain and class: none of these has a quest_template_addon row in the data
DELETE FROM `quest_template_addon` WHERE `ID` IN (39757, 39761, 39832, 39799);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(39757, 32, 0),
(39761, 32, 39757),
(39832, 32, 39761),
(39799, 32, 39832);

-- Givers (fork data: the data gives these quests no starter). Corvus offers each as the one before it is turned in
DELETE FROM `creature_queststarter` WHERE (`id`=97072 AND `quest` IN (39761, 39832)) OR (`id`=93568 AND `quest`=39799);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(97072, 39761, 0), -- Grand Master Siegesmith Corvus: Advanced Runecarving
(97072, 39832, 0), -- Grand Master Siegesmith Corvus: Plans and Preparations
(93568, 39799, 0); -- Siouxsie the Banshee: Our Next Move

-- Corvus at his POI on the upper level, facing the forge; Siouxsie at hers on the deck, facing the map; the Command Map
-- (102589, as in the priest's and the demon hunter's halls) at 39799's objective POI
DELETE FROM `creature` WHERE `guid` IN (12207003, 12207004, 12207005);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12207003, 97072, 1220, 0, 0, '0', 0, 0, 0, 0, -1536.62, 1009.29, 313.65, 4.46, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Grand Master Siegesmith Corvus
(12207004, 93568, 1220, 0, 0, '0', 0, 0, 0, 0, -1509.00, 1054.00, 260.51, 5.03, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Siouxsie the Banshee
(12207005, 102589, 1220, 0, 0, '0', 0, 0, 64062, 0, -1508.06, 1051.40, 260.54, 0, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Command Map (display 64062, as in the DH and priest halls; the template's other model 64074 does not show the table)

-- The Command Map at scale 2 (npc_acherus_command_map): at the template's 8 its sheen shows through the hall's mist
UPDATE `creature` SET `ScriptName`='npc_acherus_command_map' WHERE `guid`=12207005;

-- The teleporter: one object at each end (go_acherus_hall_teleporter). Placeholder look: the old Acherus teleport
-- rune (display 6788, from 190566); fork data
DELETE FROM `gameobject_template` WHERE `entry`=1220002;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `ScriptName`, `VerifiedBuild`) VALUES
(1220002, 10, 6788, 'Acherus Teleporter', '', '', '', 1, 'go_acherus_hall_teleporter', 0);

UPDATE `gameobject_template` SET `ScriptName`='go_acherus_soul_forge' WHERE `entry`=245433;

DELETE FROM `gameobject` WHERE `guid` IN (12207000, 12207001, 12207002);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12207000, 1220002, 1220, 0, 0, '0', 0, 0, 0, -1, -1464.00, 1124.00, 270.50, 0, 0, 0, 0, 1, 120, 255, 1, '', NULL, 0), -- teleporter, deck end
(12207001, 1220002, 1220, 0, 0, '0', 0, 0, 0, -1, -1460.06, 1130.99, 314.70, 0, 0, 0, 0, 1, 120, 255, 1, '', NULL, 0), -- teleporter, upper end
(12207002, 245433, 1220, 0, 0, '0', 0, 0, 0, -1, -1539.00, 1000.00, 313.65, 1.0287, 0, 0, 0.491960, 0.870618, 120, 255, 1, '', NULL, 0); -- Soul Forge, facing the upper rune

-- The Lich King's line on the waygate landing in Acherus (line 1794, female 1795): talking heads of ours, the same
-- actor as the scenario's (14800008-11)
DELETE FROM `conversation_template` WHERE `Id` IN (14800012, 14800013);
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(14800012, 1794, 7000, 0, '', 0),
(14800013, 1795, 7000, 0, '', 0);
DELETE FROM `conversation_actors` WHERE `ConversationId` IN (14800012, 14800013);
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(14800012, 14800001, 0, 0, 99456, 64642, 0, 0, 0),
(14800013, 14800001, 0, 0, 99456, 64642, 0, 0, 0);
DELETE FROM `conversation_line_template` WHERE `Id` IN (1794, 1795);
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(1794, 0, 296, 0, 0, 0),
(1795, 0, 296, 0, 0, 0);

-- Turn-in and progress text: text from Wowhead (the retail quest pages). The data has no quest_offer_reward or
-- quest_request_items rows for these quests; 38990 (The Call of Icecrown, Task 3) lacked them too
DELETE FROM `quest_offer_reward` WHERE `ID` IN (38990, 39757, 39761, 39832, 39799);
INSERT INTO `quest_offer_reward` (`ID`, `RewardText`, `VerifiedBuild`) VALUES
(38990, 'The Lich King made a wise choice in giving you those blades, $n. You''ve more then proven yourself worthy of them.$B$BI have to admit though, I''m glad it wasn''t me, I''m through with having magical weapons dictate my fate.', 0),
(39757, 'Ah this weapon is amazing! Do you see the souls of the damned dancing along its edge? Oh how they scream for release!$B$BYes... in their forms I can see how the blade must be shaped.', 0),
(39761, 'Oh yes... that will do nicely!$B$BReturn after you''ve fed your blade another feast of souls and we will make it the most powerful weapon that''s ever existed!', 0),
(39832, 'Deathlord, I''ve prepared a brief report on all we''ve discovered about this new land.$B$BYou should find it quite interesting.', 0),
(39799, 'A fine choice, Deathlord.', 0);
DELETE FROM `quest_request_items` WHERE `ID`=38990;
INSERT INTO `quest_request_items` (`ID`, `CompletionText`, `VerifiedBuild`) VALUES
(38990, 'It is time to begin, $n.', 0);

-- The hall's mission table under the Command Map, as in the priest's and the demon hunter's halls: its model,
-- 7af_deathknight_missiontable01.m2 (the client's display 40015), has no object in the data, so it is given one;
-- GO_FLAG_NOT_SELECTABLE and GO_FLAG_NODESPAWN, so that pointing at the table finds the Command Map; fork data
DELETE FROM `gameobject_template` WHERE `entry`=1220003;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `VerifiedBuild`) VALUES
(1220003, 5, 40015, 'Command Map', 1, 0);
DELETE FROM `gameobject_template_addon` WHERE `entry`=1220003;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`) VALUES
(1220003, 0, 0x30);
DELETE FROM `gameobject` WHERE `guid`=12207003;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12207003, 1220003, 1220, 0, 0, '0', 0, 0, 0, -1, -1508.06, 1051.40, 260.54, 0, 0, 0, 0, 1, 300, 255, 1, '', NULL, 0); -- Command Map table, at the centre of the deck's ring (a 9.2 yd circle walked by hand)
