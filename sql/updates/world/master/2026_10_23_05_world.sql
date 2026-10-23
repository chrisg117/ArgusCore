-- Breaking Out (38672), the first quest in the Vault of the Wardens (map 1468), which had no spawns at all. Maiev
-- Shadowsong (92718) wakes the player, gives the quest and leaves; the cells of Altruis (103655) and Kayn (103658) are
-- clicked to free them (npc_breaking_out_cell). Each cell and its prisoner (92849 / 92848) is shown until that prisoner
-- is freed, the freed Altruis (92986) and Kayn (92980) by the lever (244923, which ends the quest) from then on.
-- Positions from Wowhead pins (Vault of the Wardens floor 1) and the retail video, heights from the navmesh.

DELETE FROM `creature_queststarter` WHERE `id` = 92718 AND `quest` = 38672;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(92718, 38672, 0);

DELETE FROM `gameobject_questender` WHERE `id` = 244923 AND `quest` = 38672;
INSERT INTO `gameobject_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(244923, 38672, 0);

DELETE FROM `creature` WHERE `guid` BETWEEN 14680001 AND 14680007;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680001, 92718, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4326.3, -609.1, -281.86, 4.712, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Maiev, facing where the player wakes
(14680002, 103655, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4318.2, -589.7, -281.64, 0.000, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Altruis's Cell
(14680003, 92849, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4318.2, -589.7, -281.64, 0.000, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Altruis inside it
(14680004, 103658, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4334.5, -589.7, -281.33, 3.142, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kayn's Cell
(14680005, 92848, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4334.5, -589.7, -281.33, 3.142, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kayn inside it
(14680006, 92980, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4328.4, -580.5, -281.64, 4.712, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kayn freed
(14680007, 92986, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4323.5, -580.5, -281.64, 4.712, 30, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Altruis freed

DELETE FROM `gameobject` WHERE `guid` = 14680001;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680001, 244923, 1468, 7814, 0, '0', 0, 0, 0, -1, 4332.5, -583.5, -281.64, 3.14159, 0, 0, 1, 0, 120, 255, 1, '', NULL, 0);

-- The cells are clicked (spell click, as on retail: "Free Altruis/Kayn: Master") while that prisoner is still locked up
UPDATE `creature_template` SET `npcflag` = 16777216, `ScriptName` = 'npc_breaking_out_cell' WHERE `entry` IN (103655, 103658);

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (103655, 103658);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(103655, 192192, 1, 0),
(103658, 194029, 1, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` IN (103655, 103658);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 103655, 192192, 0, 0, 47, 0, 38672, 8, 0, '', 0, 0, 0, '', 'Altruis''s Cell: click while Breaking Out is in progress'),
(18, 103655, 192192, 0, 0, 48, 0, 278274, 0, 1, '', 1, 0, 0, '', 'Altruis''s Cell: Altruis not freed yet'),
(18, 103658, 194029, 0, 0, 47, 0, 38672, 8, 0, '', 0, 0, 0, '', 'Kayn''s Cell: click while Breaking Out is in progress'),
(18, 103658, 194029, 0, 0, 48, 0, 278275, 0, 1, '', 1, 0, 0, '', 'Kayn''s Cell: Kayn not freed yet');

-- Locked up until freed (also hidden once the quest is complete or done, when the objectives are no longer stored);
-- the freed ones the other way round
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (103655, 92849, 103658, 92848, 92986, 92980);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 103655, 0, 0, 48, 0, 278274, 0, 1, '', 1, 0, 0, '', 'Altruis''s Cell: shown until Altruis is freed'),
(32, 3, 103655, 0, 0, 47, 0, 38672, 66, 0, '', 1, 0, 0, '', 'Altruis''s Cell: hidden once Breaking Out is complete or done'),
(32, 3, 92849, 0, 0, 48, 0, 278274, 0, 1, '', 1, 0, 0, '', 'Altruis (cell): shown until freed'),
(32, 3, 92849, 0, 0, 47, 0, 38672, 66, 0, '', 1, 0, 0, '', 'Altruis (cell): hidden once Breaking Out is complete or done'),
(32, 3, 103658, 0, 0, 48, 0, 278275, 0, 1, '', 1, 0, 0, '', 'Kayn''s Cell: shown until Kayn is freed'),
(32, 3, 103658, 0, 0, 47, 0, 38672, 66, 0, '', 1, 0, 0, '', 'Kayn''s Cell: hidden once Breaking Out is complete or done'),
(32, 3, 92848, 0, 0, 48, 0, 278275, 0, 1, '', 1, 0, 0, '', 'Kayn (cell): shown until freed'),
(32, 3, 92848, 0, 0, 47, 0, 38672, 66, 0, '', 1, 0, 0, '', 'Kayn (cell): hidden once Breaking Out is complete or done'),
(32, 3, 92986, 0, 0, 48, 0, 278274, 0, 1, '', 0, 0, 0, '', 'Altruis (freed): shown once freed'),
(32, 3, 92986, 0, 1, 47, 0, 38672, 66, 0, '', 0, 0, 0, '', 'Altruis (freed): shown once Breaking Out is complete or done'),
(32, 3, 92980, 0, 0, 48, 0, 278275, 0, 1, '', 0, 0, 0, '', 'Kayn (freed): shown once freed'),
(32, 3, 92980, 0, 1, 47, 0, 38672, 66, 0, '', 0, 0, 0, '', 'Kayn (freed): shown once Breaking Out is complete or done');

-- Retail lines (voice files vo_701_maiev_shadowsong / vo_701_kayn_sunfury / vo_701_altruis_the_sufferer)
DELETE FROM `creature_text` WHERE `CreatureID` IN (92718, 92980, 92986);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(92718, 0, 0, 'Wake up, demon hunter.', 12, 0, 100, 0, 0, 57335, 0, 98727, 0, 'Maiev Shadowsong - player wakes'),
(92718, 1, 0, 'Take up your weapons, $n. I trust you can still handle a blade.', 12, 0, 100, 0, 0, 57336, 0, 98731, 0, 'Maiev Shadowsong - player wakes 2'),
(92718, 2, 0, 'I will need your help to stop Gul''dan. Free the other Illidari, quickly!', 12, 0, 100, 0, 0, 57338, 0, 98734, 0, 'Maiev Shadowsong - Breaking Out accepted'),
(92980, 0, 0, 'I am awake? How is this possible?', 12, 0, 100, 0, 0, 57294, 0, 100787, 0, 'Kayn Sunfury - freed'),
(92980, 1, 0, 'Why did you let THAT one free? He opposed Lord Illidan!', 12, 0, 100, 0, 0, 57320, 0, 91600, 0, 'Kayn Sunfury - reunion'),
(92980, 2, 0, 'The demons aim to destroy our world. We need to release our allies.', 12, 0, 100, 0, 0, 57340, 0, 98738, 0, 'Kayn Sunfury - reunion 2'),
(92986, 0, 0, 'How many years were wasted in that cell?', 12, 0, 100, 0, 0, 57343, 0, 100833, 0, 'Altruis the Sufferer - freed'),
(92986, 1, 0, 'It has been a long time, Kayn. I still believe in the cause. But, I do not blindly follow anyone... even Illidan.', 12, 0, 100, 0, 0, 57285, 0, 91916, 0, 'Altruis the Sufferer - reunion'),
(92986, 2, 0, 'Time for answers later. There are demons to kill.', 12, 0, 100, 0, 0, 57339, 0, 98737, 0, 'Altruis the Sufferer - reunion 2');

-- Maiev wakes a player who has not started Breaking Out, and once they take it she heads out through the door
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 92718;
DELETE FROM `smart_scripts` WHERE (`source_type` = 0 AND `entryorguid` = 92718) OR (`source_type` = 9 AND `entryorguid` IN (9271800, 9271801));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `Difficulties`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param_string`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `action_param7`, `action_param_string`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_param_string`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(92718, 0, 0, 0, '', 10, 0, 100, 0, 1, 20, 60000, 60000, 1, '', 80, 9271800, 2, 0, 0, 0, 0, 0, NULL, 1, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Maiev Shadowsong - On player in sight - Run wake-up lines'),
(92718, 0, 1, 0, '', 19, 0, 100, 0, 38672, 0, 0, 0, 0, '', 80, 9271801, 2, 0, 0, 0, 0, 0, NULL, 1, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Maiev Shadowsong - On Breaking Out accepted - Run leave'),
(9271800, 9, 0, 0, '', 0, 0, 100, 0, 0, 0, 0, 0, 0, '', 1, 0, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Maiev Shadowsong - Talk 0 to invoker'),
(9271800, 9, 1, 0, '', 0, 0, 100, 0, 4000, 4000, 0, 0, 0, '', 1, 1, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Maiev Shadowsong - After 4s - Talk 1 to invoker'),
(9271801, 9, 0, 0, '', 0, 0, 100, 0, 0, 0, 0, 0, 0, '', 1, 2, 0, 0, 0, 0, 0, 0, NULL, 7, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Maiev Shadowsong - Talk 2 to invoker'),
(9271801, 9, 1, 0, '', 0, 0, 100, 0, 5000, 5000, 0, 0, 0, '', 69, 0, 0, 0, 0, 0, 0, 0, NULL, 8, 0, 0, 0, 0, NULL, 4326.3, -566.0, -281.69, 0, 'Maiev Shadowsong - After 5s - Move to the door'),
(9271801, 9, 2, 0, '', 0, 0, 100, 0, 8000, 8000, 0, 0, 0, '', 41, 0, 30, 0, 0, 0, 0, 0, NULL, 1, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Maiev Shadowsong - After 8s - Despawn, back in 30s');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 92718;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, 92718, 0, 0, 47, 0, 38672, 1, 0, '', 0, 0, 0, '', 'Maiev Shadowsong: wake-up lines only for a player who has not started Breaking Out');
