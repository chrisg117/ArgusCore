-- Grand Theft Felbat (39682), after Stop Gul'dan!, had no one to give it and no felbat to ride.
-- - Kayn gives it in the Vault of the Betrayer; he and Altruis argue about the Warden as the player sets off
--   (quest_grand_theft_felbat).
-- - On the way, through the passage north of the Tomb, Lyana Darksorrow and Freed Illidari hold off demons (squad
--   fighters), at their Wowhead pins.
-- - By the elevator shaft, Jace Darkweaver sends the player on; clicking the Vampiric Felbat (195537) summons one that
--   carries the player up the shaft to the Demon Ward (vehicle 1907, one seat the rider can neither steer nor leave:
--   retail's is unknown; npc_grand_theft_felbat_vampiric_felbat), with Bastillax taunting on the way (talking head, our
--   conversation id).
-- - Kor'vas Bloodthorn ends it in the Demon Ward; its reward spell (195567, missing from the client and the server)
--   binds the hearthstone there ("The Demon Ward is now your home").

DELETE FROM `creature_queststarter` WHERE `quest` = 39682;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(96665, 39682, 0);

DELETE FROM `quest_template_addon` WHERE `ID` = 39682;
INSERT INTO `quest_template_addon` (`ID`, `ScriptName`) VALUES
(39682, 'quest_grand_theft_felbat');

DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId` = 19 AND `SourceEntry` = 39682) OR (`SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 96659) OR (`SourceTypeOrReferenceId` = 22 AND `SourceEntry` IN (97070, 97643));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 39682, 0, 0, 8, 0, 38723, 0, 0, '', 0, 0, 0, '', 'Grand Theft Felbat: Stop Gul''dan! (Havoc) done'),
(19, 0, 39682, 0, 1, 8, 0, 40253, 0, 0, '', 0, 0, 0, '', 'Grand Theft Felbat: Stop Gul''dan! (Vengeance) done'),
(18, 96659, 195537, 0, 0, 47, 0, 39682, 8, 0, '', 0, 0, 0, '', 'Vampiric Felbat: rideable on Grand Theft Felbat'),
(22, 1, 97070, 0, 0, 47, 0, 39682, 8, 0, '', 0, 0, 0, '', 'Jace Darkweaver: sends players on Grand Theft Felbat'),
(22, 1, 97643, 0, 0, 47, 0, 39682, 10, 0, '', 0, 0, 0, '', 'Kor''vas Bloodthorn: greets players on Grand Theft Felbat');

-- Kayn (96665) stands in the Tomb and in the Vault of the Betrayer, told apart by the portals there (99501); the Vault of
-- the Betrayer's Kayn and Altruis are gone for the player once Grand Theft Felbat is taken (they run ahead)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (96665, 92985);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 96665, 0, 0, 47, 0, 38672, 64, 0, '', 0, 0, 0, '', 'Kayn (Tomb of the Penitent): once Breaking Out is done'),
(32, 3, 96665, 0, 0, 29, 1, 99501, 40, 0, '', 1, 0, 0, '', 'Kayn (Tomb of the Penitent): not by the portals'),
(32, 3, 96665, 0, 1, 29, 1, 99501, 40, 0, '', 0, 0, 0, '', 'Kayn (Vault of the Betrayer): by the portals'),
(32, 3, 96665, 0, 1, 47, 0, 39682, 1, 0, '', 0, 0, 0, '', 'Kayn (Vault of the Betrayer): until Grand Theft Felbat is taken'),
(32, 3, 92985, 0, 0, 47, 0, 39682, 1, 0, '', 0, 0, 0, '', 'Altruis (Vault of the Betrayer): until Grand Theft Felbat is taken');

-- The felbat to ride, its vehicle, and the bind of the reward
UPDATE `creature_template` SET `npcflag` = 16777216 WHERE `entry` = 96659;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 96659;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(96659, 195537, 1, 0);
UPDATE `creature_template` SET `VehicleId` = 1907, `ScriptName` = 'npc_grand_theft_felbat_vampiric_felbat' WHERE `entry` = 99443;

-- Kayn and Altruis running ahead to the Tomb, the player's own (quest_grand_theft_felbat), with the warglaives of those
-- in the Vault of the Betrayer
UPDATE `creature_template` SET `ScriptName` = 'npc_grand_theft_felbat_running_ahead' WHERE `entry` IN (97265, 97267);
DELETE FROM `creature_equip_template` WHERE `CreatureID` IN (97265, 97267);
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(97265, 1, 128359, 0, 0, 128371, 0, 0, 0, 0, 0, 0),
(97267, 1, 128359, 0, 0, 128371, 0, 0, 0, 0, 0, 0);

-- Lyana holds the passage with the Freed Illidari, so she takes their faction and warglaives
UPDATE `creature_template` SET `faction` = 2804 WHERE `entry` = 100299;
DELETE FROM `creature_equip_template` WHERE `CreatureID` = 100299;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(100299, 1, 128359, 0, 0, 128371, 0, 0, 0, 0, 0, 0);

DELETE FROM `serverside_spell` WHERE `Id` = 195567;
INSERT INTO `serverside_spell` (`Id`, `DifficultyID`, `CategoryId`, `Dispel`, `Mechanic`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `AttributesEx7`, `AttributesEx8`, `AttributesEx9`, `AttributesEx10`, `AttributesEx11`, `AttributesEx12`, `AttributesEx13`, `AttributesEx14`, `AttributesEx15`, `Stances`, `StancesNot`, `Targets`, `TargetCreatureType`, `RequiresSpellFocus`, `FacingCasterFlags`, `CasterAuraState`, `TargetAuraState`, `ExcludeCasterAuraState`, `ExcludeTargetAuraState`, `CasterAuraSpell`, `TargetAuraSpell`, `ExcludeCasterAuraSpell`, `ExcludeTargetAuraSpell`, `CasterAuraType`, `TargetAuraType`, `ExcludeCasterAuraType`, `ExcludeTargetAuraType`, `CastingTimeIndex`, `RecoveryTime`, `CategoryRecoveryTime`, `StartRecoveryCategory`, `StartRecoveryTime`, `InterruptFlags`, `AuraInterruptFlags1`, `AuraInterruptFlags2`, `ChannelInterruptFlags1`, `ChannelInterruptFlags2`, `ProcFlags`, `ProcFlags2`, `ProcChance`, `ProcCharges`, `ProcCooldown`, `ProcBasePPM`, `MaxLevel`, `BaseLevel`, `SpellLevel`, `DurationIndex`, `RangeIndex`, `Speed`, `LaunchDelay`, `StackAmount`, `EquippedItemClass`, `EquippedItemSubClassMask`, `EquippedItemInventoryTypeMask`, `ContentTuningId`, `SpellName`, `ConeAngle`, `ConeWidth`, `MaxTargetLevel`, `MaxAffectedTargets`, `SpellFamilyName`, `SpellFamilyFlags1`, `SpellFamilyFlags2`, `SpellFamilyFlags3`, `SpellFamilyFlags4`, `DmgClass`, `PreventionType`, `AreaGroupId`, `SchoolMask`, `ChargeCategoryId`) VALUES
(195567, 0, 0, 0, 0, 384, 268435456, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0, 'Grand Theft Felbat: Bind to the Demon Ward', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0);
DELETE FROM `serverside_spell_effect` WHERE `SpellID` = 195567;
INSERT INTO `serverside_spell_effect` (`SpellID`, `EffectIndex`, `DifficultyID`, `Effect`, `EffectAura`, `EffectAmplitude`, `EffectAttributes`, `EffectAuraPeriod`, `EffectBonusCoefficient`, `EffectChainAmplitude`, `EffectChainTargets`, `EffectItemType`, `EffectMechanic`, `EffectPointsPerResource`, `EffectPosFacing`, `EffectRealPointsPerLevel`, `EffectTriggerSpell`, `BonusCoefficientFromAP`, `PvpMultiplier`, `Coefficient`, `Variance`, `ResourceCoefficient`, `GroupSizeBasePointsCoefficient`, `EffectBasePoints`, `EffectMiscValue1`, `EffectMiscValue2`, `EffectRadiusIndex1`, `EffectRadiusIndex2`, `EffectSpellClassMask1`, `EffectSpellClassMask2`, `EffectSpellClassMask3`, `EffectSpellClassMask4`, `ImplicitTarget1`, `ImplicitTarget2`) VALUES
(195567, 0, 0, 11, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7866, 0, 0, 0, 0, 0, 0, 0, 1, 0); -- bind, The Demon Ward

-- Bastillax's taunt on the way up: a talking head (his line 2803 has no conversation on the server; our id)
DELETE FROM `conversation_template` WHERE `Id` = 14680001;
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(14680001, 2803, 7000, 0, '', 0);
DELETE FROM `conversation_actors` WHERE `ConversationId` = 14680001;
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(14680001, 0, 0, 0, 96783, 66806, 1, 0, 0); -- Bastillax
DELETE FROM `conversation_line_template` WHERE `Id` = 2803;
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(2803, 0, 142, 0, 0, 0); -- Your persistence is admirable, but my demons have infiltrated every floor of the Vault. Only death awaits you.

DELETE FROM `creature_text` WHERE (`CreatureID` = 96665 AND `GroupID` IN (3, 4)) OR (`CreatureID` = 92985 AND `GroupID` = 4) OR `CreatureID` IN (97070, 97643, 97265, 97267);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(97265, 0, 0, 'You heard the Warden. We are the only ones who can defeat the Legion. We MUST survive this place.', 12, 0, 100, 0, 0, 57322, 0, 99530, 0, 'Kayn - running ahead to the Tomb'),
(97265, 1, 0, 'You trust no one and believe in nothing, Altruis. You are a leader with no followers.', 12, 0, 100, 0, 0, 57390, 0, 101999, 0, 'Kayn - running ahead to the Tomb'),
(97267, 0, 0, 'Fool! You would trust the Warden? She hates us almost as much as the Legion.', 12, 0, 100, 0, 0, 57389, 0, 101995, 0, 'Altruis - running ahead to the Tomb'),
(97070, 0, 0, 'Kayn and Altruis have already gone ahead. Take this felbat and join them.', 12, 0, 100, 0, 0, 57299, 0, 98783, 0, 'Jace Darkweaver - by the felbat'),
(97643, 0, 0, 'This is sheer madness! The Wardens have lost control of their prison.', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Kor''vas Bloodthorn - arriving in the Demon Ward');

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (97070, 97643);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (97070, 97643) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `target_type`, `comment`) VALUES
(97070, 0, 0, 0, 10, 0, 100, 0, 1, 25, 30000, 30000, 1, 0, 7, 'Jace Darkweaver - On player in sight (Grand Theft Felbat) - Talk 0'),
(97643, 0, 0, 0, 10, 0, 100, 0, 1, 25, 60000, 60000, 1, 0, 7, 'Kor''vas Bloodthorn - On player in sight (Grand Theft Felbat) - Talk 0');

DELETE FROM `creature` WHERE `guid` BETWEEN 14680220 AND 14680269;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680220, 97070, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4434.66, -292.31, -246.73, 1.4056, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Jace Darkweaver, by the elevator shaft
(14680221, 96659, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4436.02, -284.13, -246.72, 5.0936, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Vampiric Felbat, to ride up the shaft
(14680222, 97643, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4455.20, -390.80, 126.32, 1.6313, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kor'vas Bloodthorn, Demon Ward
(14680230, 100299, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4339.29, -267.37, -276.48, 1.8045, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Lyana Darksorrow, passage
(14680231, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4370.62, -231.00, -262.45, 1.4056, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Freed Illidari, passage
(14680232, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4337.93, -267.78, -276.95, 3.5004, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Freed Illidari, passage
(14680233, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4329.75, -284.13, -283.45, 0.9828, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Freed Illidari, passage
(14680234, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4425.12, -304.57, -247.29, 5.3004, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Freed Illidari, passage
(14680235, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4425.12, -314.79, -248.44, 3.1416, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Freed Illidari, passage
(14680236, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4371.99, -222.82, -262.08, 4.5472, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680237, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4359.73, -222.82, -263.85, 5.6397, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680238, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4348.82, -235.08, -266.11, 0.1853, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680239, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4343.38, -239.17, -267.48, 0.2915, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680240, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4396.51, -243.26, -260.17, 2.6992, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680241, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4340.65, -249.39, -270.05, 4.6368, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680242, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4340.65, -257.56, -273.41, 4.5744, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680243, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4337.93, -261.65, -274.45, 4.9461, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680244, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4332.48, -269.83, -279.16, 0.3588, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680245, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4333.84, -275.96, -280.79, 4.2487, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680246, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4331.11, -282.09, -283.06, 4.1244, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680247, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4430.57, -298.44, -246.78, 3.9857, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680248, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4359.73, -300.48, -260.07, 2.6422, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680249, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4426.49, -306.62, -247.32, 2.1588, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680250, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4421.04, -314.79, -248.85, 0.0000, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0), -- Fel Shocktrooper, passage
(14680251, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4371.99, -331.14, -259.05, 0.2985, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'TombPassageSquad', 0); -- Fel Shocktrooper, passage

-- Maiev leaves the cells for good once Breaking Out is taken; the Tomb's Kayn and Altruis are gone once Stop Gul'dan! is
-- taken (in its scene they run into the Vault of the Betrayer)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `ConditionTypeOrReference` = 47 AND `ConditionValue2` = 1
    AND ((`SourceEntry` = 92718 AND `ElseGroup` = 0 AND `ConditionValue1` = 38672) OR (`SourceEntry` = 96665 AND `ElseGroup` = 0 AND `ConditionValue1` IN (38723, 40253))
    OR (`SourceEntry` = 92986 AND `ElseGroup` = 3 AND `ConditionValue1` IN (38723, 40253)));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 92718, 0, 0, 47, 0, 38672, 1, 0, '', 0, 0, 0, '', 'Maiev: at the cells until Breaking Out is taken'),
(32, 3, 96665, 0, 0, 47, 0, 38723, 1, 0, '', 0, 0, 0, '', 'Kayn (Tomb of the Penitent): until Stop Gul''dan! (Havoc) is taken'),
(32, 3, 96665, 0, 0, 47, 0, 40253, 1, 0, '', 0, 0, 0, '', 'Kayn (Tomb of the Penitent): until Stop Gul''dan! (Vengeance) is taken'),
(32, 3, 92986, 0, 3, 47, 0, 38723, 1, 0, '', 0, 0, 0, '', 'Altruis (freed): in the Tomb of the Penitent until Stop Gul''dan! (Havoc) is taken'),
(32, 3, 92986, 0, 3, 47, 0, 40253, 1, 0, '', 0, 0, 0, '', 'Altruis (freed): in the Tomb of the Penitent until Stop Gul''dan! (Vengeance) is taken');
