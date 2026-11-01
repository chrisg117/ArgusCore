-- DH campaign, the Fel Hammer: Unbridled Power, Spoils of Victory, Cursed Forge of the Nathrezim, The Hunter's Gaze.
-- From the client: the spells, objects and gossip menus, the quest map points, the scene ids of 203269 and 203790, and the scene
-- packages by name. The enders of the first two quests stand at two places: the spawn at the door of the hall is a second entry
-- of the same character, and the leader moves on once the Fel Hammer is active. Positions other than the map points, who else
-- stands in the hall, and the turn-in texts (written for this server after the Warcraft Wiki's account) are ours.

-- The chain, for demon hunters
DELETE FROM `quest_template_addon` WHERE `ID` IN (41037, 41060, 41062, 41070, 41064, 41066);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(41037, 2048, 41221),
(41060, 2048, 41033),
(41062, 2048, 41037),
(41070, 2048, 41060),
(41064, 2048, 0),
(41066, 2048, 41064);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry`=41064;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 41064, 0, 0, 47, 0, 41062, 64, 0, 0, 'Cursed Forge: after Spoils of Victory (Kayn)'),
(19, 0, 41064, 0, 1, 47, 0, 41070, 64, 0, 0, 'Cursed Forge: after Spoils of Victory (Altruis)');

-- Who gives and takes what. Return to Mardum and Unbridled Power start at the door (108571 and 108572), the hub is 95237 and 95240
DELETE FROM `creature_questender` WHERE `quest` IN (41033, 41221);
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(108571, 41033, 0),
(108572, 41221, 0);
DELETE FROM `creature_queststarter` WHERE `quest` IN (41060, 41037, 41070, 41062, 41064, 41066);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(108571, 41060, 0),
(108572, 41037, 0),
(95237, 41070, 0),
(95240, 41062, 0),
(103025, 41064, 0),
(95238, 41066, 0);
DELETE FROM `creature_questender` WHERE `quest`=41066;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(95238, 41066, 0);

-- The door's Altruis and Kayn take the place of the spawns Return to Mardum made
UPDATE `creature` SET `id`=108571 WHERE `guid`=12204138;
UPDATE `creature` SET `id`=108572 WHERE `guid`=12204139;

DELETE FROM `creature` WHERE `guid` BETWEEN 12204140 AND 12204150;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204140, 95237, 1519, 0, 0, '0', 0, 0, 0, 0, 1551.00, 1414.00, 237.08, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Altruis, the hub
(12204141, 95240, 1519, 0, 0, '0', 0, 0, 0, 0, 1551.00, 1414.00, 237.08, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Kayn, the hub
(12204142, 103068, 1519, 0, 0, '0', 0, 0, 0, 0, 1561.00, 1425.00, 237.23, 3.90, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Gaardoun, waiting to show the spoils
(12204143, 103025, 1519, 0, 0, '0', 0, 0, 0, 0, 1584.00, 1409.00, 218.10, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Gaardoun, below
(12204144, 95238, 1519, 0, 0, '0', 0, 0, 0, 0, 1569.00, 1411.00, 218.12, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Allari, by the crucible
(12204145, 95241, 1519, 0, 0, '0', 0, 0, 0, 0, 1575.00, 1395.00, 218.04, 1.57, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Jace
(12204146, 102923, 1519, 0, 0, '0', 0, 0, 0, 0, 1545.00, 1405.00, 237.14, 0.70, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- the captains, rallied
(12204147, 103027, 1519, 0, 0, '0', 0, 0, 0, 0, 1545.00, 1422.00, 236.92, 5.50, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204148, 102801, 1519, 0, 0, '0', 0, 0, 0, 0, 1555.00, 1405.00, 237.18, 2.40, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204149, 95244, 1519, 0, 0, '0', 0, 0, 0, 0, 1555.00, 1422.00, 237.17, 3.90, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 12204007 AND 12204009;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(12204007, 246881, 1519, 0, 0, '0', 0, 0, 1528.00, 1412.00, 245.11, 3.14, 0, 0, 0, 1, 120, 255, 1, 0), -- the console
(12204008, 250244, 1519, 0, 0, '0', 0, 0, 1625.00, 1412.00, 209.03, 3.14, 0, 0, 0, 1, 120, 255, 1, 0), -- the forge
(12204009, 250677, 1519, 0, 0, '0', 0, 0, 1562.00, 1413.00, 217.93, 0, 0, 0, 0, 1, 120, 255, 1, 0); -- the crucible

UPDATE `creature_template` SET `ScriptName`='npc_illidari_leader_fel_hammer' WHERE `entry` IN (95237, 95240);
UPDATE `creature_template` SET `ScriptName`='npc_battlelord_gaardoun_spoils', `npcflag`=1 WHERE `entry`=103068;
UPDATE `creature_template` SET `ScriptName`='npc_battlelord_gaardoun_summoned' WHERE `entry`=103047;
UPDATE `gameobject_template` SET `ScriptName`='go_dh_cursed_forge_of_the_nathrezim' WHERE `entry`=250244;
UPDATE `gameobject_template` SET `ScriptName`='go_dh_empowered_nether_crucible' WHERE `entry`=250677;
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry` IN (108571, 108572, 103025, 103047, 103068, 95238, 95241, 102923, 103027, 102801, 95244);

DELETE FROM `creature_template_gossip` WHERE `CreatureID`=103068;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(103068, 19287, 0);

-- Who stands where and when; state masks: 1 none, 2 complete, 8 in progress, 64 rewarded
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (95237, 95240, 108571, 108572, 103068, 103025, 95241, 102923, 103027, 102801, 95244);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 108571, 0, 0, 47, 0, 40375, 64, 0, 0, 'Altruis at the door: Altruis chosen'),
(32, 3, 108571, 0, 0, 47, 0, 41060, 9, 0, 0, 'Altruis at the door: until the Fel Hammer is active'),
(32, 3, 108572, 0, 0, 47, 0, 40374, 64, 0, 0, 'Kayn at the door: Kayn chosen'),
(32, 3, 108572, 0, 0, 47, 0, 41037, 9, 0, 0, 'Kayn at the door: until the Fel Hammer is active'),
(32, 3, 95237, 0, 0, 47, 0, 40375, 64, 0, 0, 'Altruis at the hub: Altruis chosen'),
(32, 3, 95237, 0, 0, 47, 0, 41060, 66, 0, 0, 'Altruis at the hub: once the Fel Hammer is active'),
(32, 3, 95240, 0, 0, 47, 0, 40374, 64, 0, 0, 'Kayn at the hub: Kayn chosen'),
(32, 3, 95240, 0, 0, 47, 0, 41037, 66, 0, 0, 'Kayn at the hub: once the Fel Hammer is active'),
(32, 3, 103068, 0, 0, 47, 0, 41037, 66, 0, 0, 'Gaardoun above: once the Fel Hammer is active (Kayn)'),
(32, 3, 103068, 0, 0, 47, 0, 41062, 9, 0, 0, 'Gaardoun above: until he has shown the spoils (Kayn)'),
(32, 3, 103068, 0, 1, 47, 0, 41060, 66, 0, 0, 'Gaardoun above: once the Fel Hammer is active (Altruis)'),
(32, 3, 103068, 0, 1, 47, 0, 41070, 9, 0, 0, 'Gaardoun above: until he has shown the spoils (Altruis)'),
(32, 3, 103025, 0, 0, 47, 0, 41037, 66, 0, 0, 'Gaardoun below: once the Fel Hammer is active (Kayn)'),
(32, 3, 103025, 0, 1, 47, 0, 41060, 66, 0, 0, 'Gaardoun below: once the Fel Hammer is active (Altruis)'),
(32, 3, 95241, 0, 0, 47, 0, 41037, 66, 0, 0, 'Jace: once the Fel Hammer is active (Kayn)'),
(32, 3, 95241, 0, 1, 47, 0, 41060, 66, 0, 0, 'Jace: once the Fel Hammer is active (Altruis)'),
(32, 3, 102923, 0, 0, 47, 0, 41037, 66, 0, 0, 'Malevolence: once the Fel Hammer is active (Kayn)'),
(32, 3, 102923, 0, 1, 47, 0, 41060, 66, 0, 0, 'Malevolence: once the Fel Hammer is active (Altruis)'),
(32, 3, 103027, 0, 0, 47, 0, 41037, 66, 0, 0, 'S''theno: once the Fel Hammer is active (Kayn)'),
(32, 3, 103027, 0, 1, 47, 0, 41060, 66, 0, 0, 'S''theno: once the Fel Hammer is active (Altruis)'),
(32, 3, 102801, 0, 0, 47, 0, 41037, 66, 0, 0, 'Kor''vas: once the Fel Hammer is active (Kayn)'),
(32, 3, 102801, 0, 1, 47, 0, 41060, 66, 0, 0, 'Kor''vas: once the Fel Hammer is active (Altruis)'),
(32, 3, 95244, 0, 0, 47, 0, 41037, 66, 0, 0, 'Belath: once the Fel Hammer is active (Kayn)'),
(32, 3, 95244, 0, 1, 47, 0, 41060, 66, 0, 0, 'Belath: once the Fel Hammer is active (Altruis)');

-- The console and the crucible answer only while their quest is open, and Gaardoun appears by the one he is summoned from
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=17 AND `SourceEntry`=203442;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup` IN (19287, 19337);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=203676;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(17, 0, 203442, 0, 0, 47, 0, 41037, 8, 0, 0, 'Assuming Control: during Unbridled Power (Kayn)'),
(17, 0, 203442, 0, 1, 47, 0, 41060, 8, 0, 0, 'Assuming Control: during Unbridled Power (Altruis)'),
(15, 19287, 0, 0, 0, 47, 0, 41062, 8, 0, 0, 'Show me what we acquired: during Spoils of Victory (Kayn)'),
(15, 19287, 0, 0, 1, 47, 0, 41070, 8, 0, 0, 'Show me what we acquired: during Spoils of Victory (Altruis)'),
(15, 19337, 0, 0, 0, 47, 0, 41066, 8, 0, 0, 'Gaze upon the Burning Legion: during The Hunter''s Gaze'),
(13, 1, 203676, 0, 0, 31, 0, 3, 103068, 0, 0, 'Summon Battlelord Gaardoun: where he stands');

-- The scenes the two spells play, and the way back the gateway ability has no row for
DELETE FROM `scene_template` WHERE `SceneId` IN (1201, 1203);
INSERT INTO `scene_template` (`SceneId`, `Flags`, `ScriptPackageID`, `ScriptName`) VALUES
(1201, 16, 1564, 'scene_dh_activate_the_fel_hammer'),
(1203, 16, 1567, 'scene_dh_nether_crucible');

DELETE FROM `spell_target_position` WHERE `ID`=193752;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `VerifiedBuild`) VALUES
(193752, 0, 0, 1519, 1474.17, 1411.46, 243.40, 0);

DELETE FROM `creature_text` WHERE `CreatureID`=103047;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(103047, 0, 0, 'Come. What we took from this place is below.', 12, 0, 100, 0, 0, 0, 0, 0, 'Battlelord Gaardoun, leading the way');

DELETE FROM `quest_offer_reward` WHERE `ID` IN (42869, 42872, 41033, 41221, 41060, 41037, 41070, 41062, 41064, 41066);
INSERT INTO `quest_offer_reward` (`ID`, `RewardText`) VALUES
(42869, 'You came quickly. Good. What I need of you is not small.'),
(42872, 'The way to the Fel Hammer stands open, and it will hold. Our forces can move at last.'),
(41033, 'So you have come aboard. Now you will see what we kept for you.'),
(41221, 'You have come home, $n. We held this place until you could claim it.'),
(41060, 'They have seen your power. Now they will follow you, and the Legion will pay for it.'),
(41037, 'The Fel Hammer answers to you. Lord Illidan would be pleased.'),
(41070, 'While you were gone at the Black Temple, the Legion came through behind you. We destroyed the portal, and under this citadel we found what they had been hiding: a Nathrezim forge.'),
(41062, 'While you were gone at the Black Temple, the Legion came through behind you. We destroyed the portal, and under this citadel we found what they had been hiding: a Nathrezim forge.'),
(41064, 'The forge knows its new masters. Many of the Legion''s worst weapons were made on it, and now it serves yours.'),
(41066, 'The Tomb of Sargeras is the Legion''s gate into Azeroth, and they already hold the isles around it. We must not wait.');
