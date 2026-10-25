-- Frozen in Time (39685), the first of the Demon Ward's three quests after Grand Theft Felbat, had no one to give or end
-- it and nothing in the Vault of Ice. As in the retail video (youtu.be/sEpG8BpsXjQ):
-- - Kor'vas Bloodthorn gives it in the Demon Ward and sends the player down to help Kayn (quest_frozen_in_time);
--   Allari the Souleater, beside her, takes it in.
-- - In the Vault of Ice, Kayn and Freed Illidari hold off Ash'golm on the platform in the middle (squad fighters; he
--   cannot be brought down, npc_frozen_in_time_ashgolm). His Lava Wreath drops patches of lava to dodge
--   (at_frozen_in_time_lava). Kayn calls for the countermeasures. The bridge onto the platform is where the dungeon
--   has it.
-- - The three Countermeasures around the room are activated with a cast (Activate Countermeasure, 204588, their
--   spellclick); each is its own objective's credit (spell_frozen_in_time_activate_countermeasure). Once all three are
--   on, the platform's icicles close over Ash'golm for the player (their state, set per player) and they see him
--   frozen (his flameless self under Time Stop, npc_frozen_in_time_ashgolm_frozen), and Kayn leaves.
-- - The countermeasures' objectives are tracked on the minimap (gold dots, as in the video).

DELETE FROM `creature_queststarter` WHERE `quest` = 39685;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(97643, 39685, 0);
DELETE FROM `creature_questender` WHERE `quest` = 39685;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(96675, 39685, 0);

DELETE FROM `quest_template_addon` WHERE `ID` = 39685;
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`, `ScriptName`) VALUES
(39685, 39682, 'quest_frozen_in_time');

UPDATE `quest_objectives` SET `Flags` = `Flags` | 1 WHERE `ID` IN (279877, 279932, 279933);

-- Kor'vas's lines on accepting it, Kayn's in the Vault of Ice (retail lines, voice files vo_701_korvas_bloodthorn /
-- vo_703_korvas_bloodthorn / vo_701_kayn_sunfury)
DELETE FROM `creature_text` WHERE (`CreatureID` = 97643 AND `GroupID` IN (1, 2)) OR `CreatureID` = 99228;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(97643, 1, 0, 'Aside from us, the most powerful prisoners were sealed on this level. WERE being the operative word.', 12, 0, 100, 0, 0, 57324, 0, 99532, 0, 'Kor''vas Bloodthorn - Frozen in Time accepted'),
(97643, 2, 0, 'We cannot afford to lose Kayn and Altruis here. Waste no time and help them, quickly!', 12, 0, 100, 0, 0, 68365, 0, 103140, 0, 'Kor''vas Bloodthorn - Frozen in Time accepted'),
(99228, 0, 0, 'He is too strong! Our only hope is to reactivate the orbs and seal him inside the chamber!', 12, 0, 100, 0, 0, 57305, 0, 98920, 0, 'Kayn Sunfury - player on Frozen in Time arrives'),
(99228, 1, 0, 'Hold fast, Illidari! We must trap him in this chamber!', 12, 0, 100, 0, 0, 57331, 0, 101463, 0, 'Kayn Sunfury - fighting Ash''golm'),
(99228, 1, 1, 'Hurry, $n! Activate the countermeasures around the room!', 12, 0, 100, 0, 0, 57332, 0, 101464, 0, 'Kayn Sunfury - fighting Ash''golm'),
(99228, 1, 2, 'We cannot hold out like this forever. We need to contain him!', 12, 0, 100, 0, 0, 57286, 0, 101466, 0, 'Kayn Sunfury - fighting Ash''golm'),
(99228, 2, 0, 'Well done! The monster is contained. Let''s meet up with the others.', 12, 0, 100, 0, 0, 57333, 0, 101469, 0, 'Kayn Sunfury - Frozen in Time complete');

-- Ash'golm fights the Illidari and the player; Kayn fights with the Freed Illidari, with their warglaives
UPDATE `creature_template` SET `faction` = 16, `ScriptName` = 'npc_frozen_in_time_ashgolm' WHERE `entry` = 96681;
UPDATE `creature_template` SET `faction` = 2804 WHERE `entry` = 99228;
DELETE FROM `creature_equip_template` WHERE `CreatureID` = 99228;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(99228, 1, 128359, 0, 0, 128371, 0, 0, 0, 0, 0, 0);

-- Ash'golm frozen: his flameless self, held by Time Stop
UPDATE `creature_template` SET `ScriptName` = 'npc_frozen_in_time_ashgolm_frozen' WHERE `entry` = 101591;
DELETE FROM `creature_template_addon` WHERE `entry` IN (101591, 99709, 99731, 99732);
INSERT INTO `creature_template_addon` (`entry`, `auras`) VALUES
(101591, '195289'),
-- The countermeasures' orbs
(99709, '195189'),
(99731, '195189'),
(99732, '195189');

-- The countermeasures: their models are a placeholder and the invisible stalker; only the stalker is kept, the orb is
-- their aura
DELETE FROM `creature_template_model` WHERE `CreatureID` IN (99709, 99731, 99732);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(99709, 0, 51861, 1, 25549),
(99731, 0, 51861, 1, 25549),
(99732, 0, 51861, 1, 25549);
UPDATE `creature_template` SET `npcflag` = 16777216, `ScriptName` = '' WHERE `entry` IN (99709, 99731, 99732);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (99709, 99731, 99732);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(99709, 204588, 1, 0),
(99731, 204588, 1, 0),
(99732, 204588, 1, 0);
DELETE FROM `spell_script_names` WHERE `spell_id` = 204588;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(204588, 'spell_frozen_in_time_activate_countermeasure');

-- Clickable while that countermeasure's objective is open
DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId` = 18 AND `SourceGroup` IN (99709, 99731, 99732)) OR (`SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 195213);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 99709, 204588, 0, 0, 47, 0, 39685, 8, 0, '', 0, 0, 0, '', 'Eastern Countermeasure: clickable on Frozen in Time'),
(18, 99709, 204588, 0, 0, 48, 0, 279877, 0, 0, '', 0, 0, 0, '', 'Eastern Countermeasure: until it is activated'),
(18, 99731, 204588, 0, 0, 47, 0, 39685, 8, 0, '', 0, 0, 0, '', 'Southern Countermeasure: clickable on Frozen in Time'),
(18, 99731, 204588, 0, 0, 48, 0, 279932, 0, 0, '', 0, 0, 0, '', 'Southern Countermeasure: until it is activated'),
(18, 99732, 204588, 0, 0, 47, 0, 39685, 8, 0, '', 0, 0, 0, '', 'Western Countermeasure: clickable on Frozen in Time'),
(18, 99732, 204588, 0, 0, 48, 0, 279933, 0, 0, '', 0, 0, 0, '', 'Western Countermeasure: until it is activated');

-- Lava Wreath's patches of lava (192571 makes 4919): Lava on entering; their size is ours, the server has no data for
-- them. 192571's own visual draws nothing, so they borrow Lava Extrusion's (207298), the magma ground patch
-- (spells/magmazone.m2)
DELETE FROM `areatrigger_template` WHERE `Id` = 1468001 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1468001, 1, 0, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` = 4919 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `AnimId`, `Speed`, `DecalPropertiesId`, `SpellForVisuals`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(4919, 0, 1468001, 1, 0, -1, 1, 0, 207298, 0, 4, 4, 'at_frozen_in_time_lava', 0);

-- The fight is the player's until the countermeasures are on; then Ash'golm is frozen in the platform's ice for them.
-- The Freed Illidari (96656) also stand elsewhere in the Vault: only those by Ash'golm go.
DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (96681, 101591, 99228, 96656))
    OR (`SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 5 AND `SourceEntry` = 246418);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 96681, 0, 0, 47, 0, 39685, 9, 0, '', 0, 0, 0, '', 'Ash''golm: fighting until Frozen in Time is complete'),
(32, 3, 99228, 0, 0, 47, 0, 39685, 9, 0, '', 0, 0, 0, '', 'Kayn (Vault of Ice): until Frozen in Time is complete'),
(32, 3, 96656, 0, 0, 29, 1, 96681, 30, 0, '', 1, 0, 0, '', 'Freed Illidari: not by Ash''golm'),
(32, 3, 96656, 0, 1, 29, 1, 96681, 30, 0, '', 0, 0, 0, '', 'Freed Illidari (Vault of Ice): by Ash''golm'),
(32, 3, 96656, 0, 1, 47, 0, 39685, 9, 0, '', 0, 0, 0, '', 'Freed Illidari (Vault of Ice): until Frozen in Time is complete'),
(32, 3, 101591, 0, 0, 47, 0, 39685, 66, 0, '', 0, 0, 0, '', 'Ash''golm (frozen): once Frozen in Time is complete'),
(32, 5, 246418, 0, 0, 47, 0, 39685, 74, 0, '', 0, 0, 0, '', 'Vault of Ice icicles: from taking Frozen in Time');

DELETE FROM `creature` WHERE `guid` BETWEEN 14680260 AND 14680270;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680260, 96675, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4458.40, -393.00, 126.32, 1.6313, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Allari the Souleater, Demon Ward
(14680261, 96681, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4239.14, -451.33, 106.09, 6.2702, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', 'VaultOfIceSquad', 0), -- Ash'golm, Vault of Ice
(14680262, 101591, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4239.14, -451.33, 106.09, 6.2702, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Ash'golm (frozen), Vault of Ice
(14680263, 99228, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4246.00, -451.30, 106.10, 3.1416, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfIceSquad', 0), -- Kayn Sunfury, Vault of Ice
(14680264, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4234.00, -445.50, 106.09, 5.7000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfIceSquad', 0), -- Freed Illidari, Vault of Ice
(14680265, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4233.50, -457.00, 106.09, 0.7000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfIceSquad', 0), -- Freed Illidari, Vault of Ice
(14680266, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4240.00, -458.50, 106.09, 1.8000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfIceSquad', 0), -- Freed Illidari, Vault of Ice
(14680267, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4240.50, -444.00, 106.09, 4.5000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfIceSquad', 0), -- Freed Illidari, Vault of Ice
(14680268, 99709, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4245.00, -487.00, 106.61, 1.5708, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Eastern Countermeasure (its quest_poi point)
(14680269, 99731, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4203.00, -452.00, 106.62, 0.0000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Southern Countermeasure (its quest_poi point)
(14680270, 99732, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4248.00, -420.00, 106.50, 4.7124, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Western Countermeasure (its quest_poi point)

-- The platform's icicles and the bridge from the stairs onto the platform, where the Vault of the Wardens dungeon has
-- them. The icicles are not for clicking: they close for a player by script, a state kept for as long as their
-- respawn time (a week here)
DELETE FROM `gameobject_template_addon` WHERE `entry` = 246418;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`) VALUES
(246418, 0, 16);
DELETE FROM `gameobject` WHERE `guid` IN (14680210, 14680211);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680210, 246418, 1468, 7814, 0, '0', 0, 0, 0, -1, 4238.58, -451.16, 105.86, 3.1416, 0, 0, 1, 0, 604800, 255, 1, '', NULL, 0),
(14680211, 246057, 1468, 7814, 0, '0', 0, 0, 0, -1, 4293.66, -451.205, 104.521, 3.14159, 0, 0, -1, 0, 120, 255, 1, '', NULL, 0);
DELETE FROM `gameobject_addon` WHERE `guid` = 14680211;
INSERT INTO `gameobject_addon` (`guid`, `parent_rotation0`, `parent_rotation1`, `parent_rotation2`, `parent_rotation3`, `invisibilityType`, `invisibilityValue`, `WorldEffectID`, `AIAnimKitID`) VALUES
(14680211, 0, 0, 1, -0.0000000437114, 0, 0, 0, 0);
