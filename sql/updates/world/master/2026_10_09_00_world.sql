-- Broken Shore scenario (map 1460), "Storm the Beach": the Spires of Woe fire beams at the ground.
--
-- A video of the retail scenario shows a beam from the eye at the top of each spire down to the ground among
-- those who storm the beach. None of it is in the data, so it is put together from what the client has.
-- "Before We're Overrun: Fel Beam" (192664), named after a quest of Mardum, where Spires of Woe stand too, is
-- channelled by one of the creatures named "Spire of Woe" (97624), placed at the eye, at another of that name
-- (97629), which the first summons on the ground. Which creature does what on retail is not known, nor are
-- their spawns.
--
-- Both are triggers, which gives them the invisible one of the two models each has.
UPDATE `creature_template` SET `flags_extra`=`flags_extra`|0x80, `ScriptName`='npc_broken_shore_spire_of_woe' WHERE `entry`=97624;
UPDATE `creature_template` SET `flags_extra`=`flags_extra`|0x80 WHERE `entry`=97629;

-- The one at the top floats there (CREATURE_STATIC_FLAG_FLOATING): 20.6 yards above the foot of its spire,
-- where the spire's model has the glow of its eye. They belong to the spires' spawn group.
UPDATE `creature_template_difficulty` SET `StaticFlags1`=`StaticFlags1`|0x20000000 WHERE `Entry`=97624;

DELETE FROM `creature` WHERE `guid` BETWEEN 14600137 AND 14600139;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14600137, 97624, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 512.5, 2155.9, 25.49, 4.0013, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0),
(14600138, 97624, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 546.8, 2117.4, 25.31, 3.5226, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0),
(14600139, 97624, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 569.0, 2059.7, 24.31, 3.0111, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0);

DELETE FROM `spawn_group` WHERE `spawnType`=0 AND `spawnId` BETWEEN 14600137 AND 14600139;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14613, 0, 14600137),
(14613, 0, 14600138),
(14613, 0, 14600139);

-- The beam takes every creature within 110 yards of its caster that conditions let through; the spell's
-- script then leaves a spire the one target it has summoned.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceGroup`=1 AND `SourceEntry`=192664;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 192664, 0, 0, 31, 0, 3, 97629, 0, '', 0, 0, 0, '', 'Before We''re Overrun: Fel Beam - target Spire of Woe (97629)');

DELETE FROM `spell_script_names` WHERE `spell_id`=192664 AND `ScriptName`='spell_broken_shore_fel_beam';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(192664, 'spell_broken_shore_fel_beam');
