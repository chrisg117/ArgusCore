-- DK class hall campaign, slice 4 (QuestLine 232, chapter 2): The Ruined Kingdom to The King Rises, in Stromgarde.
-- Staged from Wowhead's DK campaign videos (22N8zGe2w0k, 3Dw07oOIHO0, la8kcbjP7BY, VxnWYXfWuAI, hLq_bjtdKY8): map
-- coordinates read off the videos and turned into world positions through Arathi's WorldMapArea bounds, heights from the
-- navmesh, and the floors of the keep, the chapel and the crypt from their WMOs (vmaps). The death knight travels to Stromgarde the normal way. State masks: 8 in progress, 2 complete, 64 rewarded.

DELETE FROM `quest_template_addon` WHERE `ID` IN (42533, 42534, 42535, 42536, 42537);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(42533, 32, 43539),
(42534, 32, 42533),
(42535, 32, 42533),
(42536, 32, 42534),
(42537, 32, 42536);

-- Regicide also waits for Death... and Decay: the two of Galen's tasks are done side by side
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry`=42536;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 42536, 0, 0, 8, 0, 42535, 0, 0, 0, 'Regicide: after Death... and Decay as well');

-- The data names no giver but the first quest's point on Mograine. On the videos Thassarian gives Regicide in Galen's hall
-- and takes it at the crypt, where he gives The King Rises; the crypt Thassarian is the data's ender (108042), so the
-- hall one is another of his entries (108436), as visibility goes by entry
DELETE FROM `creature_queststarter` WHERE `quest` IN (42533, 42534, 42535, 42536, 42537);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(93437, 42533, 0),  -- Mograine (deck): The Ruined Kingdom
(107806, 42534, 0), -- Galen: Our Oldest Enemies (also offered on The Ruined Kingdom's turn-in)
(107806, 42535, 0), -- Galen: Death... and Decay
(108436, 42536, 0), -- Thassarian (Galen's hall): Regicide
(108042, 42537, 0); -- Thassarian (the crypt): The King Rises

UPDATE `creature_template` SET `npcflag`=`npcflag`|2 WHERE `entry`=108436;

-- 214956 is not in Spell.db2: the core would log an error at the turn-in and give nothing
UPDATE `quest_template` SET `RewardSpell`=0 WHERE `ID` IN (42534, 42535);

-- The Witherbark in Stromgarde Keep and their chieftess are hostile, as Arathi's own Witherbark (654), and scale as the
-- Broken Isles' 98-110 creatures do (level 102 elite chieftess on the video). Galen turns on the death knight in
-- Regicide: his fight form (108044) is hostile and carries his father's sword
UPDATE `creature_template` SET `faction`=654 WHERE `entry` IN (107792, 107793, 107794, 107795, 108236, 108044);
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `Entry` IN (107792, 107793, 107794, 107795, 108236, 108044) AND `DifficultyID`=0;

UPDATE `creature_template_difficulty` SET `LootID`=`Entry` WHERE `Entry` IN (108236, 108044) AND `DifficultyID`=0;
DELETE FROM `creature_loot_template` WHERE (`Entry`=108236 AND `Item`=138142) OR (`Entry`=108044 AND `Item`=138148);
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(108236, 0, 138142, 100, 1, 1, 0, 1, 1, 'Ojin''ba: Head of Ojin''ba (Our Oldest Enemies)'),
(108044, 0, 138148, 100, 1, 1, 0, 1, 1, 'Prince Galen Trollbane: Trol''kalar (Regicide)');

-- Spawns (map 0). Galen's hall: 19.48, 67.18 (Ruined Kingdom 1:35) and 19.62, 67.11 (Regicide 0:10). The camp: the
-- objective hulls' points that lie on the courtyard mesh, and the spots read at Our Oldest Enemies 1:00-1:40 (Ojin'ba at
-- 11.84, 69.18). Galen's fight: The Sanctum (the chapel), 21.80, 59.73 (Regicide 0:56). The crypt: 23.34, 61.38 (Thassarian,
-- Regicide 1:58); the tomb room 22.58, 61.47 (The King Rises 0:25-0:38), by Trollbane's Tomb (203450)
DELETE FROM `creature` WHERE `guid` BETWEEN 10000000 AND 10000024;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(10000000, 107806, 0, 0, 0, '0', 0, 0, 0, 0, -1707.30, -1807.50, 84.31, 0.00, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Prince Galen Trollbane, his hall
(10000001, 108436, 0, 0, 0, '0', 0, 0, 0, 0, -1697.80, -1809.30, 84.31, 3.14, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Thassarian, Galen's hall
(10000002, 108236, 0, 0, 0, '0', 0, 0, 0, 0, -1745.80, -1538.80, 61.64, 0.00, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Ojin'ba
(10000003, 107792, 0, 0, 0, '0', 0, 0, 0, 0, -1705.00, -1739.00, 55.08, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000004, 107793, 0, 0, 0, '0', 0, 0, 0, 0, -1717.00, -1727.00, 53.00, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000005, 107794, 0, 0, 0, '0', 0, 0, 0, 0, -1790.00, -1626.00, 53.31, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000006, 107795, 0, 0, 0, '0', 0, 0, 0, 0, -1794.00, -1558.00, 53.20, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000007, 107792, 0, 0, 0, '0', 0, 0, 0, 0, -1794.00, -1529.00, 61.20, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000009, 107794, 0, 0, 0, '0', 0, 0, 0, 0, -1729.00, -1493.00, 66.40, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000010, 107795, 0, 0, 0, '0', 0, 0, 0, 0, -1669.00, -1534.00, 68.13, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000011, 107792, 0, 0, 0, '0', 0, 0, 0, 0, -1657.00, -1550.00, 67.89, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000012, 107793, 0, 0, 0, '0', 0, 0, 0, 0, -1705.90, -1552.70, 55.82, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000013, 107794, 0, 0, 0, '0', 0, 0, 0, 0, -1720.30, -1542.90, 56.17, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000014, 107795, 0, 0, 0, '0', 0, 0, 0, 0, -1740.00, -1560.00, 54.68, 0.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(10000015, 108044, 0, 0, 0, '0', 0, 0, 0, 0, -1526.60, -1885.10, 69.55, 0.00, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Prince Galen Trollbane, The Sanctum
(10000016, 108042, 0, 0, 0, '0', 0, 0, 0, 0, -1565.00, -1938.60, 53.55, 1.57, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Thassarian, the crypt
(10000017, 108260, 0, 0, 0, '0', 0, 0, 0, 0, -1569.60, -1916.40, 47.15, 0.00, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Mausoleum entered (credit bunny)
(10000018, 108396, 0, 0, 0, '0', 0, 0, 0, 0, -1564.00, -1906.00, 48.29, 4.73, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- King Thoras Trollbane, in his tomb
(10000019, 109000, 0, 0, 0, '0', 0, 0, 0, 0, -1564.00, -1906.00, 48.29, 4.73, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- King Thoras Trollbane, raised

DELETE FROM `creature_addon` WHERE `guid`=10000018;
INSERT INTO `creature_addon` (`guid`, `StandState`) VALUES
(10000018, 7); -- dead

-- Who sees whom: Galen and his hall's Thassarian until Regicide is taken; the camp after The Ruined Kingdom; Galen's fight
-- form during Regicide; the crypt from Regicide on; Thoras's body until he is raised, and Thoras raised until the turn-in
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (107806, 108042, 108436, 107792, 107793, 107794, 107795, 108236, 108044, 108260, 108396, 109000);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 107806, 0, 0, 47, 0, 42533, 74, 0, 0, 'Prince Galen Trollbane: from The Ruined Kingdom on'),
(32, 3, 107806, 0, 0, 47, 0, 42536, 74, 0, 1, 'Prince Galen Trollbane: until Regicide'),
(32, 3, 108436, 0, 0, 47, 0, 42533, 74, 0, 0, 'Thassarian (Galen''s hall): from The Ruined Kingdom on'),
(32, 3, 108436, 0, 0, 47, 0, 42536, 74, 0, 1, 'Thassarian (Galen''s hall): until Regicide'),
(32, 3, 107792, 0, 0, 8, 0, 42533, 0, 0, 0, 'Witherbark Witch Doctor: after The Ruined Kingdom'),
(32, 3, 107793, 0, 0, 8, 0, 42533, 0, 0, 0, 'Witherbark Headhunter: after The Ruined Kingdom'),
(32, 3, 107794, 0, 0, 8, 0, 42533, 0, 0, 0, 'Witherbark Axe Thrower: after The Ruined Kingdom'),
(32, 3, 107795, 0, 0, 8, 0, 42533, 0, 0, 0, 'Witherbark Bloodrager: after The Ruined Kingdom'),
(32, 3, 108236, 0, 0, 8, 0, 42533, 0, 0, 0, 'Ojin''ba: after The Ruined Kingdom'),
(32, 3, 108044, 0, 0, 47, 0, 42536, 8, 0, 0, 'Prince Galen Trollbane (The Sanctum): during Regicide'),
(32, 3, 108042, 0, 0, 47, 0, 42536, 74, 0, 0, 'Thassarian (the crypt): from Regicide on'),
(32, 3, 108396, 0, 0, 47, 0, 42536, 74, 0, 0, 'Thoras''s body: from Regicide on'),
(32, 3, 108396, 0, 0, 47, 0, 42537, 66, 0, 1, 'Thoras''s body: until he is raised'),
(32, 3, 109000, 0, 0, 47, 0, 42537, 2, 0, 0, 'King Thoras Trollbane: raised, until the turn-in');

-- Mausoleum entered: the tomb room's bunny credits whoever comes within 10 yd while The King Rises is open (no client
-- areatrigger lies there)
UPDATE `creature_template` SET `AIName`='SmartAI', `unit_flags`=`unit_flags`|33554432 WHERE `entry`=108260;
DELETE FROM `smart_scripts` WHERE `entryorguid`=108260 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `target_type`, `comment`) VALUES
(108260, 0, 0, 0, 10, 2, 10, 1000, 1000, 1, 33, 108260, 7, 'Mausoleum entered: on a player within 10 yd, credit them');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=1 AND `SourceEntry`=108260;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(22, 1, 108260, 0, 0, 47, 0, 42537, 8, 0, 0, 'Mausoleum entered: only during The King Rises');

-- The raise, as Nazgrim's (2026_11_07_01): while The King Rises is in progress in Arathi the player holds Raise Horseman -
-- Aura (220847), whose override bar 755 the hotfix of 2026_11_08_01_hotfixes.sql points at the button 215173. The button
-- casts Raise Horseman (215174, 10 s, the player stands still), which summons the raise's anchor (108384, the invisible
-- bunny) on the tomb and triggers the credit (215175, Trollbane raised 108261). The objective is sequenced after the tomb
-- is entered. The body is no longer clicked
UPDATE `creature_template` SET `npcflag`=`npcflag`&~16777216 WHERE `entry`=108396;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=108396;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=108396;
DELETE FROM `spell_area` WHERE `spell`=220847;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `flags`, `quest_start_status`, `quest_end_status`) VALUES
(220847, 45, 42537, 42537, 0, 0, 2, 3, 8, 8); -- Raise Horseman - Aura: in Arathi while The King Rises is in progress
DELETE FROM `spell_target_position` WHERE `ID`=215174;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(215174, 0, 0, 0, -1564.00, -1906.00, 48.29, 4.73, 0);

-- Thoras and Thassarian speak once he rises (The King Rises 0:50-1:14): the client's own lines 5236 (a silent 6 s lead-in) and 5224-5231, started by the
-- raise's anchor when it is summoned. Thoras speaks the even turns, Thassarian the replies
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=108384;
DELETE FROM `smart_scripts` WHERE `entryorguid`=108384 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `target_type`, `comment`) VALUES
(108384, 0, 0, 0, 54, 0, 0, 0, 0, 0, 143, 12207100, 23, 'Trollbane Vehicle: on summon, Thoras and Thassarian speak to the raiser');

DELETE FROM `conversation_template` WHERE `Id`=12207100;
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(12207100, 5236, 42000, 0, '', 0);

DELETE FROM `conversation_actors` WHERE `ConversationId`=12207100;
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(12207100, 12207100, 10000019, 0, 0, 0, 0, 0, 0), -- King Thoras Trollbane, raised
(12207100, 12207101, 10000016, 1, 0, 0, 0, 0, 0); -- Thassarian, the crypt

DELETE FROM `conversation_line_template` WHERE `Id` IN (5236, 5224, 5225, 5226, 5227, 5228, 5229, 5230, 5231);
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(5236, 0, 0, 0, 0, 0),     -- (no text: 6 s while he rises; the client chain starts here)
(5224, 6000, 0, 0, 0, 0),  -- My son. Where is Galen?
(5225, 10500, 0, 0, 0, 0),  -- Where is the prince who murdered his king?
(5226, 15000, 0, 1, 0, 0),  -- We took his life, my lord. The prince left us little choice.
(5227, 20000, 0, 0, 0, 0), -- What has become of Stromgarde?
(5228, 24000, 0, 1, 0, 0), -- Death and ruin. Time has not been kind to this place.
(5229, 29000, 0, 0, 0, 0), -- Then I have failed as both a father and a king.
(5230, 33500, 0, 1, 0, 0), -- Stromgarde is lost, but there is still hope for this world... if you will join us.
(5231, 38500, 0, 0, 0, 0); -- I will do whatever it takes to preserve peace on Azeroth.

-- Witherbark Supplies (250650): no template in the data or the client. A goober for Death... and Decay, opened with a short
-- cast as on the video (Death... and Decay 1:20, lock 1691 as other supplies' "Recovering"), consumed on use so one crate
-- counts once; the purple blight after use is not reproduced. Display: candidate lineup at the pass (31, 9105, 1708);
-- 31 until the user picks. Spots: the objective hull's points on the courtyard mesh, and the video's
DELETE FROM `gameobject_template` WHERE `entry`=250650;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `Data24`, `Data25`, `Data26`, `Data27`, `Data28`, `Data29`, `Data30`, `Data31`, `Data32`, `Data33`, `RequiredLevel`, `AIName`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(250650, 10, 31, 'Witherbark Supplies', 'interact', 'Defiling', '', 1, 1691, 42535, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', NULL, 0);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 10000000 AND 10000009;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(10000000, 250650, 0, 0, 0, '0', 0, 0, -1726.00, -1744.00, 52.81, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(10000001, 250650, 0, 0, 0, '0', 0, 0, -1712.00, -1704.00, 54.13, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(10000002, 250650, 0, 0, 0, '0', 0, 0, -1691.00, -1639.00, 59.79, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(10000003, 250650, 0, 0, 0, '0', 0, 0, -1687.00, -1574.00, 55.78, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(10000004, 250650, 0, 0, 0, '0', 0, 0, -1751.00, -1503.00, 61.20, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(10000005, 250650, 0, 0, 0, '0', 0, 0, -1800.00, -1553.00, 53.66, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(10000006, 250650, 0, 0, 0, '0', 0, 0, -1771.00, -1651.00, 53.87, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(10000007, 250650, 0, 0, 0, '0', 0, 0, -1735.00, -1543.30, 56.30, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(10000008, 250650, 0, 0, 0, '0', 0, 0, -1705.90, -1631.60, 59.50, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(10000009, 250650, 0, 0, 0, '0', 0, 0, -1700.00, -1660.00, 59.71, 0, 0, 0, 0, 1, 30, 255, 1, 0);

-- Texts, from Wowhead's Progress and Completion sections
DELETE FROM `quest_offer_reward` WHERE `ID` IN (42533, 42534, 42535, 42536, 42537);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(42533, 1, 0, 0, 0, 0, 0, 0, 0, 'For what purpose do you intrude upon my kingdom, $c? Speak quickly, for I have little patience...', 0),
(42534, 1, 0, 0, 0, 0, 0, 0, 0, 'Soon the heads of my enemies shall line the walls of my kingdom! These shall be the first of many.', 0),
(42535, 1, 0, 0, 0, 0, 0, 0, 0, 'You have done my kingdom a service, $c. Stromgarde will not forget the service you have done us this day.', 0),
(42536, 1, 0, 0, 0, 0, 0, 0, 0, 'We have tarried here long enough, Deathlord. The time has come to open the tomb and raise our champion.', 0),
(42537, 1, 0, 0, 0, 0, 0, 0, 0, 'Today, a great king has risen from the dead to once more take up the sword in the name of liberty.$B$BIt will be an honor to fight at his side against the legions of terror.', 0);

DELETE FROM `quest_request_items` WHERE `ID` IN (42534, 42536, 42537);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `EmoteOnCompleteDelay`, `EmoteOnIncompleteDelay`, `CompletionText`, `VerifiedBuild`) VALUES
(42534, 6, 6, 0, 0, 'Do you have the heads of my enemies? I want to look upon each and every face...', 0),
(42536, 6, 6, 0, 0, 'We need the sword Trol''kalar to open this tomb. Have you found it?', 0),
(42537, 6, 6, 0, 0, 'King Trollbane was a great man. He will be a powerful ally against the threat of the Burning Legion.', 0);

-- Regicide has no map points in the data, so nothing showed where Galen waits (pass 2026-11-08). Fork points in the shape
-- of The King Rises' own rows (Arathi, WorldMapArea 16): Trol'kalar at Galen in The Sanctum chapel, the turn-in at the
-- crypt Thassarian
DELETE FROM `quest_poi` WHERE `QuestID`=42536;
INSERT INTO `quest_poi` (`QuestID`, `BlobIndex`, `Idx1`, `ObjectiveIndex`, `QuestObjectiveID`, `QuestObjectID`, `MapID`, `WorldMapAreaId`, `Floor`, `Priority`, `Flags`, `WorldEffectID`, `PlayerConditionID`, `SpawnTrackingID`, `AlwaysAllowMergingBlobs`, `VerifiedBuild`) VALUES
(42536, 0, 0, -1, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0),
(42536, 0, 1, 0, 284494, 138148, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM `quest_poi_points` WHERE `QuestID`=42536;
INSERT INTO `quest_poi_points` (`QuestID`, `Idx1`, `Idx2`, `X`, `Y`, `VerifiedBuild`) VALUES
(42536, 0, 0, -1565, -1939, 0),
(42536, 1, 0, -1527, -1885, 0);
