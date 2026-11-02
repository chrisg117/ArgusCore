-- The Fel Hammer's hall systems: upgrades, recruiters, mission table, provisioner.
-- Spots: the client's quest map points where a quest names the NPC (Kor'vas 1551,1411, Malevolence 1576,1408, Asha 1603,1424,
-- Loramus 1536,1428, the mission objectives 1561,1413), Wowhead's pins anchored on Kor'vas and Asha for Falara; heights from the mesh.
-- Falara's list is Wowhead's sell list; every item is in the client's Item.db2. A Tiny Set of Warglaives costs 1000 Order Resources (extended cost 6075).

-- Who stands in the hall once The Hunter's Gaze is done: the captains of the rally make way for their campaign entries
DELETE FROM `creature` WHERE `guid` BETWEEN 15191001 AND 15191006;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(15191001, 103761, 1519, 0, 0, '0', 0, 0, 0, 0, 1551.00, 1409.50, 237.10, 0.70, 120, 0, 0, NULL, NULL, NULL, NULL, 0), -- Kor'vas, the campaign
(15191002, 98632, 1519, 0, 0, '0', 0, 0, 0, 0, 1576.00, 1408.00, 237.23, 3.14, 120, 0, 0, NULL, NULL, NULL, NULL, 0), -- Malevolence, Illidari Adepts
(15191003, 110599, 1519, 0, 0, '0', 0, 0, 0, 0, 1538.00, 1428.00, 220.16, 0.00, 120, 0, 0, NULL, NULL, NULL, NULL, 0), -- Loramus, class hall upgrades
(15191004, 108326, 1519, 0, 0, '0', 0, 0, 0, 0, 1603.00, 1424.00, 244.26, 3.14, 120, 0, 0, NULL, NULL, NULL, NULL, 0), -- Asha
(15191005, 95242, 1519, 0, 0, '0', 0, 0, 0, 0, 1541.00, 1413.00, 237.16, 0.00, 120, 0, 0, NULL, NULL, NULL, NULL, 0), -- Falara, provisioner
(15191006, 102589, 1519, 0, 0, '0', 0, 0, 64062, 0, 1561.00, 1413.00, 237.23, 3.14, 300, 0, 0, NULL, NULL, NULL, NULL, 0); -- Command Map, over the table

UPDATE `creature_template` SET `npcflag`=3 WHERE `entry` IN (103068, 98632);
UPDATE `creature_template` SET `npcflag`=1 WHERE `entry`=110599;
UPDATE `creature_template` SET `npcflag`=128 WHERE `entry`=95242;
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry` IN (103761, 98632, 110599, 108326, 95242);

-- Gaardoun stays at the hub after Spoils of Victory, as the recruiter; his forge wing self is done with The Cursed Forge.
-- Visibility conditions go by entry, not spawn, so one entry is one place for one player. State masks: 8 in progress, 2 complete, 64 rewarded.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (103068, 103025, 102801, 102923, 103761, 98632, 108326);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 103068, 0, 0, 47, 0, 41037, 66, 0, 0, 'Gaardoun at the hub: once the Fel Hammer is active (Kayn)'),
(32, 3, 103068, 0, 1, 47, 0, 41060, 66, 0, 0, 'Gaardoun at the hub: once the Fel Hammer is active (Altruis)'),
(32, 3, 103025, 0, 0, 47, 0, 41037, 66, 0, 0, 'Gaardoun below: once the Fel Hammer is active (Kayn)'),
(32, 3, 103025, 0, 0, 47, 0, 41064, 64, 0, 1, 'Gaardoun below: until The Cursed Forge is done (Kayn)'),
(32, 3, 103025, 0, 1, 47, 0, 41060, 66, 0, 0, 'Gaardoun below: once the Fel Hammer is active (Altruis)'),
(32, 3, 103025, 0, 1, 47, 0, 41064, 64, 0, 1, 'Gaardoun below: until The Cursed Forge is done (Altruis)'),
(32, 3, 102801, 0, 0, 47, 0, 41037, 66, 0, 0, 'Kor''vas rallied: once the Fel Hammer is active (Kayn)'),
(32, 3, 102801, 0, 0, 47, 0, 41066, 64, 0, 1, 'Kor''vas rallied: until The Hunter''s Gaze is done (Kayn)'),
(32, 3, 102801, 0, 1, 47, 0, 41060, 66, 0, 0, 'Kor''vas rallied: once the Fel Hammer is active (Altruis)'),
(32, 3, 102801, 0, 1, 47, 0, 41066, 64, 0, 1, 'Kor''vas rallied: until The Hunter''s Gaze is done (Altruis)'),
(32, 3, 102923, 0, 0, 47, 0, 41037, 66, 0, 0, 'Malevolence rallied: once the Fel Hammer is active (Kayn)'),
(32, 3, 102923, 0, 0, 47, 0, 41066, 64, 0, 1, 'Malevolence rallied: until The Hunter''s Gaze is done (Kayn)'),
(32, 3, 102923, 0, 1, 47, 0, 41060, 66, 0, 0, 'Malevolence rallied: once the Fel Hammer is active (Altruis)'),
(32, 3, 102923, 0, 1, 47, 0, 41066, 64, 0, 1, 'Malevolence rallied: until The Hunter''s Gaze is done (Altruis)'),
(32, 3, 103761, 0, 0, 47, 0, 41066, 64, 0, 0, 'Kor''vas of the campaign: after The Hunter''s Gaze'),
(32, 3, 98632, 0, 0, 47, 0, 41066, 64, 0, 0, 'Malevolence of the campaign: after The Hunter''s Gaze'),
(32, 3, 108326, 0, 0, 47, 0, 41066, 64, 0, 0, 'Asha: after The Hunter''s Gaze');

-- The menus. Gaardoun keeps his Spoils menu while that quest is open (the later menu wins when its condition holds).
DELETE FROM `gossip_menu` WHERE `MenuID` IN (1519010, 1519011, 1519012);
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(1519010, 68, 0),
(1519011, 28361, 0),
(1519012, 68, 0);

DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (1519010, 1519011, 1519012);
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `VerifiedBuild`) VALUES
(1519010, 0, 32, 'What can be done to improve the Fel Hammer?', 0),
(1519011, 0, 28, 'I need Ashtongue Warriors.', 0),
(1519011, 1, 28, 'I need Ashtongue Warriors.', 0),
(1519012, 0, 28, 'I need Illidari Adepts.', 0);

DELETE FROM `gossip_menu_option_addon` WHERE `MenuID` IN (1519010, 1519011, 1519012);
INSERT INTO `gossip_menu_option_addon` (`MenuID`, `OptionID`, `GarrTalentTreeID`, `CharShipmentID`, `VerifiedBuild`) VALUES
(1519010, 0, 125, NULL, 0),
(1519011, 0, NULL, 330, 0),
(1519011, 1, NULL, 263, 0),
(1519012, 0, NULL, 264, 0);

DELETE FROM `creature_template_gossip` WHERE `CreatureID` IN (110599, 98632) OR (`CreatureID`=103068 AND `MenuID`=1519011);
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(110599, 1519010, 0),
(98632, 1519012, 0),
(103068, 1519011, 0);

DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId`=14 AND `SourceGroup`=1519011) OR (`SourceTypeOrReferenceId`=15 AND `SourceGroup`=1519011);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(14, 1519011, 28361, 0, 0, 47, 0, 41062, 8, 0, 1, 'Gaardoun recruits: not during Spoils of Victory (Kayn)'),
(14, 1519011, 28361, 0, 0, 47, 0, 41070, 8, 0, 1, 'Gaardoun recruits: not during Spoils of Victory (Altruis)'),
(15, 1519011, 0, 0, 0, 47, 0, 42679, 10, 0, 0, 'The tutorial order: during Broken Warriors'),
(15, 1519011, 1, 0, 0, 47, 0, 42679, 10, 0, 1, 'The real order: otherwise');

-- The orders are picked up from these; no Legion object of this type exists, so the Warlords work order looks stand in
DELETE FROM `gameobject_template` WHERE `entry` IN (1519001, 1519002, 1519003, 1519004);
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `Data0`, `VerifiedBuild`) VALUES
(1519001, 45, 15585, 'Ashtongue Training', 1, 137, 0),
(1519002, 45, 15585, 'Illidari Training', 1, 138, 0),
(1519003, 5, 26338, 'Command Map', 1.5, 0, 0), -- 7dh_demonhunter_battlemap01.m2, a flat map
(1519004, 5, 26249, 'Command Table', 1.5, 0, 0); -- 7dh_demonhunter_table02.m2, 1.2 yd high at size 1

-- flags: GO_FLAG_NOT_SELECTABLE and GO_FLAG_NODESPAWN, so that pointing at the table finds the Command Map
DELETE FROM `gameobject_template_addon` WHERE `entry` IN (1519003, 1519004);
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`) VALUES
(1519003, 0, 0x30),
(1519004, 0, 0x30);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 15191001 AND 15191004;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(15191001, 1519001, 1519, 0, 0, '0', 0, 0, 1564.00, 1428.00, 237.23, 3.14, 0, 0, 0, 1, 120, 255, 1, 0),
(15191002, 1519002, 1519, 0, 0, '0', 0, 0, 1579.00, 1405.00, 237.27, 3.14, 0, 0, 0, 1, 120, 255, 1, 0),
(15191003, 1519003, 1519, 0, 0, '0', 0, 0, 1561.00, 1413.00, 239.03, 3.14, 0, 0, 0, 1, 300, 255, 1, 0),
(15191004, 1519004, 1519, 0, 0, '0', 0, 0, 1561.00, 1413.00, 237.23, 3.14, 0, 0, 0, 1, 300, 255, 1, 0);

DELETE FROM `npc_vendor` WHERE `entry`=95242;
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `type`, `VerifiedBuild`) VALUES
(95242, 0, 132752, 0, 0, 0, 1, 0),
(95242, 1, 112450, 0, 0, 0, 1, 0),
(95242, 2, 112451, 0, 0, 0, 1, 0),
(95242, 3, 112452, 0, 0, 0, 1, 0),
(95242, 4, 112453, 0, 0, 0, 1, 0),
(95242, 5, 112454, 0, 0, 0, 1, 0),
(95242, 6, 112455, 0, 0, 0, 1, 0),
(95242, 7, 112456, 0, 0, 0, 1, 0),
(95242, 8, 112457, 0, 0, 0, 1, 0),
(95242, 9, 112458, 0, 0, 0, 1, 0),
(95242, 10, 112459, 0, 0, 0, 1, 0),
(95242, 11, 112460, 0, 0, 0, 1, 0),
(95242, 12, 112461, 0, 0, 0, 1, 0),
(95242, 13, 112462, 0, 0, 0, 1, 0),
(95242, 14, 123959, 0, 0, 0, 1, 0),
(95242, 15, 123960, 0, 0, 0, 1, 0),
(95242, 16, 129181, 0, 0, 0, 1, 0),
(95242, 17, 133318, 0, 0, 0, 1, 0),
(95242, 18, 133319, 0, 0, 0, 1, 0),
(95242, 19, 133320, 0, 0, 0, 1, 0),
(95242, 20, 133321, 0, 0, 0, 1, 0),
(95242, 21, 133322, 0, 0, 0, 1, 0),
(95242, 22, 133323, 0, 0, 0, 1, 0),
(95242, 23, 133324, 0, 0, 0, 1, 0),
(95242, 24, 133325, 0, 0, 0, 1, 0),
(95242, 25, 147537, 0, 0, 6075, 1, 0);
