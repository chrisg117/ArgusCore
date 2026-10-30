-- Demon Souls and The Scythe of Souls (zone_azsuna.cpp). No sniff of The Scythe of Souls' staging or dialogue exists: the
-- gossip lines are ours, Allari and the souls stand at the quest's POIs, and Arev'naal is compelled by talking to them.
UPDATE `creature_template` SET `ScriptName`='npc_demon_souls_soul_gem' WHERE `entry`=86963;
UPDATE `creature_template` SET `ScriptName`='npc_illidari_stand_allari' WHERE `entry`=89398;
UPDATE `creature_template` SET `ScriptName`='npc_scythe_of_souls_allari', `npcflag`=`npcflag`|1 WHERE `entry`=90403;
UPDATE `creature_template` SET `ScriptName`='npc_scythe_of_souls_arevnaal', `npcflag`=`npcflag`|1 WHERE `entry`=89673;

DELETE FROM `gossip_menu` WHERE `MenuID` IN (7334001, 7334002);
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(7334001, 30317, 0),
(7334002, 1, 0);
DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (7334001, 7334002);
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(7334001, 0, 0, 'I am ready. Let us question the souls.', 0, 0, 0, 0, 0, NULL, 0, 0),
(7334002, 0, 0, 'Speak, demon. What do you know of Illidan?', 0, 0, 0, 0, 0, NULL, 0, 0);
DELETE FROM `creature_template_gossip` WHERE `CreatureID` IN (90403, 89673);
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(90403, 7334001, 0),
(89673, 7334002, 0);

-- "Leave soul gem" is at the quest's POI; nothing spawned it
DELETE FROM `gameobject` WHERE `guid`=12201101;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(12201101, 240012, 1220, 7334, 7364, '0', 0, 0, -132.00, 6915.00, 13.80, 0, 0, 0, 0, 1, 120, 255, 1, 0);
