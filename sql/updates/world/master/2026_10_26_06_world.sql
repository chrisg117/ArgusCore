-- A New Direction (40373), after All The Way Up, had no one to give it, no Pool of Judgment and no choice. As in the beta
-- video Wowhead links (youtu.be/CFgvWUwgm98) and the quest text:
-- - Kor'vas Bloodthorn gives it at the Pool of Judgment (Wowhead: auto accept), offered as soon as All The Way Up is
--   turned in. On accepting, Kayn and Altruis argue and Kor'vas cuts in (retail lines; quest_a_new_direction).
-- - Gazing into the Pool of Judgment (244455, its model is the pool; its own spell 204591 is in no data) plays the
--   "Harbingers" movie (192739, movie 478). Its end credits Pool of Judgment viewed and opens the choice
--   (196650 launches choice 234; go_a_new_direction_pool_of_judgment, player_a_new_direction). The quest text has the
--   player speak to Kor'vas to choose; no gossip line for that is in the client data, so the choice follows the movie.
-- - Choice 234 has the retail wording and art (read off the video; the response ids are ours). Its answers cast
--   A New Direction: Chose Kayn / Chose Altruis (196661 / 196662: the credit and the tracking quest 40374 / 40375;
--   playerchoice_a_new_direction). If Kayn is chosen, Altruis takes his leave.
-- - Kayn (96666) and Altruis (96669), who give the next quests, wait by the pool; the one not chosen is gone for the
--   player once A New Direction is turned in.
-- Kor'vas moves to the pool's rim (she stood where its model now is).

UPDATE `quest_template` SET `RewardNextQuest` = 40373 WHERE `ID` = 39686;

DELETE FROM `creature_queststarter` WHERE `quest` = 40373;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(97644, 40373, 0);
DELETE FROM `creature_questender` WHERE `quest` = 40373;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(97644, 40373, 0);

DELETE FROM `quest_template_addon` WHERE `ID` = 40373;
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`, `ScriptName`) VALUES
(40373, 39686, 'quest_a_new_direction');

DELETE FROM `playerchoice` WHERE `ChoiceId` = 234;
INSERT INTO `playerchoice` (`ChoiceId`, `UiTextureKitId`, `Question`, `HideWarboardHeader`, `MaxResponses`, `ScriptName`, `InfiniteRange`, `VerifiedBuild`) VALUES
(234, 0, 'Which follower will you choose?', 0, NULL, 'playerchoice_a_new_direction', 0, 0);
DELETE FROM `playerchoice_response` WHERE `ChoiceId` = 234;
INSERT INTO `playerchoice_response` (`ChoiceId`, `ResponseId`, `Index`, `ChoiceArtFileId`, `Header`, `Answer`, `Description`, `Confirmation`, `VerifiedBuild`) VALUES
(234, 2341, 0, 1274666, '', 'Kayn Sunfury', 'Kayn Sunfury is a loyal soldier and one of Lord Illidan Stormrage''s most trusted lieutenants.\n\nHe believes that cooperation and personal sacrifice are necessary to defeat the Legion.\n\nHonorable and demanding, he follows Illidan''s ideals to get the job done no matter the cost.\n\n|cFFFF0000This story choice will not affect your character''s power.|r', '', 0),
(234, 2342, 1, 1274661, '', 'Altruis the Sufferer', 'Altruis the Sufferer is a renegade who takes matters into his own hands.\n\nAlthough he and Illidan share the same goals, he often disagrees with Illidan''s methods.\n\nBelieving Illidan no longer controls his demonic powers, Altruis wants new leadership for the Illidari.\n\n|cFFFF0000This story choice will not affect your character''s power.|r', '', 0);

-- The argument by the pool, and Altruis's leave (retail lines, vo_701 Kayn / Altruis / Kor'vas)
DELETE FROM `creature_text` WHERE `CreatureID` IN (96666, 96669) OR (`CreatureID` = 97644 AND `GroupID` = 0);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(96666, 0, 0, 'Lord Illidan taught us to fight and to persevere. Together, we destroyed countless demons. YOU chose the coward''s path.', 12, 0, 100, 0, 0, 57296, 0, 99550, 0, 'Kayn Sunfury - A New Direction accepted'),
(96669, 0, 0, 'I respect our past, but I cannot forgive Illidan''s methods. The Illidari deserve a better leader.', 12, 0, 100, 0, 0, 57300, 0, 99646, 0, 'Altruis the Sufferer - A New Direction accepted'),
(97644, 0, 0, 'Stop this foolish bickering. We do not have time for this!', 12, 0, 100, 0, 0, 57295, 0, 99549, 0, 'Kor''vas Bloodthorn - A New Direction accepted'),
(96666, 1, 0, 'Then we stand at an impasse. $n, you must resolve this conflict.', 12, 0, 100, 0, 0, 57301, 0, 99647, 0, 'Kayn Sunfury - A New Direction accepted'),
(96669, 1, 0, 'There is no convincing you, I see. Very well, I will take my leave. $n, beware that you do not share Illidan''s fate.', 12, 0, 100, 0, 0, 57297, 0, 101640, 0, 'Altruis the Sufferer - Kayn chosen');

UPDATE `gameobject_template` SET `ScriptName` = 'go_a_new_direction_pool_of_judgment' WHERE `entry` = 244455;

-- The follower not chosen is gone once A New Direction is turned in (the tracking quests are rewarded on the choice)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (96666, 96669);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 96666, 0, 0, 47, 0, 40375, 66, 0, '', 1, 0, 0, '', 'Kayn (Pool of Judgment): Altruis not chosen'),
(32, 3, 96666, 0, 1, 47, 0, 40373, 64, 0, '', 1, 0, 0, '', 'Kayn (Pool of Judgment): or A New Direction not turned in yet'),
(32, 3, 96669, 0, 0, 47, 0, 40374, 66, 0, '', 1, 0, 0, '', 'Altruis (Pool of Judgment): Kayn not chosen'),
(32, 3, 96669, 0, 1, 47, 0, 40373, 64, 0, '', 1, 0, 0, '', 'Altruis (Pool of Judgment): or A New Direction not turned in yet');

-- Kor'vas on the pool's rim, Kayn and Altruis across it, facing the water
DELETE FROM `creature` WHERE `guid` IN (14680279, 14680281, 14680282);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680279, 97644, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4295.00, -452.00, 259.72, 0.0000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kor'vas Bloodthorn, Pool of Judgment
(14680281, 96666, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4275.00, -462.00, 259.69, 0.8961, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kayn Sunfury, Pool of Judgment
(14680282, 96669, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4275.00, -442.00, 259.69, 5.3871, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Altruis the Sufferer, Pool of Judgment

-- The Pool of Judgment
DELETE FROM `gameobject` WHERE `guid` = 14680214;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680214, 244455, 1468, 7814, 0, '0', 0, 0, 0, -1, 4283.00, -452.00, 259.69, 0, 0, 0, 0, 1, 120, 255, 1, '', NULL, 0);
