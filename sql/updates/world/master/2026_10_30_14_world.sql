-- Dark Revelations: Nightglaive at the quest's POI (terrain height, position rough). Stellagosa comes for the killer, and
-- clicking her casts 180768, which boards the player and credits the optional ride. Her vehicle is the Vampiric Felbat's
-- (1907): the retail one is not known. Reaching Kayn counts as "Meet back at Illidari Stand", ridden or not.
UPDATE `creature_template` SET `faction`=2856, `ScriptName`='npc_dark_revelations_nightglaive' WHERE `entry`=90621;
UPDATE `creature_template` SET `VehicleId`=1907, `npcflag`=`npcflag`|16777216, `ScriptName`='npc_dark_revelations_stellagosa' WHERE `entry`=90982;
UPDATE `creature_template` SET `ScriptName`='npc_illidari_stand_kayn' WHERE `entry`=89362;
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `Entry`=90982 AND `DifficultyID`=0;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=90982;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(90982, 180768, 1, 0);

DELETE FROM `creature` WHERE `guid`=12201031;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12201031, 90621, 1220, 7334, 7349, '0', 0, 0, 0, 0, -516.00, 7492.00, 74.10, 4.71, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0);
