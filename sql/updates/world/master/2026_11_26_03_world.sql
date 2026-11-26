-- Eye of Azshara: King Deepbeard (encounter 1812).
-- Spec: docs/superpowers/specs/2026-11-16-dungeon-eye-of-azshara-design.md, "King Deepbeard" and its amendment.

UPDATE `creature_template` SET `ScriptName` = 'boss_king_deepbeard' WHERE `entry` = 91797;
-- The Quake stalker is a player's wild summon (193153, properties 64) that casts its damage at the stalker's
-- enemies: faction 16 (the script makes it passive and unattackable).
UPDATE `creature_template` SET `ScriptName` = 'npc_deepbeard_quake', `faction` = 16 WHERE `entry` = 97916;

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_deepbeard_gaseous_bubbles', 'spell_deepbeard_ground_slam');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(193018, 'spell_deepbeard_gaseous_bubbles'),
(193093, 'spell_deepbeard_ground_slam');

-- Lines: client broadcast texts. Yells (14); the Call the Seas emote is a boss emote (41).
DELETE FROM `creature_text` WHERE `CreatureID` = 91797;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 91797, c.`grp`, c.`idx`, IF(b.`Text` <> '', b.`Text`, b.`Text1`), c.`type`, 0, 100, 0, 0, IF(b.`SoundKitID1` <> 0, b.`SoundKitID1`, b.`SoundKitID2`), 0, b.`ID`, 0, c.`cmt`
FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 0 AS grp, 0 AS idx, 102315 AS bid, 14 AS type, 'King Deepbeard - aggro' AS cmt
  UNION ALL SELECT 0, 1, 102316, 14, 'King Deepbeard - aggro'
  UNION ALL SELECT 1, 0, 102303, 14, 'King Deepbeard - Call the Seas'
  UNION ALL SELECT 2, 0, 102304, 14, 'King Deepbeard - Gaseous Bubbles'
  UNION ALL SELECT 2, 1, 102305, 14, 'King Deepbeard - Gaseous Bubbles'
  UNION ALL SELECT 2, 2, 102306, 14, 'King Deepbeard - Gaseous Bubbles'
  UNION ALL SELECT 3, 0, 102307, 14, 'King Deepbeard - Quake'
  UNION ALL SELECT 3, 1, 102308, 14, 'King Deepbeard - Quake'
  UNION ALL SELECT 4, 0, 102313, 14, 'King Deepbeard - killed a player'
  UNION ALL SELECT 4, 1, 102314, 14, 'King Deepbeard - killed a player'
  UNION ALL SELECT 5, 0, 102317, 14, 'King Deepbeard - wipe'
  UNION ALL SELECT 6, 0, 102309, 14, 'King Deepbeard - death, Serpentrix (the winds) still alive'
  UNION ALL SELECT 6, 1, 102310, 14, 'King Deepbeard - death, Serpentrix (the winds) still alive'
  UNION ALL SELECT 7, 0, 102311, 14, 'King Deepbeard - death, only the Wrath left'
  UNION ALL SELECT 7, 1, 102312, 14, 'King Deepbeard - death, only the Wrath left'
  UNION ALL SELECT 8, 0, 99824, 41, 'King Deepbeard - Call the Seas boss emote'
) c ON b.`ID` = c.`bid`;

-- Call the Seas finds the 35 stalkers by entry; each runs 193054 (area trigger 4968, 20 s).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 193051;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 193051, 0, 0, 31, 0, 3, 97844, 0, 0, 0, 0, '', 'Call the Seas - its stalkers');

-- Area trigger 4968: sphere, radius 4 (calibrate; the client has no shape).
DELETE FROM `areatrigger_template` WHERE `Id` = 4968 AND `IsCustom` = 0;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(4968, 0, 0, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` = 4968 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(4968, 0, 4968, 0, 0, 0, 4, 4, 'at_deepbeard_call_the_seas', 0);

-- Loot: the 9 journal items (JournalEncounter 1491), one group with equal chance.
DELETE FROM `creature_loot_template` WHERE `Entry` = 91797;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(91797, 0, 134406, 0, 0, 1, 1, 1, 1, 'King Deepbeard - Mainsail Cloak'),
(91797, 0, 134539, 0, 0, 1, 1, 1, 1, 'King Deepbeard - Braided Silver Ring'),
(91797, 0, 134428, 0, 0, 1, 1, 1, 1, 'King Deepbeard - Rising Ocean Legwraps'),
(91797, 0, 137369, 0, 0, 1, 1, 1, 1, 'King Deepbeard - Giant Ornamental Pearl'),
(91797, 0, 137368, 0, 0, 1, 1, 1, 1, 'King Deepbeard - Breastplate of Ten Lashes'),
(91797, 0, 134456, 0, 0, 1, 1, 1, 1, 'King Deepbeard - Taut Halyard Waistband'),
(91797, 0, 137370, 0, 0, 1, 1, 1, 1, 'King Deepbeard - Heart of the Sea'),
(91797, 0, 137371, 0, 0, 1, 1, 1, 1, 'King Deepbeard - Tumultuous Aftershock'),
(91797, 0, 134514, 0, 0, 1, 1, 1, 1, 'King Deepbeard - Keelhauler Legplates');
UPDATE `creature_template_difficulty` SET `LootID` = 91797 WHERE `Entry` = 91797 AND `DifficultyID` = 0;
