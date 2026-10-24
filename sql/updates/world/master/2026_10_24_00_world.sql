-- Stop Gul'dan! (38723 Havoc, 40253 Vengeance), after Rise of the Illidari and Fel Infusion, had no one to give it and
-- nothing to do it with.
-- - Maiev Shadowsong gives it in the Tomb of the Penitent, the version of the player's Fel Secrets path (Cry Havoc or
--   Vengeance Will Be Mine! rewarded). Her wake-up lines now belong to her first spawn only.
-- - Accepting it plays Gul'dan taking Illidan's body (scene package 1423, which nothing in the data starts; our scene
--   id) and gives Face Gul'dan (quest_stop_guldan); the scene leaves the player in the Vault of the Betrayer
--   (scene_stop_guldan).
-- - The door to the Vault of the Betrayer (Illidan's Rest) stays shut until the quest is taken; Maiev opens it in the
--   scene.
-- - There Crusher and Sledge, summoned for the player once at the scene's end, fight Kayn and Altruis before the open
--   portals, all where the scene leaves them. The brutes
--   pass the Massive Sword between them (Brutal Attacks); Kayn and Altruis argue with their retail lines once a player
--   joins in (npc_stop_guldan_brute). Kayn and Altruis take the Freed Illidari's faction so they fight the brutes.
-- - When both brutes are dead, their killers channel Taking Power on the corpses for Vengeful Retreat (Havoc) or
--   Fiery Brand (Vengeance), which gives the last objective (Mardum's Taking Power scripts). Kayn ends it there.

DELETE FROM `creature_queststarter` WHERE `quest` IN (38723, 40253);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(92718, 38723, 0),
(92718, 40253, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (38723, 40253);
INSERT INTO `quest_template_addon` (`ID`, `ScriptName`) VALUES
(38723, 'quest_stop_guldan'),
(40253, 'quest_stop_guldan');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` IN (38723, 40253);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 38723, 0, 0, 8, 0, 38690, 0, 0, '', 0, 0, 0, '', 'Stop Gul''dan!: Rise of the Illidari done'),
(19, 0, 38723, 0, 0, 8, 0, 38689, 0, 0, '', 0, 0, 0, '', 'Stop Gul''dan!: Fel Infusion done'),
(19, 0, 38723, 0, 0, 8, 0, 39516, 0, 0, '', 0, 0, 0, '', 'Stop Gul''dan! (Havoc: Cry Havoc done)'),
(19, 0, 40253, 0, 0, 8, 0, 38690, 0, 0, '', 0, 0, 0, '', 'Stop Gul''dan!: Rise of the Illidari done'),
(19, 0, 40253, 0, 0, 8, 0, 38689, 0, 0, '', 0, 0, 0, '', 'Stop Gul''dan!: Fel Infusion done'),
(19, 0, 40253, 0, 0, 8, 0, 39515, 0, 0, '', 0, 0, 0, '', 'Stop Gul''dan! (Vengeance: Vengeance Will Be Mine! done)');

-- Maiev stands in two rooms: told apart by Kayn (96665), who waits in the Tomb with her. In the Tomb she gives the quest
-- and leaves with it (the scene).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 92718;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 92718, 0, 0, 29, 1, 96665, 15, 0, '', 1, 0, 0, '', 'Maiev: at the cells (not in the Tomb)'),
(32, 3, 92718, 0, 1, 29, 1, 96665, 15, 0, '', 0, 0, 0, '', 'Maiev: in the Tomb of the Penitent'),
(32, 3, 92718, 0, 1, 47, 0, 38672, 64, 0, '', 0, 0, 0, '', 'Maiev: in the Tomb once Breaking Out is done'),
(32, 3, 92718, 0, 1, 47, 0, 38723, 1, 0, '', 0, 0, 0, '', 'Maiev: in the Tomb until Stop Gul''dan! (Havoc) is taken'),
(32, 3, 92718, 0, 1, 47, 0, 40253, 1, 0, '', 0, 0, 0, '', 'Maiev: in the Tomb until Stop Gul''dan! (Vengeance) is taken');
UPDATE `smart_scripts` SET `entryorguid` = -14680001 WHERE `entryorguid` = 92718 AND `source_type` = 0;

-- Kayn and Altruis fight the brutes, with warglaives
UPDATE `creature_template` SET `faction` = 2804 WHERE `entry` IN (96665, 92985);
DELETE FROM `creature_equip_template` WHERE `CreatureID` IN (96665, 92985);
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(96665, 1, 128359, 0, 0, 128371, 0, 0, 0, 0, 0, 0),
(92985, 1, 128359, 0, 0, 128371, 0, 0, 0, 0, 0, 0);
UPDATE `creature_template` SET `ScriptName` = 'npc_stop_guldan_brute' WHERE `entry` IN (97632, 92990);

DELETE FROM `creature` WHERE `guid` BETWEEN 14680200 AND 14680219;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680200, 92718, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4235.61, -292.96, -281.40, 3.2987, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Maiev, Tomb of the Penitent
(14680201, 96665, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4058.59, -305.24, -281.59, 3.2524, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfTheBetrayerSquad', 0), -- Kayn, against Sledge
(14680202, 92985, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4056.99, -287.60, -281.59, 2.9060, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfTheBetrayerSquad', 0), -- Altruis, against Crusher
(14680205, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4024.46, -298.00, -283.83, 3.1416, 120, 0, 0, 100, 0, 0, NULL, NULL, NULL, '', NULL, 0), -- Legion Portal, middle
(14680206, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4024.85, -279.98, -281.11, 3.1416, 120, 0, 0, 100, 0, 0, NULL, NULL, NULL, '', NULL, 0), -- Legion Portal, right
(14680207, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4025.35, -317.45, -281.11, 3.1416, 120, 0, 0, 100, 0, 0, NULL, NULL, NULL, '', NULL, 0); -- Legion Portal, left

-- The door from the Tomb of the Penitent to the Vault of the Betrayer (Illidan's Rest, the scene's door), shut until the
-- player takes Stop Gul'dan!: Maiev opens it in the scene
DELETE FROM `gameobject` WHERE `guid` = 14680200;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680200, 244404, 1468, 7814, 0, '0', 0, 0, 0, -1, 4217.04, -297.77, -281.18, 0, 0, 0, 0, 1, 0, 255, 1, '', NULL, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 5 AND `SourceEntry` = 244404;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 5, 244404, 0, 0, 47, 0, 38723, 1, 0, '', 0, 0, 0, '', 'Illidan''s Rest door: shut until Stop Gul''dan! (Havoc) is taken'),
(32, 5, 244404, 0, 0, 47, 0, 40253, 1, 0, '', 0, 0, 0, '', 'Illidan''s Rest door: shut until Stop Gul''dan! (Vengeance) is taken');

DELETE FROM `creature_text` WHERE `CreatureID` IN (97632, 92990, 92985, 96665);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(97632, 0, 0, '|TInterface\\Icons\\inv_sword_115:20|t%s equips the Massive Sword and gains |cFFFF0000|Hspell:199556|h[Brutal Attacks]|h|r!', 41, 0, 100, 0, 0, 0, 0, 104478, 0, 'Crusher - equips the Massive Sword'),
(97632, 1, 0, '|cFFFF0000$n|r attack the brute without the |cFFFF0000|Hspell:199556|h[Brutal Attacks]|h|r buff!', 42, 0, 100, 0, 0, 0, 0, 104480, 0, 'Crusher - struck with Brutal Attacks'),
(92990, 0, 0, '|TInterface\\Icons\\inv_sword_115:20|t%s equips the Massive Sword and gains |cFFFF0000|Hspell:199556|h[Brutal Attacks]|h|r!', 41, 0, 100, 0, 0, 0, 0, 104478, 0, 'Sledge - equips the Massive Sword'),
(92990, 1, 0, '|cFFFF0000$n|r attack the brute without the |cFFFF0000|Hspell:199556|h[Brutal Attacks]|h|r buff!', 42, 0, 100, 0, 0, 0, 0, 104480, 0, 'Sledge - struck with Brutal Attacks'),
(92985, 0, 0, 'Just like old times, eh Kayn?', 12, 0, 100, 0, 0, 57329, 0, 99522, 0, 'Altruis - Stop Gul''dan! fight'),
(96665, 0, 0, 'Do not speak to me of old times, Altruis. You betrayed Illidan. You betrayed us all!', 12, 0, 100, 0, 0, 57345, 0, 99524, 0, 'Kayn - Stop Gul''dan! fight'),
(92985, 1, 0, 'After ten-thousand years of imprisonment, Illidan succumbed to the fel energy within. When he was freed, he was not in his right mind.', 12, 0, 100, 0, 0, 57284, 0, 99525, 0, 'Altruis - Stop Gul''dan! fight'),
(96665, 1, 0, 'He was fighting it, as we all do. He needed us to have faith in him, Altruis.', 12, 0, 100, 0, 0, 57325, 0, 99526, 0, 'Kayn - Stop Gul''dan! fight'),
(92985, 2, 0, 'You followed Illidan blindly, like a dog. Even when he lost his way.', 12, 0, 100, 0, 0, 57326, 0, 99527, 0, 'Altruis - Stop Gul''dan! fight'),
(96665, 2, 0, 'He made the hard choices. He sacrificed EVERYTHING. What did you give?', 12, 0, 100, 0, 0, 57321, 0, 99528, 0, 'Kayn - Stop Gul''dan! fight'),
(92985, 3, 0, 'I fought as hard as the rest of you! But I did not murder thousands of innocents.', 12, 0, 100, 0, 0, 57386, 0, 102022, 0, 'Altruis - Stop Gul''dan! fight');

DELETE FROM `scene_template` WHERE `SceneId` = 14680001;
INSERT INTO `scene_template` (`SceneId`, `Flags`, `ScriptPackageID`, `ScriptName`) VALUES
(14680001, 50, 1423, 'scene_stop_guldan'); -- fade to black on complete and on cancel, infinite AOI

DELETE FROM `spell_script_names` WHERE `spell_id` IN (210453, 210467);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210453, 'spell_stop_guldan_taking_power'),
(210467, 'spell_stop_guldan_taking_power');
