-- Warlock S4: An Eye for a Scepter (43153), scenario 1155 "An Eye for a Scepter" on map 1630 (the Tol Barad copy).
--
-- The data gives 43153 no starter: 43100's RewardNextQuest is 43153 and its start POI is Caer Darrow, so Calydus 109698
-- offers it there (fork data). A portal of ours beside him casts 218595 Teleport: Tol Barad (credits "Take the Portal
-- to Tol Barad"; its TELEPORT_TO_LFG_DUNGEON effect is scripted to WorldSafeLocs 5562). The scenario's ten steps are
-- scripted (zone_orderhall_warlock.cpp, namespace TolBaradWarlockScenario); step 2715 is a PlayerTriggerGameEvent
-- criteria and needs piece/criteria-player-trigger-game-event. After Occul'tharon the warlock goes to Krasus' Landing
-- (WorldSafeLocs 5711), where Calydus 109838 ends the quest ("Return to Calydus in Dalaran": an areatrigger at him).
-- Places: Wowhead's pins for 109952/109955/109956/109964 converted with WorldMapArea 1090 (TolBaradWarlockScenario;
-- check: the entry safe loc's own pin, 40.9, 21.2, converts to -845.3, 1186.5 against 5562's -845.72, 1186.69). D-Block
-- is the underground layer (z 84-92). Occul'tharon's pins are on Baradin Hold's floor, which has no WorldMapArea row:
-- he stands on the hold's hall beyond its safe loc 5579. Lines: the client's broadcast texts.
-- The portal's look is a lineup (local-only preview file); until the pick, display 10105 (Portal to Tol Barad).
--
-- lint: texts pending 43153 offer/request texts from the user's texts file (advisor ruling 2026-11-08)

DELETE FROM `quest_template_addon` WHERE `ID`=43153;
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(43153, 256, 43100);

DELETE FROM `creature_queststarter` WHERE `id`=109698 AND `quest`=43153;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(109698, 43153, 0); -- Calydus (Caer Darrow): An Eye for a Scepter

-- The scenario
DELETE FROM `instance_template` WHERE `map`=1630;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1630, 0, 'instance_tol_barad_warlock_scenario');
DELETE FROM `scenarios` WHERE `map`=1630;
INSERT INTO `scenarios` (`map`, `difficulty`, `scenario_A`, `scenario_H`) VALUES
(1630, 12, 1155, 1155);

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_warlock_teleport_tol_barad';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(218595, 'spell_warlock_teleport_tol_barad');

UPDATE `creature_template` SET `ScriptName`='npc_allaris_tol_barad', `npcflag`=`npcflag`|1 WHERE `entry`=109956;
UPDATE `creature_template` SET `ScriptName`='npc_tyranis_malem', `npcflag`=`npcflag`|1 WHERE `entry`=109964;
UPDATE `creature_template` SET `ScriptName`='npc_occultharon' WHERE `entry`=109981;

-- Map 1630 (fork block 16300000-16309999)
DELETE FROM `creature` WHERE `guid` BETWEEN 16300000 AND 16300007;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `VerifiedBuild`) VALUES
(16300000, 109952, 1630, 0, 0, '12', 0, 0, 0, 0, -971.60, 1184.40, 119.74, 3.10, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 'npc_dark_sworn_survivor', 0), -- Dark-Sworn Warlock, the survivor (pin 41.0, 30.6)
(16300001, 109956, 1630, 0, 0, '12', 0, 0, 0, 0, -1030.70, 1132.10, 91.58, 0.40, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, '', 0), -- Allaris, the council's survivors (pin 43.6, 35.0)
(16300002, 109955, 1630, 0, 0, '12', 0, 0, 0, 0, -1027.00, 1135.00, 91.88, 3.80, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, '', 0), -- Nagaz, with Allaris (pin 43.6, 34.6)
(16300003, 109964, 1630, 0, 0, '12', 0, 0, 0, 0, -981.00, 1033.30, 84.84, 1.50, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, '', 0), -- Tyranis Malem, deep in D-Block (pin 48.5, 31.3)
(16300004, 109955, 1630, 0, 0, '12', 0, 0, 0, 0, -964.90, 1107.90, 84.97, 3.50, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 'npc_nagaz_trap', 0), -- Nagaz, his trap (pin 44.8, 30.1)
(16300005, 109956, 1630, 0, 0, '12', 0, 0, 0, 0, -1235.00, 1055.50, 107.20, 3.14, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, '', 0), -- Allaris, the final wing (pin 47.4, 50.2), on its 107.2 corridor, facing along it (west): east of her the corridor ends
(16300006, 109956, 1630, 0, 0, '12', 0, 0, 0, 0, -330.00, -4528.00, 170.60, 0.00, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, '', 0), -- Allaris, Baradin Hold
(16300007, 109981, 1630, 0, 0, '12', 0, 0, 0, 0, -300.00, -4528.00, 170.34, 3.14, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, '', 0); -- Occul'tharon, Baradin Hold

DELETE FROM `areatrigger_template` WHERE `Id` BETWEEN 16300000 AND 16300004 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(16300000, 1, 1, 0, 0, 0), (16300001, 1, 1, 0, 0, 0), (16300002, 1, 1, 0, 0, 0), (16300003, 1, 1, 0, 0, 0), (16300004, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id` BETWEEN 16300000 AND 16300004 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(16300000, 1, 16300000, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 12, 12, 0, 0, 0, 0, '', 0),
(16300001, 1, 16300001, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 10, 10, 0, 0, 0, 0, '', 0),
(16300002, 1, 16300002, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 10, 10, 0, 0, 0, 0, '', 0),
(16300003, 1, 16300003, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 12, 12, 0, 0, 0, 0, '', 0),
(16300004, 1, 16300004, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 8, 8, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId` BETWEEN 16300000 AND 16300004;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(16300000, 16300000, 1, 1630, '12', -1030.70, 1132.10, 91.58, 0, 0, 0, 0, 'at_tol_barad_where_are_they', 'Tol Barad (warlock scenario 1155) - the council survivors: Where Are They?', 0),
(16300001, 16300001, 1, 1630, '12', -981.00, 1033.30, 84.84, 0, 0, 0, 0, 'at_tol_barad_the_third_one', 'Tol Barad (warlock scenario 1155) - Tyranis: The Third One', 0),
(16300002, 16300002, 1, 1630, '12', -964.90, 1107.90, 84.97, 0, 0, 0, 0, 'at_tol_barad_back_to_searching', 'Tol Barad (warlock scenario 1155) - Nagaz''s corner: Back to Searching', 0),
(16300003, 16300003, 1, 1630, '12', -1235.00, 1055.50, 107.20, 0, 0, 0, 0, 'at_tol_barad_back_to_the_manifest', 'Tol Barad (warlock scenario 1155) - the final wing: Back to the Manifest', 0),
(16300004, 16300004, 1, 1630, '12', -354.91, -4528.85, 170.43, 0, 0, 0, 0, 'at_tol_barad_baradin_hold', 'Tol Barad (warlock scenario 1155) - inside Baradin Hold', 0);

-- Krasus' Landing: Calydus 109838 (the ender) and "Return to Calydus in Dalaran"
DELETE FROM `creature` WHERE `guid`=12205002;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12205002, 109838, 1220, 0, 0, '0', 0, 0, 0, 0, -816.00, 4260.00, 746.25, 2.30, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Calydus, Krasus' Landing (the turn-in POI)
DELETE FROM `areatrigger_template` WHERE `Id`=12205000 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(12205000, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=12205000 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(12205000, 1, 12205000, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 8, 8, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId`=12205000;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(12205000, 12205000, 1, 1220, '0', -816.00, 4260.00, 746.25, 0, 0, 0, 0, 'at_krasus_landing_calydus_post_scenario', 'Krasus'' Landing - Calydus after the warlock''s Tol Barad scenario', 0);

-- The portal at Caer Darrow (fork template, a spellcaster with 218595) and who sees what
DELETE FROM `gameobject_template` WHERE `entry`=1220012;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `ScriptName`, `VerifiedBuild`) VALUES
(1220012, 22, 10105, 'Portal to Tol Barad', '', '', '', 1, 218595, -1, '', 0);
DELETE FROM `gameobject` WHERE `guid`=10000017;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(10000017, 1220012, 0, 0, 0, '0', 0, 0, 0, -1, 1101.00, -2563.00, 60.17, 0, 0, 0, 0, 1, 60, 255, 1, '', NULL, 0); -- Portal to Tol Barad, beside Calydus
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND ((`SourceGroup`=3 AND `SourceEntry`=109838) OR (`SourceGroup`=5 AND `SourceEntry`=1220012));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 109838, 0, 0, 47, 0, 43153, 74, 0, 0, 'Calydus (Krasus'' Landing): An Eye for a Scepter incomplete, complete or rewarded'),
(32, 5, 1220012, 0, 0, 47, 0, 43153, 8, 0, 0, 'Portal to Tol Barad: An Eye for a Scepter incomplete');

-- Lines (broadcast texts)
DELETE FROM `creature_text` WHERE `CreatureID` IN (109952, 109956, 109955, 109964);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT c.`cid`, c.`grp`, 0, b.`Text`, 12, 0, 100, 0, 0, 0, 0, b.`ID`, 0, c.`cmt` FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 109952 AS cid, 0 AS grp, 116314 AS bid, 'Dark-Sworn Warlock - Tol Barad survivor: the others are in D-Block' AS cmt
  UNION ALL SELECT 109956, 0, 116383, 'Allaris Narassin - Gul''dan knows the Eye is here'
  UNION ALL SELECT 109956, 1, 116385, 'Allaris Narassin - Nagaz sent his disciple below'
  UNION ALL SELECT 109956, 2, 116456, 'Allaris Narassin - found Tyranis too late'
  UNION ALL SELECT 109956, 3, 116484, 'Allaris Narassin - came up empty, no Nagaz either'
  UNION ALL SELECT 109956, 4, 116489, 'Allaris Narassin - the manifest: a void terror'
  UNION ALL SELECT 109956, 5, 116967, 'Allaris Narassin - Baradin Hold: the most dangerous prisoners'
  UNION ALL SELECT 109956, 6, 116968, 'Allaris Narassin - Baradin Hold: the Horde and Alliance'
  UNION ALL SELECT 109956, 7, 116969, 'Allaris Narassin - Occul''tharon: never faced a void terror'
  UNION ALL SELECT 109956, 8, 116972, 'Allaris Narassin - the beast falls'
  UNION ALL SELECT 109956, 9, 116973, 'Allaris Narassin - I''ll retrieve the Eye'
  UNION ALL SELECT 109956, 10, 116974, 'Allaris Narassin - I have the Eye, you have the Book'
  UNION ALL SELECT 109955, 0, 116387, 'Nagaz - Tyranis went off to the right'
  UNION ALL SELECT 109964, 0, 116459, 'Tyranis Malem - at Gul''dan''s side'
) c ON c.`bid` = b.`ID`;
