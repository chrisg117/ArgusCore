-- Forged in Fire (39683 Havoc, 40254 Vengeance): Immolanth fights with his abilities, and Cyana Nightglaive fights beside
-- the player.
-- - Immolanth (npc_forged_in_fire_immolanth) casts Chaos Nova (199828: burst and stun within 25 yd) and Burning Fel
--   (199758), whose fonts of fel (199761 makes areatrigger 5572) rise around the players and burn them with Burning Fel
--   (199773; at_forged_in_fire_burning_fel). Retail raises the fonts from creatures in the room that Burning Fel's
--   aura picks; which ones is not in the data, so he raises them himself. The fonts' size is ours.
-- - Cyana Nightglaive (96672) waits by him and joins the fight when he is pulled (damage from creatures stops at 30%
--   for both); when he dies she tells his killers "the demon's power is yours" (retail line, vo_701_cyana_nightglaive).
--   She is gone for the player once the quest is turned in, as he is.

DELETE FROM `areatrigger_template` WHERE `Id` = 1468103 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1468103, 1, 0, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` = 5572 AND `IsCustom` = 0;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(5572, 0, 1468103, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 4, 4, 0, 0, 0, 0, 'at_forged_in_fire_burning_fel', 0);

DELETE FROM `creature_text` WHERE `CreatureID` = 96672;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(96672, 0, 0, 'That was too close for my liking. $n, the demon''s power is yours.', 12, 0, 100, 0, 0, 57357, 0, 101409, 0, 'Cyana Nightglaive - Immolanth slain');

-- Cyana takes the Illidari's side and warglaives, and fights as a squad fighter (the floor on creature damage)
UPDATE `creature_template` SET `faction` = 2804 WHERE `entry` = 96672;
DELETE FROM `creature_equip_template` WHERE `CreatureID` = 96672;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(96672, 1, 128359, 0, 0, 128371, 0, 0, 0, 0, 0, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 96672;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 96672, 0, 0, 47, 0, 39683, 64, 0, '', 1, 0, 0, '', 'Cyana (Vault of Law): until Forged in Fire (Havoc) is turned in'),
(32, 3, 96672, 0, 0, 47, 0, 40254, 64, 0, '', 1, 0, 0, '', 'Cyana (Vault of Law): until Forged in Fire (Vengeance) is turned in');

-- Cyana, in front of Immolanth, facing him
DELETE FROM `creature` WHERE `guid` = 14680280;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680280, 96672, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4652.00, -454.00, 122.09, 0.0000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', NULL, 0); -- Cyana Nightglaive, Vault of Law
