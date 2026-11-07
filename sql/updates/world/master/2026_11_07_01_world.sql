-- DK class hall campaign, slice 1 fix round (the 2026-11-07 pass): the raise staged as the data has it, and Thassarian
-- mounted and armed in Durotar.

-- The raise has no body on the ground: the player is given a button instead (Raise Horseman - Aura 213932, override bar
-- 719 with 213614) while The Firstborn Rises is in progress, and Thassarian stages the rest (npc_thassarian_the_firstborn_rises).
-- Nazgrim leaves through a Death Gate, so the Durotar spawns of the body and of Nazgrim standing go.
DELETE FROM `creature` WHERE `guid` IN (10010001, 10010002);
DELETE FROM `creature_addon` WHERE `guid` IN (10010001, 10010002);
UPDATE `creature_template` SET `npcflag`=`npcflag`&~16777216 WHERE `entry`=109886;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=109886;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=109886;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry`=109886;

-- Zone 14 Durotar: the server puts the spot at Thassarian in area 14 itself, not in a subarea
DELETE FROM `spell_area` WHERE `spell`=213932;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `flags`, `quest_start_status`, `quest_end_status`) VALUES
(213932, 14, 42484, 42484, 0, 0, 2, 3, 8, 8); -- Raise Horseman - Aura: while The Firstborn Rises is in progress

-- Thassarian faces the raise spot; the anchor (and Nazgrim on it) face him
UPDATE `spell_target_position` SET `Orientation`=0.59 WHERE `ID`=213633 AND `EffectIndex`=0;

UPDATE `creature_template` SET `ScriptName`='npc_thassarian_the_firstborn_rises' WHERE `entry`=107554;
UPDATE `creature_template` SET `ScriptName`='npc_the_firstborn_rises_anchor' WHERE `entry`=107597;
DELETE FROM `spell_script_names` WHERE `spell_id`=213614;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(213614, 'spell_dk_the_firstborn_rises_raise_horseman');

-- The exchange: each speaker is the one the line's voice file is named for (SoundKitEntry -> vo_703_*)
DELETE FROM `creature_text` WHERE `CreatureID` IN (107554, 107602);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(107554, 0, 0, 'It is time, my lord. Do what must be done.', 12, 0, 100, 1, 0, 69388, 0, 112575, 0, 'Thassarian (Durotar) - The Firstborn Rises accepted'),
(107554, 1, 0, 'Nazgrim, your Deathlord calls! Feel the icy grip of your master and rise once more!', 14, 0, 100, 0, 0, 69387, 0, 112573, 0, 'Thassarian (Durotar) - raise begun'),
(107554, 2, 0, 'Long enough, my friend. The Burning Legion has returned and you have been called to the battle.', 12, 0, 100, 1, 0, 69391, 0, 112588, 0, 'Thassarian (Durotar) - to Nazgrim'),
(107554, 3, 0, 'Indeed you shall, general. Indeed you shall...', 12, 0, 100, 1, 0, 69392, 0, 112589, 0, 'Thassarian (Durotar) - to Nazgrim'),
(107602, 0, 0, 'I live again... How long has it been since I tasted death?', 12, 0, 100, 0, 0, 69389, 0, 112584, 0, 'Nazgrim (raised) - risen'),
(107602, 1, 0, 'Say no more. I will sever their heads from their bodies...', 12, 0, 100, 1, 0, 69390, 0, 112587, 0, 'Nazgrim (raised) - to Thassarian');

-- Thassarian rides a Crimson Deathcharger in Durotar (Mount 366 -> MountXDisplay 25279); picked from the preview lineup
DELETE FROM `creature_addon` WHERE `guid`=10010000;
INSERT INTO `creature_addon` (`guid`, `mount`) VALUES
(10010000, 25279);

-- Thassarian in Durotar carries his two swords: item 35561, the pair every earlier Thassarian (26170, 29799, 32310,
-- 44453, 53427) is equipped with
DELETE FROM `creature_equip_template` WHERE `CreatureID`=107554;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(107554, 1, 35561, 0, 0, 35561, 0, 0, 0, 0, 0, 0);
UPDATE `creature` SET `equipment_id`=1 WHERE `guid`=10010000;
