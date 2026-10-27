-- All The Way Up (39686), after the Demon Ward's three quests, had no one to give or end it and no elevator.
-- - Allari the Souleater gives it in the Demon Ward once Frozen in Time, Beam Me Up and Forged in Fire (either path) are
--   done ("Kor'vas went ahead ... Step onto the elevator when it arrives").
-- - The elevator up from the Demon Ward (246047) runs where the Vault of the Wardens dungeon has it: the client moves it
--   by its TransportAnimation rows, between the Demon Ward (z 118) and the Hall of Judgment (z 253); a script keeps it
--   cycling between its two stops.
-- - Arriving at the top credits Ascend to the Hall of Judgment (a server-side area trigger at the top landing,
--   at_all_the_way_up_hall_of_judgment).
-- - Kor'vas Bloodthorn (97644) ends it at the Pool of Judgment, where its turn-in point is. Its reward spell (195568,
--   missing from the client and the server) binds the hearthstone there; the area is our guess (The Warden's Court).

DELETE FROM `creature_queststarter` WHERE `quest` = 39686;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(96675, 39686, 0);
DELETE FROM `creature_questender` WHERE `quest` = 39686;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(97644, 39686, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` = 39686;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 39686, 0, 0, 8, 0, 39685, 0, 0, '', 0, 0, 0, '', 'All The Way Up: Frozen in Time done'),
(19, 0, 39686, 0, 0, 8, 0, 39684, 0, 0, '', 0, 0, 0, '', 'All The Way Up: Beam Me Up done'),
(19, 0, 39686, 0, 0, 8, 0, 39683, 0, 0, '', 0, 0, 0, '', 'All The Way Up: Forged in Fire (Havoc) done'),
(19, 0, 39686, 0, 1, 8, 0, 39685, 0, 0, '', 0, 0, 0, '', 'All The Way Up: Frozen in Time done'),
(19, 0, 39686, 0, 1, 8, 0, 39684, 0, 0, '', 0, 0, 0, '', 'All The Way Up: Beam Me Up done'),
(19, 0, 39686, 0, 1, 8, 0, 40254, 0, 0, '', 0, 0, 0, '', 'All The Way Up: Forged in Fire (Vengeance) done');

-- The reward: bind to the Hall of Judgment
DELETE FROM `serverside_spell` WHERE `Id` = 195568;
INSERT INTO `serverside_spell` (`Id`, `DifficultyID`, `CategoryId`, `Dispel`, `Mechanic`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `AttributesEx7`, `AttributesEx8`, `AttributesEx9`, `AttributesEx10`, `AttributesEx11`, `AttributesEx12`, `AttributesEx13`, `AttributesEx14`, `AttributesEx15`, `Stances`, `StancesNot`, `Targets`, `TargetCreatureType`, `RequiresSpellFocus`, `FacingCasterFlags`, `CasterAuraState`, `TargetAuraState`, `ExcludeCasterAuraState`, `ExcludeTargetAuraState`, `CasterAuraSpell`, `TargetAuraSpell`, `ExcludeCasterAuraSpell`, `ExcludeTargetAuraSpell`, `CasterAuraType`, `TargetAuraType`, `ExcludeCasterAuraType`, `ExcludeTargetAuraType`, `CastingTimeIndex`, `RecoveryTime`, `CategoryRecoveryTime`, `StartRecoveryCategory`, `StartRecoveryTime`, `InterruptFlags`, `AuraInterruptFlags1`, `AuraInterruptFlags2`, `ChannelInterruptFlags1`, `ChannelInterruptFlags2`, `ProcFlags`, `ProcFlags2`, `ProcChance`, `ProcCharges`, `ProcCooldown`, `ProcBasePPM`, `MaxLevel`, `BaseLevel`, `SpellLevel`, `DurationIndex`, `RangeIndex`, `Speed`, `LaunchDelay`, `StackAmount`, `EquippedItemClass`, `EquippedItemSubClassMask`, `EquippedItemInventoryTypeMask`, `ContentTuningId`, `SpellName`, `ConeAngle`, `ConeWidth`, `MaxTargetLevel`, `MaxAffectedTargets`, `SpellFamilyName`, `SpellFamilyFlags1`, `SpellFamilyFlags2`, `SpellFamilyFlags3`, `SpellFamilyFlags4`, `DmgClass`, `PreventionType`, `AreaGroupId`, `SchoolMask`, `ChargeCategoryId`) VALUES
(195568, 0, 0, 0, 0, 384, 268435456, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0, 'All The Way Up: Bind to the Hall of Judgment', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0);
DELETE FROM `serverside_spell_effect` WHERE `SpellID` = 195568;
INSERT INTO `serverside_spell_effect` (`SpellID`, `EffectIndex`, `DifficultyID`, `Effect`, `EffectAura`, `EffectAmplitude`, `EffectAttributes`, `EffectAuraPeriod`, `EffectBonusCoefficient`, `EffectChainAmplitude`, `EffectChainTargets`, `EffectItemType`, `EffectMechanic`, `EffectPointsPerResource`, `EffectPosFacing`, `EffectRealPointsPerLevel`, `EffectTriggerSpell`, `BonusCoefficientFromAP`, `PvpMultiplier`, `Coefficient`, `Variance`, `ResourceCoefficient`, `GroupSizeBasePointsCoefficient`, `EffectBasePoints`, `EffectMiscValue1`, `EffectMiscValue2`, `EffectRadiusIndex1`, `EffectRadiusIndex2`, `EffectSpellClassMask1`, `EffectSpellClassMask2`, `EffectSpellClassMask3`, `EffectSpellClassMask4`, `ImplicitTarget1`, `ImplicitTarget2`) VALUES
(195568, 0, 0, 11, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7865, 0, 0, 0, 0, 0, 0, 0, 1, 0); -- bind, The Warden's Court

-- The top landing of the elevator credits the ride (server side, our ids)
DELETE FROM `areatrigger_template` WHERE `Id` = 1468102 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1468102, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` = 1468102 AND `IsCustom` = 1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(1468102, 1, 1468102, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 8, 8, 0, 0, 0, 0, 'at_all_the_way_up_hall_of_judgment', 0);
DELETE FROM `areatrigger` WHERE `SpawnId` = 1468002;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1468002, 1468102, 1, 1468, '0', 4446.00, -452.00, 255.00, 0, 0, 0, 0, '', 'Vault of the Wardens - Hall of Judgment, top of the elevator (Quest: "All The Way Up" - Ascend to the Hall of Judgment)', 0);

-- Kor'vas at the Pool of Judgment, facing the way from the elevator
DELETE FROM `creature` WHERE `guid` = 14680279;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680279, 97644, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4290.00, -452.00, 259.67, 0.0000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Kor'vas Bloodthorn, Pool of Judgment

-- The elevator from the Demon Ward up to the Hall of Judgment, as the dungeon has it; it runs by itself
-- (go_all_the_way_up_elevator, on this map only)
DELETE FROM `gameobject` WHERE `guid` = 14680213;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680213, 246047, 1468, 7814, 0, '0', 0, 0, 0, -1, 4450.764, -451.194, 253.485, 0, 0, 0, 0, 1, 120, 255, 25, '', NULL, 0);
UPDATE `gameobject_template` SET `ScriptName` = 'go_all_the_way_up_elevator' WHERE `entry` = 246047;
