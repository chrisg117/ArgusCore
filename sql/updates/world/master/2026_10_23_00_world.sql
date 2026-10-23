-- Fel Secrets (40051) could not be started: Kor'vas Bloodthorn (99045) did not give it, the Tome of Fel Secrets
-- (245112) was never spawned and its choice (231, launched by the tome's spell 194938) had no data. The tome offers
-- Havoc or Vengeance; playerchoice_fel_secrets casts the picked spec's spell (194939 / 194940), which gives the credit and
-- completes a tracking quest (39517 / 39518) that decides whether Kayn Sunfury then offers Cry Havoc and Let Slip the
-- Illidari! (39516) or Vengeance Will Be Mine! (39515).

-- Kor'vas Bloodthorn (99045) stands at the Illidari Foothold on retail (Wowhead pins; the retail video of The Imp
-- Mother's Tome heads back there to turn in), not at the Foul Creche where 2026_10_22_04 put him: on one of his pins,
-- in front of Kayn
UPDATE `creature` SET `position_x` = 1456.1, `position_y` = 1759.9, `position_z` = 54.56, `orientation` = 1.597 WHERE `guid` = 14810005;

DELETE FROM `creature_queststarter` WHERE `id` = 99045 AND `quest` = 40051;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(99045, 40051, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (40051, 39515, 39516);
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(40051, 40222),
(39515, 40051),
(39516, 40051);

-- Kayn offers the quest of the spec that was picked. He only ended both quests; nothing started them (Wowhead: Kayn
-- Sunfury starts and ends both)
DELETE FROM `creature_queststarter` WHERE `id` = 93127 AND `quest` IN (39515, 39516);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(93127, 39515, 0),
(93127, 39516, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` IN (39515, 39516);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 39516, 0, 0, 8, 0, 39517, 0, 0, '', 0, 0, 0, '', 'Cry Havoc and Let Slip the Illidari! needs Havoc picked in Fel Secrets'),
(19, 0, 39515, 0, 0, 8, 0, 39518, 0, 0, '', 0, 0, 0, '', 'Vengeance Will Be Mine! needs Vengeance picked in Fel Secrets');

-- The tome, from its Wowhead pins (beside Kayn), there while Fel Secrets is in the log and not yet done ("the book will
-- be consumed")
DELETE FROM `gameobject` WHERE `guid` = 14810005;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14810005, 245112, 1481, 7705, 0, '0', 0, 0, 0, -1, 1457.4, 1766.0, 54.73, 4.90137, 0, 0, -0.637594, 0.770373, 120, 255, 1, '', NULL, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 5 AND `SourceEntry` = 245112;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 5, 245112, 0, 0, 47, 0, 40051, 8, 0, '', 0, 0, 0, '', 'Tome of Fel Secrets is visible while Fel Secrets is incomplete');

-- The choice. Its text is not in the client: the question follows the quest text, each spec's description is its
-- ChrSpecialization text (577 Havoc, 581 Vengeance), the art is answer-dhhavoc / answer-dhvengeance. The response ids
-- are ours (choice id x 10 + n).
DELETE FROM `playerchoice` WHERE `ChoiceId` = 231;
INSERT INTO `playerchoice` (`ChoiceId`, `UiTextureKitId`, `Question`, `HideWarboardHeader`, `MaxResponses`, `ScriptName`, `InfiniteRange`, `VerifiedBuild`) VALUES
(231, 0, 'Which secrets will you use against the Burning Legion?', 0, NULL, 'playerchoice_fel_secrets', 0, 0);

DELETE FROM `playerchoice_response` WHERE `ChoiceId` = 231;
INSERT INTO `playerchoice_response` (`ChoiceId`, `ResponseId`, `Index`, `ChoiceArtFileId`, `Header`, `Answer`, `Description`, `Confirmation`, `VerifiedBuild`) VALUES
(231, 2311, 0, 1274664, 'Havoc', 'Select', 'A brooding master of warglaives and the destructive power of Fel magic.', '', 0),
(231, 2312, 1, 1274665, 'Vengeance', 'Select', 'Embraces the demon within to incinerate enemies and protect their allies.', '', 0);
