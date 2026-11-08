-- Warlock S2: The New Blood (40729), scenario 962 "Into the Mouth of the Nether" on map 1522, and The Tome of Blighted
-- Implements (40684), the weapon choice.
--
-- Staging from Wowhead's Destruction acquisition video (0:42-14:20); every spoken line is the client's broadcast text.
-- Map 1522 has no sniffed spawns: places come from its own doodads on the navigation mesh. The prison is the cages
-- around the Front Entrance (WorldSafeLocs 5332, 2964.78, 1128.12): the one ground-level cage (2971.8, 1143.7) holds
-- the warlock, the upper level (z about 221) the captive council; the way to the archive passes GY 5455 (3060, 1085)
-- and the fel walls; the escape leaves through the finale gateway (WMO 7Lg_Legion_Gatewaysmall01, 3090, 846).
--
-- lint: texts pending 40729 offer/request texts from the user's texts file (advisor ruling 2026-11-08)
-- lint: texts pending 40684 offer/request texts from the user's texts file (advisor ruling 2026-11-08)

-- Chain and class (no quest_template_addon rows in the data)
DELETE FROM `quest_template_addon` WHERE `ID` IN (40729, 40684);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(40729, 256, 40716),
(40684, 256, 40729);

-- Ritssyn offers The New Blood at The Sixth's turn-in (video 0:42; fork data, 40716's RewardNextQuest names it)
-- Calydus offers The Tome of Blighted Implements: on retail 40729's turn-in to him offers it (RewardNextQuest), but here
-- the scenario's last step rewards 40729 itself, so nothing would (fork data; PrevQuestID 40729 gates it)
DELETE FROM `creature_queststarter` WHERE (`id`=101456 AND `quest`=40729) OR (`id`=101097 AND `quest`=40684);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(101456, 40729, 0), -- Ritssyn Flamescowl: The New Blood
(101097, 40684, 0); -- Calydus: The Tome of Blighted Implements

-- Ritssyn (the ritual, and its restart by gossip) and Calydus in the Circle of Wills (the choice, and its reopening)
UPDATE `creature_template` SET `ScriptName`='npc_ritssyn_flamescowl_circle_of_wills', `npcflag`=`npcflag`|1 WHERE `entry`=101456;
UPDATE `creature_template` SET `ScriptName`='npc_calydus_circle_of_wills', `npcflag`=`npcflag`|1 WHERE `entry`=101097;
UPDATE `creature_template` SET `ScriptName`='npc_calydus_prison' WHERE `entry`=101050;

-- Calydus in the Circle once the warlock is back from the prison (visibility by entry: 101097 is also the hall's)
DELETE FROM `creature` WHERE `guid`=12205001;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12205001, 101097, 1220, 0, 0, '0', 0, 0, 0, 0, -858.00, 4456.00, 696.35, 5.72, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Calydus
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup` IN (3, 5) AND `SourceEntry`=101097;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 101097, 0, 0, 47, 0, 40729, 66, 0, 0, 'Calydus: seen once The New Blood is complete or rewarded');

-- The prison scenario
DELETE FROM `instance_template` WHERE `map`=1522;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1522, 0, 'instance_dreadscar_rift_scenario');
DELETE FROM `scenarios` WHERE `map`=1522;
INSERT INTO `scenarios` (`map`, `difficulty`, `scenario_A`, `scenario_H`) VALUES
(1522, 12, 962, 962);

-- 199359 Start Scenario: into the ground cage; 196340 Exit Teleport: back to the Circle (WorldSafeLocs 5380)
DELETE FROM `spell_target_position` WHERE `ID` IN (199359, 196340);
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(199359, 0, 0, 1522, 2965.33, 1139.20, 206.68, 4.76, 0),
(196340, 1, 0, 1220, -873.39, 4431.86, 697.39, 4.01, 0);

-- Spell scripts
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_warlock_force_specialization', 'spell_warlock_enslave_beshtal');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(211250, 'spell_warlock_force_specialization'),
(211251, 'spell_warlock_force_specialization'),
(211252, 'spell_warlock_force_specialization'),
(199336, 'spell_warlock_enslave_beshtal');

-- Enslave Beshtal: a click on the jailer; its nearby-entry target is Beshtal
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=98836;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(98836, 199336, 1, 0); -- Beshtal: Enslave Beshtal
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=199336;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(13, 1, 199336, 0, 0, 31, 0, 3, 98836, 0, 0, 'Enslave Beshtal: Beshtal');

-- The weapon choice: 199411 launches player choice 245 (Destruction only; Affliction and Demonology are not built).
-- Response art 0 until a preview (no art id goes in unpreviewed)
DELETE FROM `playerchoice` WHERE `ChoiceId`=245;
INSERT INTO `playerchoice` (`ChoiceId`, `UiTextureKitId`, `Question`, `HideWarboardHeader`, `MaxResponses`, `ScriptName`, `InfiniteRange`, `VerifiedBuild`) VALUES
(245, 0, 'Which weapon should we pursue first?', 0, NULL, 'playerchoice_the_tome_of_blighted_implements', 0, 0);
DELETE FROM `playerchoice_response` WHERE `ChoiceId`=245;
INSERT INTO `playerchoice_response` (`ChoiceId`, `ResponseId`, `Index`, `ChoiceArtFileId`, `Header`, `Answer`, `Description`, `Confirmation`, `VerifiedBuild`) VALUES
(245, 2451, 0, 0, 'Destruction', 'Select', 'The Scepter of Sargeras is in the hands of Gul''dan.\n\nTake it from him and turn its fire against Jagganoth.', 'CONFIRM_ARTIFACT_CHOICE', 0);

-- Lines (broadcast texts)
DELETE FROM `creature_text` WHERE `CreatureID` IN (101456, 98797, 98831, 98771, 101050);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT c.`cid`, c.`grp`, 0, b.`Text`, c.`type`, 0, 100, 0, 0, 0, 0, b.`ID`, 0, c.`cmt` FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 101456 AS cid, 0 AS grp, 103743 AS bid, 12 AS type, 'Ritssyn Flamescowl - The New Blood: the sixth is here' AS cmt
  UNION ALL SELECT 98797, 0, 103744, 12, 'Zelifrax Wobblepox - the ritual: studied the rites'
  UNION ALL SELECT 98797, 1, 103747, 12, 'Zelifrax Wobblepox - the ritual: channel on the portal'
  UNION ALL SELECT 98797, 2, 103750, 12, 'Zelifrax Wobblepox - the ritual begins'
  UNION ALL SELECT 98831, 0, 103762, 14, 'Jagganoth - the ritual: foolish mortals'
  UNION ALL SELECT 98831, 1, 101418, 14, 'Jagganoth - the escape: none can escape'
  UNION ALL SELECT 98831, 2, 101038, 14, 'Jagganoth - the ritual: hurry up with that portal'
  UNION ALL SELECT 98771, 0, 100641, 12, 'Ritssyn Flamescowl - prisoner: Jagganoth''s prisoners'
  UNION ALL SELECT 101050, 0, 100800, 12, 'Calydus - prison: don''t hurt Calydus'
  UNION ALL SELECT 101050, 1, 100801, 12, 'Calydus - prison: Beshdal is gone'
  UNION ALL SELECT 101050, 2, 100802, 12, 'Calydus - prison: servant to Jagganoth'
  UNION ALL SELECT 101050, 3, 100803, 12, 'Calydus - prison: will distract sentries'
  UNION ALL SELECT 101050, 4, 100909, 12, 'Calydus - jailbreak: Calydus is a friend'
  UNION ALL SELECT 101050, 5, 100910, 12, 'Calydus - jailbreak: powerful weapons'
  UNION ALL SELECT 101050, 6, 100913, 12, 'Calydus - jailbreak: come to the archives'
  UNION ALL SELECT 101050, 7, 101229, 12, 'Calydus - archive: Xel''toth waits there'
  UNION ALL SELECT 101050, 8, 103457, 12, 'Calydus - archive: here is the tome'
  UNION ALL SELECT 101050, 9, 103458, 12, 'Calydus - archive: take it'
  UNION ALL SELECT 101050, 10, 101259, 12, 'Calydus - escape: help friend find the weapon'
) c ON c.`bid` = b.`ID`;

-- The ritual gateway the script opens in the Circle (fork template). Its look is the user's pick from a lineup (a
-- local-only preview file); until then display 30007 (243679 Ritual Portal), unpreviewed
DELETE FROM `gameobject_template` WHERE `entry`=1220004;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `ScriptName`, `VerifiedBuild`) VALUES
(1220004, 5, 30007, 'Ritual Gateway', '', '', '', 1, '', 0);

-- Map 1522: the prison, the upper level, the archive, the way out (fork block 15220000-15229999)
DELETE FROM `creature` WHERE `guid` BETWEEN 15220000 AND 15220009;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `VerifiedBuild`) VALUES
(15220000, 98771, 1522, 0, 0, '12', 0, 0, 0, 0, 2958.00, 1132.00, 206.81, 0.78, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 'npc_ritssyn_flamescowl_prisoner', 0), -- Ritssyn, prisoner
(15220001, 98836, 1522, 0, 0, '12', 0, 0, 0, 0, 2964.78, 1128.12, 206.68, 1.52, 86400, 0, 0, 0, 16777216, NULL, NULL, NULL, 'npc_beshtal', 0), -- Beshtal, before the warlock's cage
(15220002, 99147, 1522, 0, 0, '12', 0, 0, 0, 0, 3004.80, 1168.50, 221.21, 4.70, 86400, 0, 0, 0, 1, NULL, NULL, NULL, 'npc_dreadscar_scenario_captive', 0), -- Jubeka, captive
(15220003, 99145, 1522, 0, 0, '12', 0, 0, 0, 0, 3020.80, 1152.00, 221.35, 3.14, 86400, 0, 0, 0, 1, NULL, NULL, NULL, 'npc_dreadscar_scenario_captive', 0), -- Zinnin, captive
(15220004, 99146, 1522, 0, 0, '12', 0, 0, 0, 0, 3036.67, 1109.33, 222.95, 2.40, 86400, 0, 0, 0, 1, NULL, NULL, NULL, 'npc_dreadscar_scenario_captive', 0), -- Shinfel, captive
(15220005, 101460, 1522, 0, 0, '12', 0, 0, 0, 0, 3013.87, 1122.27, 222.01, 2.30, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 'npc_dreadscar_scenario_hostile', 0), -- Dreadscar Jailer
(15220006, 98873, 1522, 0, 0, '12', 0, 0, 0, 0, 3023.33, 1126.00, 221.48, 3.80, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 'npc_dreadscar_scenario_hostile', 0), -- Dreadscar Raider
(15220007, 98834, 1522, 0, 0, '12', 0, 0, 0, 0, 3069.20, 922.00, 251.43, 3.30, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 'npc_xeltoth', 0), -- Xel'toth, the archive
(15220008, 98873, 1522, 0, 0, '12', 0, 0, 0, 0, 3048.93, 941.73, 254.09, 5.50, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 'npc_dreadscar_scenario_hostile', 0), -- Dreadscar Raider, the archive
(15220009, 98831, 1522, 0, 0, '12', 0, 0, 0, 0, 3100.00, 880.00, 248.94, 2.80, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, '', 0); -- Jagganoth, above the way out (staging)

-- Server-side areatriggers: past the fel barrier (Jailbreak) and the way out (To Fight Another Day)
DELETE FROM `areatrigger_template` WHERE `Id` IN (15220000, 15220001) AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(15220000, 1, 1, 0, 0, 0),
(15220001, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` IN (15220000, 15220001) AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(15220000, 1, 15220000, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 6, 6, 0, 0, 0, 0, '', 0),
(15220001, 1, 15220001, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 5, 5, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId` IN (15220000, 15220001);
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(15220000, 15220000, 1, 1522, '12', 3060.15, 1085.20, 232.54, 0, 0, 0, 0, 'at_dreadscar_scenario_jailbreak', 'Dreadscar Rift (warlock scenario 962) - past the fel barrier: Jailbreak', 0),
(15220001, 15220001, 1, 1522, '12', 3085.60, 833.33, 245.29, 0, 0, 0, 0, 'at_dreadscar_scenario_way_out', 'Dreadscar Rift (warlock scenario 962) - the gateway: To Fight Another Day', 0);
