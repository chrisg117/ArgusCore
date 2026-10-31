-- Azurewing Repose: Journey to the Repose, The Death of the Eldest, Agapanthus and Their Dying Breaths. Nothing here was
-- sniffed. Everyone stands at the quests' POIs, heights read in game, positions rough.
-- Journey to the Repose starts from the Khadgar at Illidari Stand (93337), not the one it ends at (93354), or he would take
-- it back as soon as he gave it.
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(38443, 37449),
(37853, 38443),
(37991, 37853),
(42271, 37991)
ON DUPLICATE KEY UPDATE `PrevQuestID`=VALUES(`PrevQuestID`);

DELETE FROM `creature_queststarter` WHERE (`id`=93337 AND `quest`=38443) OR (`id`=93354 AND `quest`=37853) OR (`id`=90423 AND `quest`=37991) OR (`id`=90543 AND `quest`=42271);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(93337, 38443, 0),
(93354, 37853, 0),
(90423, 37991, 0),
(90543, 42271, 0);

-- Leyworms in Leyhollow carry the cores
UPDATE `creature_template` SET `faction`=2856 WHERE `entry`=106630;
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110, `LootID`=106630 WHERE `Entry`=106630 AND `DifficultyID`=0;
DELETE FROM `creature_loot_template` WHERE `Entry`=106630 AND `Item`=122095;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(106630, 0, 122095, 100, 1, 1, 0, 1, 1, 'Burrowing Leyworm - Crackling Leyworm Core');

UPDATE `creature_template` SET `ScriptName`='npc_death_of_the_eldest_senegos' WHERE `entry`=89975;
UPDATE `creature_template` SET `ScriptName`='npc_agapanthus' WHERE `entry`=90543;

-- Clicking a whelpling casts Resuscitating, once the player is on the quest
UPDATE `creature_template` SET `npcflag`=`npcflag`|16777216, `ScriptName`='npc_their_dying_breaths_whelpling' WHERE `entry`=90880;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=90880;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(90880, 180713, 1, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=90880 AND `SourceEntry`=180713;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(18, 90880, 180713, 0, 0, 9, 0, 42271, 0, 0, 0, 'Mana-Drained Whelpling - Resuscitating while on Their Dying Breaths');

-- The Cracked Ley Crystals had a quest item row but no loot
UPDATE `gameobject_template` SET `Data1`=240267, `Data3`=0 WHERE `entry`=240267;
DELETE FROM `gameobject_loot_template` WHERE `Entry`=240267 AND `Item`=122306;
INSERT INTO `gameobject_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(240267, 0, 122306, 100, 1, 1, 0, 1, 1, 'Cracked Ley Crystal');

DELETE FROM `creature` WHERE `guid` BETWEEN 12202001 AND 12202030;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12202001, 93337, 1220, 7334, 7364, '0', 0, 0, 0, 0, -109.00, 6985.00, 12.12, 4.71, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Journey to the Repose giver
(12202002, 93354, 1220, 7334, 7340, '0', 0, 0, 0, 0, 612.00, 6677.00, 60.20, 4.71, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- its ender
(12202003, 89794, 1220, 7334, 7340, '0', 0, 0, 0, 0, 613.00, 6667.00, 60.27, 4.71, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- The Death of the Eldest ender
(12202004, 90423, 1220, 7334, 7340, '0', 4476, 0, 0, 0, 618.00, 6660.00, 60.30, 1.57, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Agapanthus giver
(12202011, 106630, 1220, 7334, 7943, '0', 0, 0, 0, 0, 790.00, 6520.00, 59.29, 0, 60, 3, 0, 1, NULL, NULL, NULL, NULL, 0),
(12202012, 106630, 1220, 7334, 7943, '0', 0, 0, 0, 0, 805.00, 6490.00, 57.41, 0, 60, 3, 0, 1, NULL, NULL, NULL, NULL, 0),
(12202013, 106630, 1220, 7334, 7943, '0', 0, 0, 0, 0, 770.00, 6490.00, 61.19, 0, 60, 3, 0, 1, NULL, NULL, NULL, NULL, 0),
(12202014, 106630, 1220, 7334, 7943, '0', 0, 0, 0, 0, 820.00, 6530.00, 56.80, 0, 60, 3, 0, 1, NULL, NULL, NULL, NULL, 0),
(12202015, 106630, 1220, 7334, 7943, '0', 0, 0, 0, 0, 760.00, 6520.00, 65.94, 0, 60, 3, 0, 1, NULL, NULL, NULL, NULL, 0),
(12202016, 106630, 1220, 7334, 7943, '0', 0, 0, 0, 0, 835.00, 6510.00, 55.25, 0, 60, 3, 0, 1, NULL, NULL, NULL, NULL, 0),
(12202017, 106630, 1220, 7334, 7943, '0', 0, 0, 0, 0, 790.00, 6490.00, 57.50, 0, 60, 3, 0, 1, NULL, NULL, NULL, NULL, 0),
(12202018, 106630, 1220, 7334, 7943, '0', 0, 0, 0, 0, 810.00, 6515.00, 56.32, 0, 60, 3, 0, 1, NULL, NULL, NULL, NULL, 0);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 12202101 AND 12202105;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(12202101, 240267, 1220, 7334, 7340, '0', 0, 0, 721.00, 6563.00, 105.29, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(12202102, 240267, 1220, 7334, 7340, '0', 0, 0, 669.02, 6571.10, 65.20, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(12202103, 240267, 1220, 7334, 7340, '0', 0, 0, 698.00, 6586.00, 91.12, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(12202104, 240267, 1220, 7334, 7340, '0', 0, 0, 696.00, 6547.00, 95.42, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(12202105, 240267, 1220, 7334, 7340, '0', 0, 0, 681.00, 6538.00, 98.30, 0, 0, 0, 0, 1, 30, 255, 1, 0);
