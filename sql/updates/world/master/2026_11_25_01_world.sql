-- Azsuna, Save Yourself (37530): Prince Farondis's account of how he saved you, played as Farondis. After The Tidestone:
-- Shattered (37469) the naga carry the player to the Hatecoil Slave Pen; Farondis (89009) there gives the quest, the player
-- rides him (vehicle 89089, phase 4321 from 178284) through the account, and turns it in to their own body (89090).
-- The scripts are in zone_azsuna.cpp. Positions: navmesh heights, at least 4 yd from walls (6 for scene points); the
-- source of each is noted on its row.

-- chain: 37469 -> 37530 -> 37470 (neither had an addon row); the 37469 hand-over and 37530's ending are a player script
DELETE FROM `quest_template_addon` WHERE `ID` IN (37530, 37470);
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(37530, 37469),
(37470, 37530);

-- Prince Farondis at the cave mouth: the start and turn-in POI is -98,6018, 0.8 yd from the cave wall; moved 4 yd out
DELETE FROM `creature` WHERE `guid` = 12205300;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12205300, 89009, 1220, 7334, 0, '0', 0, 0, 0, 0, -94.49, 6019.93, 0.49, 5.54, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0);
DELETE FROM `creature_queststarter` WHERE `id` = 89009 AND `quest` IN (37530, 37470);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(89009, 37530, 0),
(89009, 37470, 0);
UPDATE `creature_template` SET `ScriptName` = 'npc_farondis_slave_pen' WHERE `entry` = 89009;

-- his gossip after the accept (the quest launches it): "Listen well, $n." and the player's "What happened, Farondis?",
-- shown while the account has not started
DELETE FROM `npc_text` WHERE `ID` = 1220530;
INSERT INTO `npc_text` (`ID`, `Probability0`, `BroadcastTextID0`, `VerifiedBuild`) VALUES
(1220530, 1, 93574, 0);
DELETE FROM `gossip_menu` WHERE `MenuID` = 1220530;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(1220530, 1220530, 0);
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 1220530;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `VerifiedBuild`) VALUES
(1220530, 0, 0, 'What happened, Farondis?', 91808, 0);
DELETE FROM `creature_template_gossip` WHERE `CreatureID` = 89009;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(89009, 1220530, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 1220530;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(15, 1220530, 0, 0, 0, 9, 0, 37530, 0, 0, 0, 'Farondis: the account, while Save Yourself is in the log'),
(15, 1220530, 0, 0, 0, 48, 0, 276735, 0, 0, 0, 'and Speak with Prince Farondis is at 0');

-- the vehicle Farondis: no vehicle id in any data (TDB 735, retail TC TDB 1210); 5565 is the one client vehicle with a
-- single seat that hides its passenger and lets them steer, turn, cast and attack, with no vehicle UI or power bar.
-- Bar: fireball (222305, an instant level-scaled one: the guide's first button, missing from the data), Meteor Storm,
-- Blink, in the guide's order
UPDATE `creature_template` SET `VehicleId` = 5565, `ScriptName` = 'npc_save_yourself_farondis' WHERE `entry` = 89089;
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 89089;
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(89089, 0, 222305, 0),
(89089, 1, 179215, 0),
(89089, 2, 225948, 0);
DELETE FROM `spell_script_names` WHERE `spell_id` = 179215 AND `ScriptName` = 'spell_save_yourself_meteor_storm';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(179215, 'spell_save_yourself_meteor_storm');

-- the naga: Athissa B and Parjesh B fight (Hatecoil faction, as Athissa 88091); the A pair and Parjesh C only carry the
-- prisoner away and can't be attacked
UPDATE `creature_template` SET `ScriptName` = 'npc_save_yourself_naga' WHERE `entry` IN (89116, 89117, 89118, 89119, 89120);
UPDATE `creature_template` SET `faction` = 16 WHERE `entry` IN (89118, 89119);
UPDATE `creature_template` SET `unit_flags` = `unit_flags` | 0x300 WHERE `entry` IN (89116, 89117, 89120, 89315, 89316, 91402);
UPDATE `creature_template` SET `ScriptName` = 'npc_save_yourself_azshara' WHERE `entry` = 91402;
UPDATE `creature_template` SET `ScriptName` = 'npc_save_yourself_doppelganger' WHERE `entry` IN (89090, 89315, 89316);

-- levels (all were 1): 110 with the zone's 98..110 scaling; health modifiers stay the data's
UPDATE `creature_template_difficulty` SET `MinLevel` = 110, `MaxLevel` = 110, `LevelScalingMin` = 98, `LevelScalingMax` = 110
WHERE `DifficultyID` = 0 AND `Entry` IN (89009, 89089, 89090, 89116, 89117, 89118, 89119, 89120, 89315, 89316, 91402);

-- the ropes find their prisoner by entry
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (178758, 178760);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(13, 1, 178758, 0, 0, 31, 0, 3, 89315, 0, 0, 'Save Yourself: Rope A ties Your Doppelganger A'),
(13, 1, 178760, 0, 0, 31, 0, 3, 89316, 0, 0, 'Save Yourself: Rope B ties Your Doppelganger B');

-- the naga on the way through the account are summoned by the script (zone_azsuna.cpp, AccountNagaSpots), not spawned
-- here in phase 4321: a personal-phase spawn that aggroes the player-driven Farondis on its own overflows the stack
DELETE FROM `creature` WHERE `guid` BETWEEN 12205301 AND 12205305;
