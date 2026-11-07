-- DK class hall campaign, slice 1 (QuestLine 232): Return of the Four Horsemen, The Firstborn Rises, Rise, Champions,
-- the first champions (Thassarian, Nazgrim) and Spread the Word.
-- Givers and enders from Wowhead's quest pages; the data has no giver for any of them, so every queststarter row here is
-- fork data. Champion: Nazgrim is ended by Nazgrim (109890) on Wowhead, by Koltira (93453) in the data: Wowhead's is kept.
-- Visibility conditions go by entry. State masks: 8 in progress, 2 complete, 64 rewarded.

DELETE FROM `quest_template_addon` WHERE `ID` IN (42449, 42484, 43264, 39816, 39818, 43265);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(42449, 32, 39799),
(42484, 32, 42449),
(43264, 32, 42484),
(39816, 32, 0),
(39818, 32, 0),
(43265, 32, 43264);

-- 225995 is not in Spell.db2: the core would log an error at the turn-in and give nothing
UPDATE `quest_template` SET `RewardSpell`=0 WHERE `ID`=43264;

DELETE FROM `creature_queststarter` WHERE `quest` IN (42449, 42484, 43264, 39816, 39818, 43265);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(93437, 42449, 0),  -- Highlord Darion Mograine: Return of the Four Horsemen
(107554, 42484, 0), -- Thassarian (Durotar): The Firstborn Rises
(93437, 43264, 0),  -- Mograine: Rise, Champions
(93456, 39816, 0),  -- Thassarian: Champion: Thassarian
(109890, 39818, 0), -- Nazgrim: Champion: Nazgrim
(93568, 43265, 0);  -- Siouxsie: Spread the Word (also offered on Rise, Champions' turn-in)

DELETE FROM `creature_questender` WHERE `quest`=39818;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(109890, 39818, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (39816, 39818);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 39816, 0, 0, 47, 0, 43264, 8, 0, 0, 'Champion: Thassarian: during Rise, Champions'),
(19, 0, 39818, 0, 0, 47, 0, 43264, 8, 0, 0, 'Champion: Nazgrim: during Rise, Champions');

-- Durotar: Thassarian waits at the 42449 turn-in point; Nazgrim's body lies at the 42484 objective point, and once he is
-- raised (42484 complete) the body gives way to Nazgrim standing. The Acherus champions stand on the ring at 270 by the
-- Rise, Champions points (-1445,1024 and -1562,1102), on the nearest walkable mesh. Heights from the navmesh.
DELETE FROM `creature` WHERE `guid` IN (10010000, 10010001, 10010002, 12207006, 12207007);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(10010000, 107554, 1, 0, 0, '0', 0, 0, 0, 0, 1185.00, -4464.00, 21.41, 3.70, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Thassarian
(10010001, 109886, 1, 0, 0, '0', 0, 0, 0, 0, 1179.00, -4468.00, 21.76, 0.60, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Nazgrim's body
(10010002, 109890, 1, 0, 0, '0', 0, 0, 0, 0, 1179.00, -4468.00, 21.76, 0.60, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Nazgrim, raised
(12207006, 93456, 1220, 0, 0, '0', 0, 0, 0, 0, -1446.00, 1024.00, 270.07, 2.75, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Thassarian, champion
(12207007, 109890, 1220, 0, 0, '0', 0, 0, 0, 0, -1562.00, 1100.00, 270.90, 5.50, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Nazgrim, champion

DELETE FROM `creature_addon` WHERE `guid`=10010001;
INSERT INTO `creature_addon` (`guid`, `StandState`) VALUES
(10010001, 7); -- dead

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (107554, 109886, 109890, 93456);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 107554, 0, 0, 47, 0, 42449, 74, 0, 0, 'Thassarian in Durotar: from Return of the Four Horsemen on'),
(32, 3, 109886, 0, 0, 47, 0, 42449, 74, 0, 0, 'Nazgrim''s body: from Return of the Four Horsemen on'),
(32, 3, 109886, 0, 0, 47, 0, 42484, 66, 0, 1, 'Nazgrim''s body: until he is raised'),
(32, 3, 109890, 0, 0, 47, 0, 42484, 66, 0, 0, 'Nazgrim: once raised'),
(32, 3, 93456, 0, 0, 47, 0, 43264, 74, 0, 0, 'Thassarian the champion: from Rise, Champions on');

-- The raise: clicking the body casts Raise Horseman (213633, 10 s, the player stands still), which summons the raise's
-- anchor (107597, an invisible bunny) on the body and triggers the credit (213679, Nazgrim Raised 107719)
UPDATE `creature_template` SET `npcflag`=`npcflag`|16777216 WHERE `entry`=109886;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=109886;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(109886, 213633, 3, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=109886;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(18, 109886, 213633, 0, 0, 47, 0, 42484, 8, 0, 0, 'Raise Horseman: during The Firstborn Rises');
DELETE FROM `spell_target_position` WHERE `ID`=213633;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(213633, 0, 0, 1, 1179.00, -4468.00, 21.76, 0.60, 0);
