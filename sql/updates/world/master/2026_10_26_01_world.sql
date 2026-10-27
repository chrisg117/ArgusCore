-- Forged in Fire (39683 Havoc, 40254 Vengeance), the third of the Demon Ward's quests after Grand Theft Felbat, had no
-- one to give it and no Immolanth. As in the retail video (youtu.be/qkV3pFgvj9g):
-- - Allari the Souleater gives it in the Demon Ward, in the version of the player's Fel Secrets path, and takes it in.
-- - Immolanth waits in the Vault of Law and taunts the demon hunters who fight him (npc_forged_in_fire_immolanth).
-- - Those who kill him channel Taking Power on his corpse for Chaos Nova (210485, Havoc) or Immolation Aura (210489,
--   Vengeance); its end gives the credit (spell_forged_in_fire_taking_power, Mardum's Taking Power scripts).
-- - He is gone for the player once the quest is turned in.

DELETE FROM `creature_queststarter` WHERE `quest` IN (39683, 40254);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(96675, 39683, 0),
(96675, 40254, 0);
DELETE FROM `creature_questender` WHERE `quest` IN (39683, 40254);
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(96675, 39683, 0),
(96675, 40254, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (39683, 40254);
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(39683, 39682),
(40254, 39682);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` IN (39683, 40254);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 39683, 0, 0, 8, 0, 39516, 0, 0, '', 0, 0, 0, '', 'Forged in Fire (Havoc: Cry Havoc done)'),
(19, 0, 40254, 0, 0, 8, 0, 39515, 0, 0, '', 0, 0, 0, '', 'Forged in Fire (Vengeance: Vengeance Will Be Mine! done)');

-- Taking Power on Immolanth's corpse
DELETE FROM `spell_script_names` WHERE `spell_id` IN (210485, 210489);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210485, 'spell_forged_in_fire_taking_power'),
(210489, 'spell_forged_in_fire_taking_power');

-- Immolanth's lines (retail lines, voice files vo_701_immolanth)
DELETE FROM `creature_text` WHERE `CreatureID` = 96682;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(96682, 0, 0, 'I will burn this world with my fury!', 12, 0, 100, 0, 0, 57337, 0, 98732, 0, 'Immolanth - aggro'),
(96682, 1, 0, 'Join me, demon hunters. I will increase your power beyond imagining!', 12, 0, 100, 0, 0, 57358, 0, 101410, 0, 'Immolanth - in combat'),
(96682, 2, 0, 'The wardens would keep us rotting here forever. Why do you fight on their side?', 12, 0, 100, 0, 0, 57359, 0, 101412, 0, 'Immolanth - in combat');

UPDATE `creature_template` SET `faction` = 16, `ScriptName` = 'npc_forged_in_fire_immolanth' WHERE `entry` = 96682;

-- Gone for the player once either version is turned in
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 96682;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 96682, 0, 0, 47, 0, 39683, 64, 0, '', 1, 0, 0, '', 'Immolanth: until Forged in Fire (Havoc) is turned in'),
(32, 3, 96682, 0, 0, 47, 0, 40254, 64, 0, '', 1, 0, 0, '', 'Immolanth: until Forged in Fire (Vengeance) is turned in');

-- Immolanth, in the Vault of Law at the quest's objective point, facing the way in
DELETE FROM `creature` WHERE `guid` = 14680278;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680278, 96682, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4665.00, -454.00, 122.09, 3.1416, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Immolanth, Vault of Law
