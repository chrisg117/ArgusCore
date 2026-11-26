-- Eye of Azshara: Wrath of Azshara (encounter 1814).
-- Spec: docs/superpowers/specs/2026-11-16-dungeon-eye-of-azshara-design.md, "Wrath of Azshara" and its amendment.

UPDATE `creature_template` SET `ScriptName` = 'boss_wrath_of_azshara' WHERE `entry` = 96028;
-- The tornado and the bomb cast at players: faction 16 (the scripts make them passive and unattackable).
UPDATE `creature_template` SET `ScriptName` = 'npc_wrath_mystic_tornado', `faction` = 16 WHERE `entry` = 97673;
UPDATE `creature_template` SET `ScriptName` = 'npc_wrath_arcane_bomb', `faction` = 16 WHERE `entry` = 97691;
-- The Tidal Wave stalker only marks the wave; the Wrath casts its damage.
UPDATE `creature_template` SET `ScriptName` = 'npc_wrath_tidal_wave' WHERE `entry` = 97739;

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_wrath_mystic_tornado', 'spell_wrath_arcane_bomb', 'spell_wrath_crushing_depths', 'spell_wrath_resonance');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(192680, 'spell_wrath_mystic_tornado'),
(192706, 'spell_wrath_arcane_bomb'),
(197365, 'spell_wrath_crushing_depths'),
-- Elemental Resonance (Heroic, Mythic) on each of her damaging spells
(192619, 'spell_wrath_resonance'),
(192696, 'spell_wrath_resonance'),
(192675, 'spell_wrath_resonance'),
(192708, 'spell_wrath_resonance'),
(192794, 'spell_wrath_resonance'),
(192801, 'spell_wrath_resonance');

-- Lines: client broadcast texts. Yells (14); the Arcane Bomb emote (99877) is a boss emote (41) naming the target.
DELETE FROM `creature_text` WHERE `CreatureID` = 96028;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 96028, c.`grp`, c.`idx`, IF(b.`Text` <> '', b.`Text`, b.`Text1`), c.`type`, 0, 100, 0, 0, IF(b.`SoundKitID1` <> 0, b.`SoundKitID1`, b.`SoundKitID2`), 0, b.`ID`, 3, c.`cmt`
FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 0 AS grp, 0 AS idx, 99950 AS bid, 14 AS type, 'Wrath of Azshara - the ritual is broken' AS cmt
  UNION ALL SELECT 1, 0, 100046, 14, 'Wrath of Azshara - aggro'
  UNION ALL SELECT 2, 0, 99948, 14, 'Wrath of Azshara - Massive Deluge'
  UNION ALL SELECT 3, 0, 99947, 14, 'Wrath of Azshara - Crushing Depths'
  UNION ALL SELECT 4, 0, 99949, 14, 'Wrath of Azshara - Mystic Tornado'
  UNION ALL SELECT 5, 0, 99956, 14, 'Wrath of Azshara - Arcane Bomb'
  UNION ALL SELECT 5, 1, 99957, 14, 'Wrath of Azshara - Arcane Bomb'
  UNION ALL SELECT 6, 0, 99943, 14, 'Wrath of Azshara - Cry of Wrath'
  UNION ALL SELECT 6, 1, 99944, 14, 'Wrath of Azshara - Cry of Wrath'
  UNION ALL SELECT 7, 0, 99945, 14, 'Wrath of Azshara - killed a player'
  UNION ALL SELECT 7, 1, 99946, 14, 'Wrath of Azshara - killed a player'
  UNION ALL SELECT 8, 0, 100477, 14, 'Wrath of Azshara - death'
  UNION ALL SELECT 9, 0, 99877, 41, 'Wrath of Azshara - Arcane Bomb emote (DBM keys on spell:192708)'
) c ON b.`ID` = c.`bid`;

-- Mystic Tornado 4930: sphere, radius 3 (calibrate; the client has no shape).
DELETE FROM `areatrigger_template` WHERE `Id` = 4930 AND `IsCustom` = 0;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(4930, 0, 0, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` = 4930 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(4930, 0, 4930, 0, 4, 0, 3, 3, 'at_wrath_mystic_tornado', 0);

-- Loot: the 14 journal items (JournalEncounter 1492), one group with equal chance.
DELETE FROM `creature_loot_template` WHERE `Entry` = 96028;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(96028, 0, 134255, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Sea Stalker''s Hood'),
(96028, 0, 137379, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Tempestbinder''s Crystal'),
(96028, 0, 134259, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Seawitch Cinch'),
(96028, 0, 137378, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Bottled Hurricane'),
(96028, 0, 134497, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Stormcharged Choker'),
(96028, 0, 137381, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Pact of Vengeful Service'),
(96028, 0, 134532, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Band of Fused Coral'),
(96028, 0, 134512, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Casque of the Deep'),
(96028, 0, 134459, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Cuffs of the Arcane Storm'),
(96028, 0, 134417, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Slippers of Martyrdom'),
(96028, 0, 134478, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Thundercrush Pauldrons'),
(96028, 0, 134237, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Brinewashed Leather Boots'),
(96028, 0, 137380, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Rage of the Tides'),
(96028, 0, 134225, 0, 0, 1, 1, 1, 1, 'Wrath of Azshara - Coralplate Girdle');
UPDATE `creature_template_difficulty` SET `LootID` = 96028 WHERE `Entry` = 96028 AND `DifficultyID` = 0;
