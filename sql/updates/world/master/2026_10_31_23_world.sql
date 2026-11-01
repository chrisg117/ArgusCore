-- DH campaign, The Hunt (41119 Altruis / 39247 Kayn): the fel bat and the first two steps of scenario 900 on map 1498.
-- From the client: the fel bat's spells 186671 / 188922 / 186709, safe locations 5313 and 5097, the camp explosion
-- 186760 / 186771. Ours: the bat's spot at the quest POI, and that the dive counts once the player has dropped below height 110.
DELETE FROM `creature_queststarter` WHERE `quest` IN (41119, 39247);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(103156, 41119, 0),
(94902, 39247, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (41119, 39247);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(41119, 2048, 41121),
(39247, 2048, 39051);

DELETE FROM `instance_template` WHERE `map`=1498;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1498, 0, 'instance_felsoul_hold');
DELETE FROM `scenarios` WHERE `map`=1498;
INSERT INTO `scenarios` (`map`, `difficulty`, `scenario_A`, `scenario_H`) VALUES
(1498, 12, 900, 900);

UPDATE `creature_template` SET `npcflag`=`npcflag`|16777216 WHERE `entry`=94321;
UPDATE `creature_template` SET `ScriptName`='npc_illidari_fel_bat_felsoul_hold' WHERE `entry`=94324;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=94321;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(94321, 186671, 3, 0);

DELETE FROM `creature` WHERE `guid`=12204108;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204108, 94321, 1220, 7502, 7505, '0', 0, 0, 0, 0, -845.00, 4254.00, 746.28, 0.96, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Illidari Fel Bat, Krasus' Landing

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_felsoul_hold_fly_in';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(186709, 'spell_felsoul_hold_fly_in');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (41119, 39247);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry`=94321;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=186671;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 41119, 0, 0, 47, 0, 40375, 64, 0, 0, 0, 0, '', 'The Hunt (Altruis): Altruis chosen'),
(19, 0, 39247, 0, 0, 47, 0, 40374, 64, 0, 0, 0, 0, '', 'The Hunt (Kayn): Kayn chosen'),
(32, 3, 94321, 0, 0, 47, 0, 41119, 8, 0, 0, 0, 0, '', 'Illidari Fel Bat: The Hunt (Altruis) under way'),
(32, 3, 94321, 0, 1, 47, 0, 39247, 8, 0, 0, 0, 0, '', 'Illidari Fel Bat: The Hunt (Kayn) under way'),
(13, 1, 186671, 0, 0, 31, 0, 3, 94321, 0, 0, 0, 0, '', 'Fly to Felsoul Hold: the fel bat comes to the one at Krasus'' Landing');
