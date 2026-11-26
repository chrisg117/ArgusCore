-- Eye of Azshara: Serpentrix (encounter 1813).
-- Spec: docs/superpowers/specs/2026-11-16-dungeon-eye-of-azshara-design.md, "Serpentrix" and its amendment.

UPDATE `creature_template` SET `ScriptName` = 'boss_serpentrix' WHERE `entry` = 91808;
UPDATE `creature_template` SET `ScriptName` = 'npc_serpentrix_hydra_spawn' WHERE `entry` IN (97259, 97260);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_serpentrix_poison_spit', 'spell_serpentrix_spit_missile', 'spell_eoa_violent_winds');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(192050, 'spell_serpentrix_poison_spit'),
(191839, 'spell_serpentrix_spit_missile'),
(191850, 'spell_serpentrix_spit_missile'),
(191797, 'spell_eoa_violent_winds');

-- Submerge (99006) as a boss emote (41): DBM finds |Hspell:191873|h in it. No voiced lines.
DELETE FROM `creature_text` WHERE `CreatureID` = 91808;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 91808, 0, 0, IF(b.`Text` <> '', b.`Text`, b.`Text1`), 41, 0, 100, 0, 0, 0, 0, b.`ID`, 0, 'Serpentrix - Submerge'
FROM `hotfixes`.`broadcast_text` b WHERE b.`ID` = 99006;

-- Submerge teleports to a database destination 50-200 yd away (TARGET_DEST_NEARBY_DB on effect 1):
-- the six Sinkhole stalkers (guids 6003929, 6003944, 6003945, 6003976, 6003980, 6003990).
DELETE FROM `spell_target_position` WHERE `ID` = 191873;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(191873, 1, 0, 1456, -3294.20, 4460.52, -0.63, 3.93, 0),
(191873, 1, 1, 1456, -3304.17, 4405.53, 0.09, 3.25, 0),
(191873, 1, 2, 1456, -3246.71, 4479.65, 0.27, 3.29, 0),
(191873, 1, 3, 1456, -3192.24, 4440.18, -0.65, 3.35, 0),
(191873, 1, 4, 1456, -3256.36, 4370.39, 0.37, 2.72, 0),
(191873, 1, 5, 1456, -3199.40, 4384.95, 0.16, 3.03, 0);

-- Toxic Puddle 4856: sphere, radius 3 (calibrate; the client has no shape).
DELETE FROM `areatrigger_template` WHERE `Id` = 4856 AND `IsCustom` = 0;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(4856, 0, 0, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` = 4856 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(4856, 0, 4856, 0, 0, 0, 3, 3, 'at_serpentrix_toxic_puddle', 0);

-- Loot: the 11 journal items (JournalEncounter 1479), one group with equal chance.
DELETE FROM `creature_loot_template` WHERE `Entry` = 91808;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(91808, 0, 134508, 0, 0, 1, 1, 1, 1, 'Serpentrix - Stormwake Handguards'),
(91808, 0, 137375, 0, 0, 1, 1, 1, 1, 'Serpentrix - Blazing Hydra Flame Sac'),
(91808, 0, 134239, 0, 0, 1, 1, 1, 1, 'Serpentrix - Brinewashed Leather Grips'),
(91808, 0, 134256, 0, 0, 1, 1, 1, 1, 'Serpentrix - Sea Stalker''s Leggings'),
(91808, 0, 137377, 0, 0, 1, 1, 1, 1, 'Serpentrix - Serpentrix''s Guile'),
(91808, 0, 134438, 0, 0, 1, 1, 1, 1, 'Serpentrix - Tunic of the Pitiless Monstrosity'),
(91808, 0, 137372, 0, 0, 1, 1, 1, 1, 'Serpentrix - Wristbands of the Swirling Deeps'),
(91808, 0, 134228, 0, 0, 1, 1, 1, 1, 'Serpentrix - Coralplate Pauldrons'),
(91808, 0, 137373, 0, 0, 1, 1, 1, 1, 'Serpentrix - Tempered Egg of Serpentrix'),
(91808, 0, 134260, 0, 0, 1, 1, 1, 1, 'Serpentrix - Seawitch Hood'),
(91808, 0, 134465, 0, 0, 1, 1, 1, 1, 'Serpentrix - Hydra Scale Sabatons');
UPDATE `creature_template_difficulty` SET `LootID` = 91808 WHERE `Entry` = 91808 AND `DifficultyID` = 0;
