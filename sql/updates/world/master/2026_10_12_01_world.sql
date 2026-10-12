-- The priest's way from the tomb of Tyr to Netherlight Temple: "Blade in Twilight" (40710) is handed in at the
-- Antonidas Memorial in Dalaran, and Velen there gives "The Light and the Void" (40938) and leads to the portal
-- in Aegwynn's Gallery. Dalaran had none of this. Who stands where and what is said are from a video of the
-- retail quests; the places are measured on the city's model.

-- Dalaran's teleport pad in the middle of the city and the one below it in Aegwynn's Gallery did nothing. The
-- client has no area triggers on them and the spells that take one down and up ("Teleport: Portrait Room"
-- 192293, "Teleport: Dalaran" 192295, both with a place in `spell_target_position`) had no caster, so each
-- pad is an area trigger of the server's own that casts its spell on a player who steps on it. A third is
-- around the three at the memorial, a fourth where one arrives in the gallery (see zone_orderhall_priest.cpp).
DELETE FROM `areatrigger_template` WHERE `Id` IN (102285, 102462, 192293, 192295) AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(102285, 1, 1, 0, 0, 0),
(102462, 1, 1, 0, 0, 0),
(192293, 1, 1, 0, 0, 0),
(192295, 1, 1, 0, 0, 0);

DELETE FROM `areatrigger_create_properties` WHERE `Id` IN (102285, 102462, 192293, 192295) AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(102285, 1, 102285, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 13, 13, 0, 0, 0, 0, '', 0),
(102462, 1, 102462, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 5, 5, 0, 0, 0, 0, '', 0),
(192293, 1, 192293, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 2.2, 2.2, 0, 0, 0, 0, '', 0),
(192295, 1, 192295, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 2.2, 2.2, 0, 0, 0, 0, '', 0);

DELETE FROM `areatrigger` WHERE `SpawnId` BETWEEN 1220001 AND 1220004;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1220001, 192293, 1, 1220, '0', -844.65, 4467.65, 735.88, 0, 0, 0, 0, 'at_dalaran_teleport_pad_to_aegwynns_gallery', 'Dalaran - the teleport pad in the middle of the city', 0),
(1220002, 192295, 1, 1220, '0', -780, 4415.2, 602.63, 0, 0, 0, 0, 'at_dalaran_teleport_pad_to_the_city', 'Dalaran - Aegwynn''s Gallery - the teleport pad', 0),
(1220003, 102285, 1, 1220, '0', -698.55, 4492.46, 728.64, 0, 0, 0, 0, 'at_dalaran_antonidas_memorial_priests', 'Dalaran - Antonidas Memorial - a priest returns to Alonsus and Moira (Quest: "Blade in Twilight")', 0),
(1220004, 102462, 1, 1220, '0', -784.91, 4419.19, 602.48, 0, 0, 0, 0, 'at_dalaran_aegwynns_gallery_velen', 'Dalaran - Aegwynn''s Gallery - Velen goes ahead to the portal (Quest: "The Light and the Void")', 0);

-- Moira Thaurissan, Alonsus Faol and Velen at the memorial, in a row before the statue and turned to the way
-- in. WorldSafeLocs 5223 is at this place and 5726 six yards before it, turned to it; Faol stands on the
-- first, the others three yards to his sides (Moira on the left of who comes, as in the video).
-- Priests see them from "Blade in Twilight" on until "The Light and the Void" is done, and the portal to the
-- temple while they are on that quest. The phases of the retail game are not known; these are the next two
-- unused ones after the phases of the priest's quests in Tirisfal (5737, 5738), cosmetic like them.
DELETE FROM `phase_area` WHERE `AreaId`=7502 AND `PhaseId` IN (5739, 5740);
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(7502, 5739, 'See Moira Thaurissan, Alonsus Faol and Velen at the Antonidas Memorial'),
(7502, 5740, 'See the portal to Netherlight Temple in Aegwynn''s Gallery');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup` IN (5739, 5740) AND `SourceEntry`=7502;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 5739, 7502, 0, 0, 47, 0, 40710, 74, 0, '', 0, 0, 0, '', 'Apply Phase 5739 if Quest 40710 is in progress | complete | rewarded'),
(26, 5739, 7502, 0, 0, 47, 0, 40938, 64, 0, '', 1, 0, 0, '', 'Apply Phase 5739 if Quest 40938 is not rewarded'),
(26, 5740, 7502, 0, 0, 47, 0, 40938, 10, 0, '', 0, 0, 0, '', 'Apply Phase 5740 if Quest 40938 is in progress | complete');

DELETE FROM `creature` WHERE `guid` BETWEEN 12200001 AND 12200003;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12200001, 101310, 1220, 0, 0, '0', 0, 5739, 0, -1, 0, 0, -701.04, 4494.11, 728.52, 4.1, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Moira Thaurissan
(12200002, 102363, 1220, 0, 0, '0', 0, 5739, 0, -1, 0, 0, -698.55, 4492.46, 728.64, 3.9, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Alonsus Faol
(12200003, 101313, 1220, 0, 0, '0', 0, 5739, 0, -1, 0, 0, -696.06, 4490.81, 728.76, 3.7, 300, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Prophet Velen

-- Velen leads at a brisk walk: in the video he is at the pad, some 180 yards on, 45 seconds after setting out.
-- In the gallery (102462) he walks as others do.
UPDATE `creature_template` SET `speed_walk`=1.6, `ScriptName`='npc_prophet_velen_antonidas_memorial' WHERE `entry`=101313;
UPDATE `creature_template` SET `ScriptName`='npc_prophet_velen_aegwynns_gallery' WHERE `entry`=102462;
UPDATE `creature_template` SET `ScriptName`='npc_moira_thaurissan_antonidas_memorial' WHERE `entry`=101310;

DELETE FROM `creature_text` WHERE `CreatureID` IN (101310, 101313, 102363, 102462);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(102363, 0, 0, 'Look! $n has returned!', 12, 0, 100, 0, 0, 63405, 0, 104645, 0, 'Alonsus Faol - a priest returns with the weapon'),
(102363, 1, 0, 'With that we are in agreement. Now, speak with Velen. He has an... interesting matter to discuss with you.', 12, 0, 100, 0, 0, 63400, 0, 104649, 0, 'Alonsus Faol - Blade in Twilight is rewarded'),
(101310, 0, 0, 'This champion shows talent!', 12, 0, 100, 0, 0, 63406, 0, 104646, 0, 'Moira Thaurissan - a priest returns with the weapon'),
(101313, 0, 0, 'Well done indeed!', 12, 0, 100, 0, 0, 63407, 0, 104647, 0, 'Prophet Velen - a priest returns with the weapon'),
(101313, 1, 0, 'The mages have established a portal for us to use. If you would follow me, I would speak of the naaru and the draenei.', 12, 0, 100, 0, 0, 63401, 0, 104673, 0, 'Prophet Velen - The Light and the Void is accepted'),
(101313, 2, 0, 'Over the long years of the dranei''s flight from the Burning Legion, the naaru helped us... sometimes at the cost of their very lives. We would have perished without their aid.', 12, 0, 100, 0, 0, 63402, 0, 104674, 0, 'Prophet Velen - on the way to the teleport pad'),
(102462, 0, 0, 'At last we found a way to repay their kindness. We came upon a terrible sight, a naaru that had turned to the void. There had to be a way to save it!', 12, 0, 100, 0, 0, 63403, 0, 104675, 0, 'Prophet Velen - in Aegwynn''s Gallery'),
(102462, 1, 0, 'We built a prison to contain and study the fallen creature. A place to commune with the naaru and ponder the nature of the Twisting Nether. It is here that I would take you.', 12, 0, 100, 0, 0, 63404, 0, 104676, 0, 'Prophet Velen - in the Portrait Room');

-- What Moira says to who hands in "Blade in Twilight" (from the video; the database had no text)
DELETE FROM `quest_offer_reward` WHERE `ID`=40710;
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(40710, 0, 0, 0, 0, 0, 0, 0, 0, 'You have demonstrated no small amount of skill in these sorts of tasks.$B$BThat is good, for we will require even more of you if we are to survive this conflict.', 0);

-- Velen gives "The Light and the Void", which had no giver. It follows "Blade in Twilight": the holy and the
-- discipline priest come to it from quests of their own, which are not built.
DELETE FROM `creature_queststarter` WHERE `id`=101313 AND `quest`=40938;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(101313, 40938, 0);

DELETE FROM `quest_template_addon` WHERE `ID`=40938;
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(40938, 16, 40710);

-- The portal to Netherlight Temple of this quest (246663, which casts "Teleport: Priest Order Hall") on the
-- low dais in the middle of the gallery's deepest room, turned to the stairs one comes down. WorldSafeLocs
-- 5653 is at this place and has this facing. Where Wowhead has the portal (36.4, 73.8) is at the foot
-- of the stairs, ten yards before the dais.
DELETE FROM `gameobject` WHERE `guid`=12200001;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12200001, 246663, 1220, 0, 0, '0', 0, 5740, 0, -1, -882.45, 4498.21, 580.31, 5.599, 0, 0, 0.335452, -0.942057, 300, 255, 1, '', NULL, 0); -- Portal to Netherlight Temple

DELETE FROM `spell_script_names` WHERE `spell_id`=202324;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(202324, 'spell_orderhall_priest_teleport_priest_order_hall');

-- The portal leads to the temple while it is still Saraka's prison (map 1629, "Priest Order Formation"), on
-- which nothing stands yet
DELETE FROM `instance_template` WHERE `map`=1629;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1629, 0, '');
