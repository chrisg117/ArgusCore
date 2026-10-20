-- Elerethe Renferal (creature 106087), the third boss of the Emerald Nightmare, and her adds.
-- Positions are the user's .gps readings of 2026-10-19 (GM.log, map 1520); see the design spec.

-- The templates were unsniffed: faction 35 (friendly), no script. Hostile (14), 2 s swings,
-- energy class on the boss, and the script names.
UPDATE `creature_template` SET `faction` = 14, `BaseAttackTime` = 2000, `unit_class` = 4, `ScriptName` = 'boss_elerethe_renferal' WHERE `entry` = 106087;
UPDATE `creature_template` SET `faction` = 14, `BaseAttackTime` = 2000, `ScriptName` = 'npc_en_venomous_spiderling' WHERE `entry` = 106311;
UPDATE `creature_template` SET `faction` = 14, `BaseAttackTime` = 2000, `ScriptName` = 'npc_en_skittering_spiderling' WHERE `entry` = 108542;

-- Twisting Shadows tornado: not selectable (UNIT_FLAG_UNINTERACTIBLE). The fork has no
-- no-threat bit in flags_extra; the script moves it and it never takes part in the fight.
UPDATE `creature_template` SET `faction` = 14, `BaseAttackTime` = 2000, `unit_flags` = 0x02000000, `ScriptName` = 'npc_en_twisting_shadows' WHERE `entry` = 106350;

-- Egg sac: not selectable, and never enters combat (CREATURE_FLAG_EXTRA_CANNOT_ENTER_COMBAT = 0x2000)
-- in place of a no-threat bit. UNIT_FLAG_NON_ATTACKABLE is rejected in creature_template.
UPDATE `creature_template` SET `faction` = 14, `BaseAttackTime` = 2000, `unit_flags` = 0x02000000, `flags_extra` = `flags_extra` | 0x2000, `ScriptName` = 'npc_en_elerethe_egg_sac' WHERE `entry` = 108540;

-- The templates came in at level 1 (the health is the level 1 base) until the difficulty row says 110
UPDATE `creature_template_difficulty` SET `MinLevel` = 110, `MaxLevel` = 110 WHERE `Entry` IN (106087, 106311, 108542, 106350, 108540) AND `DifficultyID` = 0;

-- Her lines: all 17 are in hotfixes.broadcast_text (Text1, the female text; the voice is
-- SoundKitID[1]). 14 = yell, 41 = raid boss emote. Groups: 0 Web of Pain, 1 Feeding Time,
-- 2 Gathering Clouds, 3 aggro, 4 Razor Wing, 5 Spider form, 6 kill, 7 wipe, 8 death,
-- 9-11 intro, 12 Feeding Time raid emote, 13 Gathering Clouds raid emote.
-- 108982 is the voiced "Scream for me!" (90971 has the same text and no sound).
DELETE FROM `creature_text` WHERE `CreatureID` = 106087;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(106087, 0, 0, 'Scream for me!', 14, 0, 100, 0, 0, 0, 0, 108982, 0, 'Elerethe Renferal - Web of Pain'),
(106087, 0, 1, 'I''ll adorn my web with your bones!', 14, 0, 100, 0, 0, 0, 0, 108983, 0, 'Elerethe Renferal - Web of Pain'),
(106087, 1, 0, 'Such juicy morsels!', 14, 0, 100, 0, 0, 0, 0, 108981, 0, 'Elerethe Renferal - Feeding Time'),
(106087, 2, 0, 'Now you have become prey!', 14, 0, 100, 0, 0, 0, 0, 108978, 0, 'Elerethe Renferal - Gathering Clouds'),
(106087, 2, 1, 'There is nowhere left to hide!', 14, 0, 100, 0, 0, 0, 0, 108980, 0, 'Elerethe Renferal - Gathering Clouds'),
(106087, 3, 0, 'This time I''m ready for your treachery!', 14, 0, 100, 0, 0, 0, 0, 108988, 0, 'Elerethe Renferal - Aggro'),
(106087, 3, 1, 'Mercy? None for you! Only gushing blood and broken bodies!', 14, 0, 100, 0, 0, 0, 0, 108989, 0, 'Elerethe Renferal - Aggro'),
(106087, 4, 0, 'Scurry, scurry, little rats!', 14, 0, 100, 0, 0, 0, 0, 108979, 0, 'Elerethe Renferal - Razor Wing'),
(106087, 5, 0, 'Not afraid of spiders, are you?', 14, 0, 100, 0, 0, 0, 0, 108986, 0, 'Elerethe Renferal - Spider form'),
(106087, 6, 0, 'Feel the crush of my talons!', 14, 0, 100, 0, 0, 0, 0, 108984, 0, 'Elerethe Renferal - Kill'),
(106087, 6, 1, 'My beak will tear your flesh, vermin!', 14, 0, 100, 0, 0, 0, 0, 108985, 0, 'Elerethe Renferal - Kill'),
(106087, 7, 0, 'Revenge is said to be sweet, but I taste only bitterness.', 14, 0, 100, 0, 0, 0, 0, 108976, 0, 'Elerethe Renferal - Wipe'),
(106087, 7, 1, 'Your faces no longer appear familiar. Did you truly deserve my wrath?', 14, 0, 100, 0, 0, 0, 0, 108977, 0, 'Elerethe Renferal - Wipe'),
(106087, 8, 0, 'Betrayers, will you remember killing me now?', 14, 0, 100, 0, 0, 0, 0, 108991, 0, 'Elerethe Renferal - Death'),
(106087, 9, 0, 'Betrayers! Liars! Murderers! Has Hamuul sent his lapdogs to fetch the knife from my back?', 14, 0, 100, 0, 0, 0, 0, 108972, 0, 'Elerethe Renferal - Intro 1'),
(106087, 10, 0, 'Do you remember murdering me? Or have you grown so used to slaughter that my cries of agony are long forgotten?', 14, 0, 100, 0, 0, 0, 0, 108974, 0, 'Elerethe Renferal - Intro 2'),
(106087, 11, 0, 'I recall my final moments surrounded by those I had trusted. Your satisfied sneers as I drew my last breath.', 14, 0, 100, 0, 0, 0, 0, 108975, 0, 'Elerethe Renferal - Intro 3'),
(106087, 12, 0, '|TInterface\Icons\ability_hunter_pet_spider.blp:20|t %s calls for |cFFF00000|Hspell:212364|h[Feeding Time]|h|r.', 41, 0, 100, 0, 0, 0, 0, 111635, 0, 'Elerethe Renferal - Feeding Time emote'),
(106087, 13, 0, '|TInterface\Icons\ability_hunter_pet_dragonhawk.blp:20|t Elerethe begins |cFFFF0000|Hspell:212707|h[Gathering Clouds]|h|r and prepares to fly to another platform!', 41, 0, 100, 0, 0, 0, 0, 111411, 0, 'Elerethe Renferal - Gathering Clouds emote');

-- Spawns: Normal only (14). Elerethe stands on the first platform's landing spot, facing the second.
-- The egg sacs sit on the two bridge webs, 3 per bridge at 25, 50 and 75 percent of the way
-- along the two straight segments (z interpolated between the readings, calibrate).
DELETE FROM `creature` WHERE `guid` BETWEEN 1520104 AND 1520110;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(1520104, 106087, 1520, 0, 0, '14', 0, 0, 0, -1, 0, 0, 11401.604, 11404.183, -85.51545, 4.555, 604800, 0, 0, 100, 0, 0, 0, 0, 0, '', NULL, 0),
(1520105, 108540, 1520, 0, 0, '14', 0, 0, 0, -1, 0, 0, 11385.564, 11360.438, -92.047, 4.3609, 604800, 0, 0, 100, 0, 0, 0, 0, 0, '', NULL, 0),
(1520106, 108540, 1520, 0, 0, '14', 0, 0, 0, -1, 0, 0, 11370.146, 11316.591, -98.409, 4.7423, 604800, 0, 0, 100, 0, 0, 0, 0, 0, '', NULL, 0),
(1520107, 108540, 1520, 0, 0, '14', 0, 0, 0, -1, 0, 0, 11371.537, 11270.019, -100.195, 4.7423, 604800, 0, 0, 100, 0, 0, 0, 0, 0, '', NULL, 0),
(1520108, 108540, 1520, 0, 0, '14', 0, 0, 0, -1, 0, 0, 11420.631, 11229.017, -97.448, 0.1163, 604800, 0, 0, 100, 0, 0, 0, 0, 0, '', NULL, 0),
(1520109, 108540, 1520, 0, 0, '14', 0, 0, 0, -1, 0, 0, 11465.865, 11240.393, -93.262, 0.6877, 604800, 0, 0, 100, 0, 0, 0, 0, 0, '', NULL, 0),
(1520110, 108540, 1520, 0, 0, '14', 0, 0, 0, -1, 0, 0, 11502.974, 11270.878, -90.215, 0.6877, 604800, 0, 0, 100, 0, 0, 0, 0, 0, '', NULL, 0);
