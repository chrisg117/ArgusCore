-- Warlock re-do of The Sixth / The New Blood, staged to the watcher's timeline of Wowhead's acquisition video
-- (warlock re-do, 2026-11-10). The ritual in the Circle, scenario 962's actors and lines, the captives' guards, Nelach in
-- the archive, the Tome on its lectern, Xel'toth at the exit gateway, and Calydus's lines on the landing.
-- Entries from the data: the Circle council and Jagganoth are the copies whose CreatureDifficultyIDs run with Ritssyn
-- 101456 and the Jailer 101460 (101455 Shinfel, 101457 Jubeka, 101468 Jagganoth); 962 keeps 98771 / 99145-99147 / 98831.

-- creature_text: every line from its broadcast_text row, in the order the video plays it
DELETE FROM `creature_text` WHERE `CreatureID` IN (101456, 98797, 101455, 101457, 101468, 98831, 98771, 98836, 101050,
99397, 98878, 99147, 99145, 99146, 99516, 101097);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT c.`cid`, c.`grp`, 0, b.`Text`, c.`type`, 0, 100, 0, 0, 0, 0, b.`ID`, c.`rng`, c.`cmt` FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 101456 AS cid, 0 AS grp, 103743 AS bid, 12 AS type, 0 AS rng, 'Ritssyn Flamescowl (Circle) - the sixth has come' AS cmt
  UNION ALL SELECT 101456, 1, 103744, 12, 0, 'Ritssyn Flamescowl (Circle) - Zelifrax leads'
  UNION ALL SELECT 101456, 2, 103753, 12, 0, 'Ritssyn Flamescowl (Circle) - a great power beyond'
  UNION ALL SELECT 101456, 3, 103755, 12, 0, 'Ritssyn Flamescowl (Circle) - snaps the gnome''s name'
  UNION ALL SELECT 98797, 0, 103745, 12, 0, 'Zelifrax Wobblepox - knows the job'
  UNION ALL SELECT 98797, 1, 103746, 12, 0, 'Zelifrax Wobblepox - incantation 1'
  UNION ALL SELECT 98797, 2, 103747, 12, 0, 'Zelifrax Wobblepox - channel on the portal'
  UNION ALL SELECT 98797, 3, 103750, 12, 0, 'Zelifrax Wobblepox - the ritual begins'
  UNION ALL SELECT 98797, 4, 103764, 12, 0, 'Zelifrax Wobblepox - incantation 2'
  UNION ALL SELECT 98797, 5, 103752, 12, 0, 'Zelifrax Wobblepox - incantation 3'
  UNION ALL SELECT 98797, 6, 103754, 12, 0, 'Zelifrax Wobblepox - incantation 4'
  UNION ALL SELECT 98797, 7, 103756, 12, 0, 'Zelifrax Wobblepox - mixes the words up'
  UNION ALL SELECT 98797, 8, 103759, 12, 0, 'Zelifrax Wobblepox - commands the demon'
  UNION ALL SELECT 98797, 9, 103761, 14, 0, 'Zelifrax Wobblepox - death scream'
  UNION ALL SELECT 101455, 0, 103748, 12, 0, 'Shinfel Blightsworn (Circle) - worry about the newcomer'
  UNION ALL SELECT 101455, 1, 103749, 12, 0, 'Shinfel Blightsworn (Circle) - begin'
  UNION ALL SELECT 101457, 0, 103751, 12, 0, 'Jubeka Shadowbreaker (Circle) - deeper into the Nether'
  UNION ALL SELECT 101457, 1, 103757, 12, 0, 'Jubeka Shadowbreaker (Circle) - the portal'
  UNION ALL SELECT 101468, 0, 103758, 14, 0, 'Jagganoth (Circle) - who summons me'
  UNION ALL SELECT 101468, 1, 103760, 14, 0, 'Jagganoth (Circle) - scorns the gnome'
  UNION ALL SELECT 101468, 2, 103762, 14, 0, 'Jagganoth (Circle) - foolish mortals'
  UNION ALL SELECT 101468, 3, 103763, 14, 0, 'Jagganoth (Circle) - you will serve me'
  UNION ALL SELECT 98771, 0, 100640, 12, 0, 'Ritssyn Flamescowl (prisoner) - the new blood wakes'
  UNION ALL SELECT 98771, 1, 100641, 12, 0, 'Ritssyn Flamescowl (prisoner) - Jagganoth''s prisoners'
  UNION ALL SELECT 98771, 2, 100642, 12, 0, 'Ritssyn Flamescowl (prisoner) - skip the torment'
  UNION ALL SELECT 98771, 3, 100673, 12, 0, 'Ritssyn Flamescowl (prisoner) - barely focus'
  UNION ALL SELECT 98771, 4, 100674, 12, 0, 'Ritssyn Flamescowl (prisoner) - control the jailer'
  UNION ALL SELECT 98771, 5, 100680, 41, 0, 'Ritssyn Flamescowl (prisoner) - a scream from the top of the pit'
  UNION ALL SELECT 98771, 6, 100681, 12, 0, 'Ritssyn Flamescowl (prisoner) - that was Jubeka'
  UNION ALL SELECT 98771, 7, 100682, 12, 0, 'Ritssyn Flamescowl (prisoner) - find the others'
  UNION ALL SELECT 98771, 8, 100683, 12, 0, 'Ritssyn Flamescowl (prisoner) - leave me'
  UNION ALL SELECT 98836, 0, 101101, 12, 0, 'Beshtal - obeys'
  UNION ALL SELECT 98836, 1, 100678, 14, 0, 'Beshtal - a prisoner left his cells'
  UNION ALL SELECT 98836, 2, 122631, 12, 0, 'Beshtal - dying'
  UNION ALL SELECT 101050, 0, 100800, 12, 0, 'Calydus (prison) - don''t hurt Calydus'
  UNION ALL SELECT 101050, 1, 100802, 12, 0, 'Calydus (prison) - servant to Jagganoth'
  UNION ALL SELECT 101050, 2, 100803, 12, 0, 'Calydus (prison) - will distract the guards'
  UNION ALL SELECT 101050, 3, 100907, 14, 0, 'Calydus (prison) - this way'
  UNION ALL SELECT 101050, 4, 100909, 12, 0, 'Calydus (prison) - a friend'
  UNION ALL SELECT 101050, 5, 100913, 12, 0, 'Calydus (prison) - the archives are close'
  UNION ALL SELECT 101050, 6, 103457, 12, 0, 'Calydus (prison) - here is the tome'
  UNION ALL SELECT 101050, 7, 103458, 12, 0, 'Calydus (prison) - take it and go'
  UNION ALL SELECT 101050, 8, 103429, 14, 0, 'Calydus (prison) - flee through the gateway'
  UNION ALL SELECT 101050, 9, 103433, 14, 0, 'Calydus (prison) - the way is open'
  UNION ALL SELECT 99397, 0, 100804, 14, 0, 'Dreadscar Sunderer - a prisoner escaped'
  UNION ALL SELECT 98878, 0, 100868, 12, 0, 'Dreadscar Torturer - the silent worgen'
  UNION ALL SELECT 98878, 1, 100866, 12, 0, 'Dreadscar Torturer - her mind'
  UNION ALL SELECT 99147, 0, 100871, 12, 0, 'Jubeka Shadowbreaker - nearly hound food'
  UNION ALL SELECT 99147, 1, 100893, 12, 0, 'Jubeka Shadowbreaker - where is Ritssyn'
  UNION ALL SELECT 99147, 2, 100897, 12, 0, 'Jubeka Shadowbreaker - keep moving'
  UNION ALL SELECT 99147, 3, 100912, 12, 0, 'Jubeka Shadowbreaker - can we trust it'
  UNION ALL SELECT 99147, 4, 100969, 12, 0, 'Jubeka Shadowbreaker - a bad feeling'
  UNION ALL SELECT 99147, 5, 103455, 12, 0, 'Jubeka Shadowbreaker - what the tomes could teach'
  UNION ALL SELECT 99147, 6, 103359, 14, 0, 'Jubeka Shadowbreaker - Shinfel'
  UNION ALL SELECT 99147, 7, 103422, 14, 0, 'Jubeka Shadowbreaker - warns Zinnin'
  UNION ALL SELECT 99147, 8, 103431, 12, 0, 'Jubeka Shadowbreaker - Zinnin'
  UNION ALL SELECT 99147, 9, 103440, 14, 0, 'Jubeka Shadowbreaker - dismay'
  UNION ALL SELECT 99147, 10, 103364, 12, 0, 'Jubeka Shadowbreaker - close the portal'
  UNION ALL SELECT 99145, 0, 100869, 12, 0, 'Zinnin Smythe - silence (pillar)'
  UNION ALL SELECT 99145, 1, 100895, 12, 0, 'Zinnin Smythe - silence (passage)'
  UNION ALL SELECT 99146, 0, 100865, 12, 0, 'Shinfel Blightsworn - weak torture'
  UNION ALL SELECT 99146, 1, 100867, 12, 0, 'Shinfel Blightsworn - thanks'
  UNION ALL SELECT 99146, 2, 100894, 12, 0, 'Shinfel Blightsworn - the orc is too hurt'
  UNION ALL SELECT 99146, 3, 100896, 12, 0, 'Shinfel Blightsworn - enslave him if we can'
  UNION ALL SELECT 99146, 4, 100908, 12, 0, 'Shinfel Blightsworn - what is this wretched thing'
  UNION ALL SELECT 99146, 5, 100911, 12, 0, 'Shinfel Blightsworn - ruling'
  UNION ALL SELECT 99146, 6, 103456, 12, 0, 'Shinfel Blightsworn - the power'
  UNION ALL SELECT 99146, 7, 103355, 12, 0, 'Shinfel Blightsworn - discovered'
  UNION ALL SELECT 99146, 8, 103357, 12, 0, 'Shinfel Blightsworn - struggles'
  UNION ALL SELECT 99516, 0, 103453, 14, 0, 'Nelach, the All-Seeing - intruders'
  UNION ALL SELECT 99516, 1, 103454, 12, 0, 'Nelach, the All-Seeing - death'
  UNION ALL SELECT 98831, 0, 103354, 14, 3, 'Jagganoth (962) - insolent'
  UNION ALL SELECT 98831, 1, 103356, 14, 3, 'Jagganoth (962) - slip away'
  UNION ALL SELECT 98831, 2, 103358, 14, 3, 'Jagganoth (962) - command my kind'
  UNION ALL SELECT 98831, 3, 103360, 14, 3, 'Jagganoth (962) - like gnats'
  UNION ALL SELECT 98831, 4, 103361, 14, 3, 'Jagganoth (962) - fel fire'
  UNION ALL SELECT 98831, 5, 103363, 14, 3, 'Jagganoth (962) - a skull trophy (talking head in the video)'
  UNION ALL SELECT 98831, 6, 103365, 14, 3, 'Jagganoth (962) - burn to ash (talking head in the video)'
  UNION ALL SELECT 98831, 7, 103366, 14, 3, 'Jagganoth (962) - not over (talking head in the video)'
  UNION ALL SELECT 101097, 0, 103567, 12, 0, 'Calydus (Circle) - they did it'
  UNION ALL SELECT 101097, 1, 103568, 12, 0, 'Calydus (Circle) - find the weapon'
) c ON b.`ID` = c.`bid`;

-- levels: the new actors were level 1 with no scaling (the warlock opening's shape)
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110
WHERE `DifficultyID`=0 AND `Entry` IN (98878, 99397, 99516, 101468);

-- 962 spawns. The captives hang 3 yd up on their spike pillars; 98831 is summoned at the archive now; the archive's
-- Raider becomes Nelach; Xel'toth stands before the exit gateway
DELETE FROM `creature` WHERE `guid` = 15220009;
UPDATE `creature` SET `position_z` = 224.21 WHERE `guid` = 15220002;
UPDATE `creature` SET `position_z` = 224.40 WHERE `guid` = 15220003;
UPDATE `creature` SET `position_z` = 225.95 WHERE `guid` = 15220004;
UPDATE `creature` SET `id` = 99516, `ScriptName` = 'npc_nelach_the_all_seeing' WHERE `guid` = 15220008;
UPDATE `creature` SET `position_x` = 3087.00, `position_y` = 858.00, `position_z` = 244.50, `orientation` = 1.57 WHERE `guid` = 15220007;
DELETE FROM `creature` WHERE `guid` BETWEEN 15220011 AND 15220014;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15220011, 99397, 1522, 0, 0, '12', 0, 0, 0, -1, 0, 0, 3012.50, 1166.50, 221.35, 3.80, 86400, 0, 0, 0, 0, 0, 0, 0, 'npc_dreadscar_captive_guard', NULL, 0),
(15220012, 98813, 1522, 0, 0, '12', 0, 0, 0, -1, 0, 0, 3008.50, 1165.50, 221.30, 3.80, 86400, 0, 0, 0, 0, 0, 0, 0, 'npc_dreadscar_captive_guard', NULL, 0),
(15220013, 98878, 1522, 0, 0, '12', 0, 0, 0, -1, 0, 0, 3019.20, 1147.00, 221.40, 1.40, 86400, 0, 0, 0, 0, 0, 0, 0, 'npc_dreadscar_captive_guard', NULL, 0),
(15220014, 98878, 1522, 0, 0, '12', 0, 0, 0, -1, 0, 0, 3034.50, 1113.50, 222.60, 0.90, 86400, 0, 0, 0, 0, 0, 0, 0, 'npc_dreadscar_captive_guard', NULL, 0);

-- the fel-flame wall across the passage (the data's "Fel Wall" 243492, display 26055 6fx_firewall_doorfel) and the Tome
-- on the archive's central lectern (fork chest, display 27637 7lg_legion_book02; not selectable until Nelach falls)
DELETE FROM `gameobject_template` WHERE `entry` = 1522002;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `ScriptName`, `VerifiedBuild`) VALUES
(1522002, 3, 27637, 'Tome of Blighted Implements', '', '', '', 1, 43, 1522002, 0, 1, '', 0);
DELETE FROM `gameobject_template_addon` WHERE `entry` = 1522002;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`) VALUES (1522002, 0, 16);
DELETE FROM `gameobject_loot_template` WHERE `Entry` = 1522002;
INSERT INTO `gameobject_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(1522002, 0, 132512, 100, 0, 1, 0, 1, 1, 'Tome of Blighted Implements');
DELETE FROM `gameobject` WHERE `guid` IN (15220001, 15220002);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15220001, 243492, 1522, 0, 0, '12', 0, 0, 0, -1, 3045.00, 1086.50, 226.50, 5.84, 0, 0, 0.219784, -0.975549, 86400, 255, 1, '', NULL, 0),
(15220002, 1522002, 1522, 0, 0, '12', 0, 0, 0, -1, 3062.00, 925.50, 251.50, 3.30, 0, 0, 0.996865, -0.079121, 86400, 255, 1, '', NULL, 0);

-- the new 962 actors are hidden on The Heart of the Dreadscar (998 shares the map), like the rest of 962
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND ((`SourceGroup` = 3 AND `SourceEntry` IN (99397, 98813, 98878, 99516))
  OR (`SourceGroup` = 5 AND `SourceEntry` IN (243492, 1522002)));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 99397, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Dreadscar Sunderer: not in 998'),
(32, 3, 98813, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Bloodscent Felhound: not in 998'),
(32, 3, 98878, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Dreadscar Torturer: not in 998'),
(32, 3, 99516, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Nelach: not in 998'),
(32, 5, 243492, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Fel Wall: not in 998'),
(32, 5, 1522002, 0, 0, 47, 0, 40731, 8, 0, 1, '962 Tome of Blighted Implements: not in 998');

-- spell targets: the council's Fel Ritual channels on the World Trigger in the gateway; Jagganoth's Grab Zelifrax
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (199426, 200016);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(13, 1, 199426, 0, 0, 31, 0, 3, 22515, 0, 0, 'Fel Ritual: the World Trigger in the ritual gateway'),
(13, 1, 200016, 0, 0, 31, 0, 3, 98797, 0, 0, 'Grab Zelifrax: Zelifrax');

-- Exit Teleport lands on the grate's centre, beside Calydus (13:20)
UPDATE `spell_target_position` SET `PositionX` = -855.50, `PositionY` = 4452.00, `PositionZ` = 696.70, `Orientation` = 2.10
WHERE `ID` = 196340 AND `EffectIndex` = 1;
