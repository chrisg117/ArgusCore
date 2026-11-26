-- Eye of Azshara: Warlord Parjesh (encounter 1810).
-- Spec: docs/superpowers/specs/2026-11-16-dungeon-eye-of-azshara-design.md, "Warlord Parjesh" and its amendment.

-- Scripts. Parjesh's energy bar is set by the script (POWER_ENERGY), not by unit_class.
UPDATE `creature_template` SET `ScriptName` = 'boss_warlord_parjesh' WHERE `entry` = 91784;
UPDATE `creature_template` SET `ScriptName` = 'npc_parjesh_shellbreaker' WHERE `entry` = 97264;
UPDATE `creature_template` SET `ScriptName` = 'npc_parjesh_crestrider' WHERE `entry` = 97269;

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_parjesh_call_reinforcements', 'spell_parjesh_impaling_spear_mark');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(196563, 'spell_parjesh_call_reinforcements'),
(192094, 'spell_parjesh_impaling_spear_mark');

-- Lines: client broadcast texts only; text and sound from the client row. Yells (14); the spear emote is a
-- boss whisper (42) to the marked player. Moments are read from the wording and sound-kit order (54181-54191).
DELETE FROM `creature_text` WHERE `CreatureID` = 91784;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 91784, c.`grp`, c.`idx`, b.`Text`, c.`type`, 0, 100, 0, 0, b.`SoundKitID1`, 0, b.`ID`, 0, c.`cmt` FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 0 AS grp, 0 AS idx, 99890 AS bid, 14 AS type, 'Warlord Parjesh - aggro' AS cmt
  UNION ALL SELECT 1, 0, 99891, 14, 'Warlord Parjesh - Call Reinforcements (melee)'
  UNION ALL SELECT 2, 0, 99892, 14, 'Warlord Parjesh - Call Reinforcements (caster)'
  UNION ALL SELECT 3, 0, 99893, 14, 'Warlord Parjesh - Crashing Wave'
  UNION ALL SELECT 4, 0, 99898, 14, 'Warlord Parjesh - Impaling Spear'
  UNION ALL SELECT 4, 1, 99899, 14, 'Warlord Parjesh - Impaling Spear'
  UNION ALL SELECT 5, 0, 99896, 14, 'Warlord Parjesh - killed a player'
  UNION ALL SELECT 5, 1, 99897, 14, 'Warlord Parjesh - killed a player'
  UNION ALL SELECT 6, 0, 100462, 14, 'Warlord Parjesh - wipe'
  UNION ALL SELECT 7, 0, 99894, 14, 'Warlord Parjesh - death'
  UNION ALL SELECT 8, 0, 99873, 42, 'Warlord Parjesh - Impaling Spear emote to the marked player'
  UNION ALL SELECT 9, 0, 99895, 14, 'Warlord Parjesh - the spear hit his own reinforcement'
) c ON b.`ID` = c.`bid`;

-- Crashing Wave: aura 191900 creates 4863 and 4888 on Parjesh for 0.8 s while he charges 25 yd (191902).
-- Attached (flag 0x4) so they travel with him. 4863 is taken as the hitbox (sphere, radius 5: calibrate),
-- 4888 as its visual. The client has no create-properties for either.
DELETE FROM `areatrigger_template` WHERE `Id` IN (4863, 4888, 4876, 4887) AND `IsCustom` = 0;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(4863, 0, 0, 0, 0, 0),
(4888, 0, 0, 0, 0, 0),
(4876, 0, 0, 0, 0, 0),
(4887, 0, 0, 0, 0, 0);

-- Quicksand (Heroic, Mythic): five missiles each land 192041, which creates 4876 and 4887 for 120 s.
-- 4876 is taken as the puddle (sphere, radius 4: calibrate) that applies 192053 while a player stands in it.
DELETE FROM `areatrigger_create_properties` WHERE `Id` IN (4863, 4888, 4876, 4887) AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(4863, 0, 4863, 0, 4, 0, 5, 5, 'at_parjesh_crashing_wave', 0),
(4888, 0, 4888, 0, 4, 0, 5, 5, '', 0),
(4876, 0, 4876, 0, 0, 0, 4, 4, 'at_parjesh_quicksand', 0),
(4887, 0, 4887, 0, 0, 0, 4, 4, '', 0);

-- Restoration's heal (197504) picks allies by entry: Parjesh and his two reinforcements.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 197504;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 3, 197504, 0, 0, 31, 0, 3, 91784, 0, 0, 0, 0, '', 'Restoration - heals Warlord Parjesh'),
(13, 3, 197504, 0, 1, 31, 0, 3, 97264, 0, 0, 0, 0, '', 'Restoration - heals Hatecoil Shellbreaker'),
(13, 3, 197504, 0, 2, 31, 0, 3, 97269, 0, 0, 0, 0, '', 'Restoration - heals Hatecoil Crestrider');

-- Loot: the 11 journal items (JournalEncounter 1480), one group with equal chance: one item per kill, personal.
DELETE FROM `creature_loot_template` WHERE `Entry` = 91784;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(91784, 0, 137361, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Roaring Shellbreaker Greatbelt'),
(91784, 0, 134240, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Brinewashed Leather Cowl'),
(91784, 0, 134484, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Crestrider Conduit Bracers'),
(91784, 0, 134261, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Seawitch Gloves'),
(91784, 0, 134441, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Shellshock Footguards'),
(91784, 0, 137360, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Shoulderpads of Crashing Waves'),
(91784, 0, 134223, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Coralplate Chestguard'),
(91784, 0, 137362, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Parjesh''s Medallion'),
(91784, 0, 137363, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Bloodied Spearhead'),
(91784, 0, 134251, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Sea Stalker''s Cinch'),
(91784, 0, 134492, 0, 0, 1, 1, 1, 1, 'Warlord Parjesh - Hatecoil Commander''s Amulet');
UPDATE `creature_template_difficulty` SET `LootID` = 91784 WHERE `Entry` = 91784 AND `DifficultyID` = 0;
