-- Beam Me Up (39684), the second of the Demon Ward's three quests after Grand Theft Felbat, had no one to give or end it
-- and nothing in the Vault of Mirrors. As in the retail video (youtu.be/u0lS6nhLpBs):
-- - Kor'vas Bloodthorn gives it in the Demon Ward; Allari the Souleater, beside her, takes it in.
-- - In the Vault of Mirrors, Glazer floats shielded (Focusing) and burns Altruis with his Eye Beam while Altruis and Freed
--   Illidari fight him (squad fighters; npc_beam_me_up_glazer). Altruis asks the player to turn the final mirror.
-- - Turning the Reflective Mirror (its "Rotating" lock) sends the beam back at Glazer for that player
--   (go_beam_me_up_reflective_mirror): his shield breaks, which is the quest's credit, and Altruis calls everyone out.
--   The player then sees Glazer stunned without his shield, and Altruis and the Illidari gone.
-- Retail bounces the beam between more mirrors; here two Lens statues bounce it on its way to Altruis.

DELETE FROM `creature_queststarter` WHERE `quest` = 39684;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(97643, 39684, 0);
DELETE FROM `creature_questender` WHERE `quest` = 39684;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(96675, 39684, 0);

DELETE FROM `quest_template_addon` WHERE `ID` = 39684;
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(39684, 39682);

-- Altruis's lines in the Vault of Mirrors (retail lines)
DELETE FROM `creature_text` WHERE `CreatureID` = 99229;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(99229, 0, 0, '$n, the creature''s destructive beam is focused upon me. Redirect the final mirror to shatter its shield!', 12, 0, 100, 0, 0, 57307, 0, 98935, 0, 'Altruis the Sufferer - player on Beam Me Up arrives'),
(99229, 1, 0, 'The shield is broken! Quickly, everyone out while the creature is stunned!', 12, 0, 100, 0, 0, 57306, 0, 98934, 0, 'Altruis the Sufferer - Glazer''s shield broken');

-- Glazer: hostile, shielded; stunned once his shield is broken. Altruis fights him with the Freed Illidari, with their
-- warglaives
UPDATE `creature_template` SET `faction` = 16, `ScriptName` = 'npc_beam_me_up_glazer' WHERE `entry` = 96680;
UPDATE `creature_template` SET `faction` = 2804 WHERE `entry` = 99229;
DELETE FROM `creature_equip_template` WHERE `CreatureID` = 99229;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(99229, 1, 128359, 0, 0, 128371, 0, 0, 0, 0, 0, 0);
DELETE FROM `creature_template_addon` WHERE `entry` IN (96680, 99865);
INSERT INTO `creature_template_addon` (`entry`, `emote`, `auras`) VALUES
(96680, 0, '191915'),
(99865, 64, '');

-- The mirrors that bounce Glazer's beam: the statue model only (the other is an invisible stalker), not for clicking
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 191916;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 98082 AND `Idx` = 1;
UPDATE `creature_template` SET `unit_flags` = 33554432 WHERE `entry` = 98082;

-- The Reflective Mirror is the quest's (its glow) and scripted
UPDATE `gameobject_template` SET `Data1` = 39684, `ScriptName` = 'go_beam_me_up_reflective_mirror' WHERE `entry` = 244449;

-- The fight is the player's until the shield is broken; then they see Glazer stunned. The Freed Illidari (96656) stand
-- elsewhere too: only those by Ash'golm or Glazer go.
DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (96680, 99865, 99229))
    OR (`SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 96656 AND (`ElseGroup` = 2 OR `ConditionValue1` = 96680));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 96680, 0, 0, 47, 0, 39684, 9, 0, '', 0, 0, 0, '', 'Glazer: shielded until Beam Me Up is complete'),
(32, 3, 99229, 0, 0, 47, 0, 39684, 9, 0, '', 0, 0, 0, '', 'Altruis (Vault of Mirrors): until Beam Me Up is complete'),
(32, 3, 96656, 0, 0, 29, 1, 96680, 30, 0, '', 1, 0, 0, '', 'Freed Illidari: not by Glazer'),
(32, 3, 96656, 0, 2, 29, 1, 96680, 30, 0, '', 0, 0, 0, '', 'Freed Illidari (Vault of Mirrors): by Glazer'),
(32, 3, 96656, 0, 2, 47, 0, 39684, 9, 0, '', 0, 0, 0, '', 'Freed Illidari (Vault of Mirrors): until Beam Me Up is complete'),
(32, 3, 99865, 0, 0, 47, 0, 39684, 66, 0, '', 0, 0, 0, '', 'Glazer (stunned): once Beam Me Up is complete');

DELETE FROM `creature` WHERE `guid` BETWEEN 14680271 AND 14680277;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680271, 96680, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4450.98, -673.28, 117.29, 0.0000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', 'VaultOfMirrorsSquad', 0), -- Glazer, Vault of Mirrors (where the dungeon has him)
(14680272, 99865, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4450.98, -673.28, 117.29, 0.0000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Glazer (stunned), Vault of Mirrors
(14680273, 99229, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4463.00, -673.30, 117.29, 3.1416, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfMirrorsSquad', 0), -- Altruis the Sufferer, Vault of Mirrors
(14680274, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4451.00, -661.00, 117.29, 4.7124, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfMirrorsSquad', 0), -- Freed Illidari, Vault of Mirrors
(14680275, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4442.00, -682.00, 117.29, 0.7854, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'VaultOfMirrorsSquad', 0), -- Freed Illidari, Vault of Mirrors
(14680276, 98082, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4433.00, -660.00, 117.29, 0.6000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', 'VaultOfMirrorsLens', 0), -- Lens, Vault of Mirrors (first bounce)
(14680277, 98082, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4478.00, -662.00, 117.29, 3.5000, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', 'VaultOfMirrorsLens', 0); -- Lens, Vault of Mirrors (second bounce)

-- The Reflective Mirror, between the way in and Glazer, at rest facing the second lens; turned, it faces Glazer for a
-- while (npc_beam_me_up_glazer)
DELETE FROM `gameobject` WHERE `guid` = 14680212;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680212, 244449, 1468, 7814, 0, '0', 0, 0, 0, -1, 4466.00, -655.00, 117.29, 5.7551, 0, 0, 0.26098, -0.965344, 120, 255, 1, '', NULL, 0);
