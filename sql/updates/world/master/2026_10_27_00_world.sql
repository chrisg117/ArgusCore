-- Between Us and Freedom (Havoc 39688 Alliance / 39694 Horde, Vengeance 40255 Alliance / 40256 Horde), after
-- A New Direction, had no one to give it, no Bastillax and nowhere to end. As in the retail video
-- (youtu.be/Yz1w86hv2XU) and the quest data:
-- - Kor'vas Bloodthorn gives it at the Pool of Judgment (Wowhead), the spec's version by the Fel Secrets choice, as for
--   Forged in Fire. On accepting she sends the player on (retail line; quest_between_us_and_freedom).
-- - Bastillax (96783) waits in the Chamber of Night, at the objective's point. He fights with the abilities Wowhead
--   lists for him and the Mephistos voice lines the client has for him (npc_between_us_and_freedom_bastillax): Fel
--   Annihilation (a cone in front, to dodge), Crushing Shadows (knocks everyone up) and Blur of Shadows (he dodges
--   everything and answers with Shadow Blast; spell_bastillax_blur_of_shadows). Portal Surge is left out: what it calls
--   is not in the data. The follower the player chose (Kayn 102393 / Altruis 102391, both Wowhead-pinned in the
--   Vault) fights beside the player, but cannot bring him down.
-- - Those who killed him channel Taking Power on his corpse, for Blur (Havoc) or Torment (Vengeance); its end gives
--   the credit (210499 / 210502, spell_between_us_and_freedom_taking_power).
-- - The chosen follower waits outside the Vault's exit, at the turn-in's point, and ends it. Coming out with Bastillax
--   dead, the follower cheers and Archmage Khadgar arrives: the optional "Khadgar arrived" (at_between_us_and_freedom_
--   vault_exit). Khadgar gives the next quest, which is not built yet.
-- Spawn visibility is per entry in this core, so the enders 96666 / 96669 also stay by the pool, 400 yards away.

DELETE FROM `creature_queststarter` WHERE `quest` IN (39688, 39694, 40255, 40256);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(97644, 39688, 0),
(97644, 39694, 0),
(97644, 40255, 0),
(97644, 40256, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (39688, 39694, 40255, 40256);
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`, `ScriptName`) VALUES
(39688, 40373, 'quest_between_us_and_freedom'),
(39694, 40373, 'quest_between_us_and_freedom'),
(40255, 40373, 'quest_between_us_and_freedom'),
(40256, 40373, 'quest_between_us_and_freedom');

-- Havoc or Vengeance, by the Fel Secrets choice (the races are in the quests' own data)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` IN (39688, 39694, 40255, 40256);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 39688, 0, 0, 8, 0, 39516, 0, 0, '', 0, 0, 0, '', 'Between Us and Freedom (Havoc): Havoc chosen'),
(19, 0, 39694, 0, 0, 8, 0, 39516, 0, 0, '', 0, 0, 0, '', 'Between Us and Freedom (Havoc): Havoc chosen'),
(19, 0, 40255, 0, 0, 8, 0, 39515, 0, 0, '', 0, 0, 0, '', 'Between Us and Freedom (Vengeance): Vengeance chosen'),
(19, 0, 40256, 0, 0, 8, 0, 39515, 0, 0, '', 0, 0, 0, '', 'Between Us and Freedom (Vengeance): Vengeance chosen');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (210499, 210502, 200002);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210499, 'spell_between_us_and_freedom_taking_power'),
(210502, 'spell_between_us_and_freedom_taking_power'),
(200002, 'spell_bastillax_blur_of_shadows');

-- Blur of Shadows answers only what it dodges
DELETE FROM `spell_proc` WHERE `SpellId` = 200002;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `SpellFamilyMask3`, `ProcFlags`, `ProcFlags2`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(200002, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10, 0, 0, 0, 0, 0, 0);

UPDATE `creature_template` SET `faction` = 16, `ScriptName` = 'npc_between_us_and_freedom_bastillax' WHERE `entry` = 96783;
UPDATE `creature_template` SET `faction` = 2804 WHERE `entry` IN (102391, 102393);

-- Retail lines: Kor'vas (vo_701_korvas; which file is this line's is unknown, so no sound), Bastillax (vo_701_mephistos),
-- Kayn / Altruis (vo_701), Khadgar
DELETE FROM `creature_text` WHERE (`CreatureID` = 97644 AND `GroupID` = 1) OR (`CreatureID` IN (96666, 96669) AND `GroupID` = 2) OR `CreatureID` IN (96783, 97978);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(97644, 1, 0, 'Let''s keep moving. The others are in the chamber just ahead.', 12, 0, 100, 0, 0, 0, 0, 99645, 0, 'Kor''vas Bloodthorn - Between Us and Freedom accepted'),
(96783, 0, 0, 'Impressive you have made it this far, demon hunters. Now, I will extinguish your souls like a candle''s flame!', 14, 0, 100, 0, 0, 57362, 0, 99652, 0, 'Bastillax - aggro'),
(96783, 1, 0, 'You will never see the light of this world again.', 14, 0, 100, 0, 0, 57310, 0, 103143, 0, 'Bastillax - in combat'),
(96783, 2, 0, 'Freedom is but a hopeless dream.', 14, 0, 100, 0, 0, 57311, 0, 103144, 0, 'Bastillax - in combat'),
(96783, 3, 0, 'Such arrogance.... To think you can stand against the Legion!', 14, 0, 100, 0, 0, 57385, 0, 103145, 0, 'Bastillax - in combat'),
(96783, 4, 0, 'No... noooo! You shall never be free...', 14, 0, 100, 0, 0, 57313, 0, 101957, 0, 'Bastillax - death'),
(96666, 2, 0, 'We''ve done it, Illidari! FREEDOM IS OURS!', 14, 0, 100, 0, 0, 57363, 0, 99653, 0, 'Kayn Sunfury - out of the Vault'),
(96669, 2, 0, 'We''ve done it, Illidari! FREEDOM IS OURS!', 14, 0, 100, 0, 0, 57366, 0, 99008, 0, 'Altruis the Sufferer - out of the Vault'),
(97978, 0, 0, 'I am Archmage Khadgar, leader of the Kirin Tor.', 12, 0, 100, 0, 0, 57302, 0, 99656, 0, 'Archmage Khadgar - arrived at the Vault'),
(97978, 1, 0, 'Demon hunters, Azeroth needs your help.', 12, 0, 100, 0, 0, 57364, 0, 99658, 0, 'Archmage Khadgar - arrived at the Vault');

-- Who sees whom: Bastillax until Between Us and Freedom is turned in; the follower fighting beside the player is the
-- one chosen in A New Direction (its tracking quest, rewarded on the choice); Khadgar from his arrival on.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (96783, 102391, 102393, 97978);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 96783, 0, 0, 47, 0, 39688, 64, 0, '', 1, 0, 0, '', 'Bastillax: Between Us and Freedom (Havoc, A) not turned in'),
(32, 3, 96783, 0, 0, 47, 0, 39694, 64, 0, '', 1, 0, 0, '', 'Bastillax: Between Us and Freedom (Havoc, H) not turned in'),
(32, 3, 96783, 0, 0, 47, 0, 40255, 64, 0, '', 1, 0, 0, '', 'Bastillax: Between Us and Freedom (Vengeance, A) not turned in'),
(32, 3, 96783, 0, 0, 47, 0, 40256, 64, 0, '', 1, 0, 0, '', 'Bastillax: Between Us and Freedom (Vengeance, H) not turned in'),
(32, 3, 102393, 0, 0, 47, 0, 40374, 64, 0, '', 0, 0, 0, '', 'Kayn (Chamber of Night): Kayn chosen'),
(32, 3, 102391, 0, 0, 47, 0, 40375, 64, 0, '', 0, 0, 0, '', 'Altruis (Chamber of Night): Altruis chosen'),
(32, 3, 97978, 0, 0, 48, 0, 286671, 0, 1, '', 0, 0, 0, '', 'Khadgar (Vault exit): arrived (Havoc, A)'),
(32, 3, 97978, 0, 1, 48, 0, 286673, 0, 1, '', 0, 0, 0, '', 'Khadgar (Vault exit): arrived (Havoc, H)'),
(32, 3, 97978, 0, 2, 48, 0, 286672, 0, 1, '', 0, 0, 0, '', 'Khadgar (Vault exit): arrived (Vengeance, A)'),
(32, 3, 97978, 0, 3, 48, 0, 286674, 0, 1, '', 0, 0, 0, '', 'Khadgar (Vault exit): arrived (Vengeance, H)'),
(32, 3, 97978, 0, 4, 47, 0, 39688, 64, 0, '', 0, 0, 0, '', 'Khadgar (Vault exit): or Between Us and Freedom turned in'),
(32, 3, 97978, 0, 5, 47, 0, 39694, 64, 0, '', 0, 0, 0, '', 'Khadgar (Vault exit): or Between Us and Freedom turned in'),
(32, 3, 97978, 0, 6, 47, 0, 40255, 64, 0, '', 0, 0, 0, '', 'Khadgar (Vault exit): or Between Us and Freedom turned in'),
(32, 3, 97978, 0, 7, 47, 0, 40256, 64, 0, '', 0, 0, 0, '', 'Khadgar (Vault exit): or Between Us and Freedom turned in');

-- Bastillax in the middle of the Chamber of Night, facing the way from the pool, and the chosen follower before him;
-- outside the exit, the follower who ends the quest and the spot where Khadgar arrives (the next quest's turn-in point)
DELETE FROM `creature` WHERE `guid` BETWEEN 14680283 AND 14680288;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680283, 96783, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4185.00, -628.00, 255.49, 1.0600, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Bastillax, Chamber of Night
(14680284, 102393, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4205.00, -618.00, 255.50, 3.6000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', NULL, 0), -- Kayn Sunfury, Chamber of Night
(14680285, 102391, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4205.00, -618.00, 255.50, 3.6000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', NULL, 0), -- Altruis the Sufferer, Chamber of Night
(14680286, 96666, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4161.00, -856.00, 291.30, 1.3600, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kayn Sunfury, Vault exit
(14680287, 96669, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4161.00, -856.00, 291.30, 1.3600, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Altruis the Sufferer, Vault exit
(14680288, 97978, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4187.00, -846.00, 293.12, 3.5100, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Archmage Khadgar, Vault exit

-- Coming out of the Vault (server-side)
DELETE FROM `areatrigger_template` WHERE `Id` = 1468104 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1468104, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` = 1468104 AND `IsCustom` = 1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(1468104, 1, 1468104, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 20, 20, 0, 0, 0, 0, 'at_between_us_and_freedom_vault_exit', 0);
DELETE FROM `areatrigger` WHERE `SpawnId` = 1468003;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1468003, 1468104, 1, 1468, '0', 4168.00, -845.00, 292.00, 0, 0, 0, 0, '', 'Vault of the Wardens - outside the exit (Quest: "Between Us and Freedom" - Khadgar arrived)', 0);
