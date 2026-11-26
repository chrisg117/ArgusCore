-- Eye of Azshara: Lady Hatecoil (encounter 1811).
-- Spec: docs/superpowers/specs/2026-11-16-dungeon-eye-of-azshara-design.md, "Lady Hatecoil" and its amendment.

UPDATE `creature_template` SET `ScriptName` = 'boss_lady_hatecoil' WHERE `entry` = 91789;
UPDATE `creature_template` SET `ScriptName` = 'npc_hatecoil_saltsea_globule' WHERE `entry` = 98293;
-- Monsoon casts its knockback at players: faction 16 (the script makes it unattackable and passive).
UPDATE `creature_template` SET `ScriptName` = 'npc_hatecoil_monsoon', `faction` = 16 WHERE `entry` = 99852;

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_hatecoil_static_nova', 'spell_hatecoil_excess_lightning',
 'spell_hatecoil_beckon_storm', 'spell_hatecoil_curse_selector', 'spell_hatecoil_curse_of_the_witch', 'spell_hatecoil_monsoon_selector');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(193597, 'spell_hatecoil_static_nova'),
(193625, 'spell_hatecoil_excess_lightning'),
(193682, 'spell_hatecoil_beckon_storm'),
(193712, 'spell_hatecoil_curse_selector'),
(193716, 'spell_hatecoil_curse_selector'),
(193717, 'spell_hatecoil_curse_selector'),
(193698, 'spell_hatecoil_curse_of_the_witch'),
(196634, 'spell_hatecoil_monsoon_selector');

-- Lines: client broadcast texts; her lines are in the female columns (Text1, SoundKitID2).
DELETE FROM `creature_text` WHERE `CreatureID` = 91789;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 91789, c.`grp`, c.`idx`, IF(b.`Text` <> '', b.`Text`, b.`Text1`), 14, 0, 100, 0, 0, IF(b.`SoundKitID1` <> 0, b.`SoundKitID1`, b.`SoundKitID2`), 0, b.`ID`, 0, c.`cmt`
FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 0 AS grp, 0 AS idx, 100227 AS bid, 'Lady Hatecoil - aggro' AS cmt
  UNION ALL SELECT 1, 0, 100236, 'Lady Hatecoil - Static Nova'
  UNION ALL SELECT 2, 0, 100235, 'Lady Hatecoil - Focused Lightning'
  UNION ALL SELECT 3, 0, 100245, 'Lady Hatecoil - Beckon Storm'
  UNION ALL SELECT 3, 1, 100237, 'Lady Hatecoil - Beckon Storm'
  UNION ALL SELECT 4, 0, 100244, 'Lady Hatecoil - Curse of the Witch'
  UNION ALL SELECT 5, 0, 100228, 'Lady Hatecoil - Monsoon'
  UNION ALL SELECT 6, 0, 100230, 'Lady Hatecoil - killed a player'
  UNION ALL SELECT 6, 1, 100231, 'Lady Hatecoil - killed a player'
  UNION ALL SELECT 7, 0, 100463, 'Lady Hatecoil - wipe'
  UNION ALL SELECT 8, 0, 100458, 'Lady Hatecoil - death'
) c ON b.`ID` = c.`bid`;

-- Entry-area targets. Excess Lightning: 193624 hurts players beside the struck player; 193625 finds Sand
-- Dunes. The curse's release: 193700 knocks players back, 193720 kills Saltsea Globules.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (193624, 193625, 193700, 193720);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 193624, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Excess Lightning - players'),
(13, 1, 193625, 0, 0, 31, 0, 3, 97853, 0, 0, 0, 0, '', 'Excess Lightning - Sand Dunes'),
(13, 1, 193700, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Curse of the Witch release - players'),
(13, 1, 193720, 0, 0, 31, 0, 3, 98293, 0, 0, 0, 0, '', 'Curse of the Witch release - Saltsea Globules');

-- Loot: the 12 journal items (JournalEncounter 1490), one group with equal chance.
DELETE FROM `creature_loot_template` WHERE `Entry` = 91789;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(91789, 0, 134525, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Seal of the Nazjatar Empire'),
(91789, 0, 137367, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Stormsinger Fulmination Charge'),
(91789, 0, 137364, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Crashing Oceantide Mantle'),
(91789, 0, 134238, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Brinewashed Leather Pants'),
(91789, 0, 134230, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Coralplate Wristguard'),
(91789, 0, 137365, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Condensed Saltsea Globule'),
(91789, 0, 134253, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Sea Stalker''s Gloves'),
(91789, 0, 134263, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Seawitch Robes'),
(91789, 0, 134471, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Helm of Endless Dunes'),
(91789, 0, 134433, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Cord of the Sea-Caller'),
(91789, 0, 134505, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Horizon Line Warboots'),
(91789, 0, 137366, 0, 0, 1, 1, 1, 1, 'Lady Hatecoil - Gift of the Ocean Empress');
UPDATE `creature_template_difficulty` SET `LootID` = 91789 WHERE `Entry` = 91789 AND `DifficultyID` = 0;
