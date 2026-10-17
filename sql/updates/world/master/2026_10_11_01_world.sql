-- "Blade in Twilight" (scenario 991): its fifth and sixth stages, "Reconsecration" and "Don't Fear the
-- Reaper", in the hall of the tomb.
--
-- The hall had nothing in it. What happens there is taken from a video of the retail scenario; where things
-- stand was measured on the tomb's collision (the dais in the middle of the hall and the four pedestals at
-- its corners) or, for the four ritualists, estimated from the video. The lines are Blizzard's
-- (broadcast_text 104272, 104275, 105084, 105085 and 106115).

-- The Twilight's Hammer and what it has called up fight the player, and are of the level of the player who
-- looks at them; Slaghammer, as he goes through the tomb with the player, is a friend who fights them and
-- answers questions. All four templates were placeholders: friendly to all, of level 1, without scripts.
UPDATE `creature_template` SET `faction`=16, `ScriptName`='npc_twilight_ritualist' WHERE `entry`=101875;
UPDATE `creature_template` SET `faction`=16 WHERE `entry`=101120;
UPDATE `creature_template` SET `faction`=16, `ScriptName`='npc_amassing_darkness' WHERE `entry`=102693;
UPDATE `creature_template` SET `faction`=495, `npcflag`=1, `ScriptName`='npc_shadowlord_slaghammer_spy' WHERE `entry`=101899;
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `DifficultyID`=0 AND `Entry` IN (101875, 101120, 102693, 101899);

-- A ritualist is warded: "Void Touched" takes up damage done to it until it is dispelled. The video, from
-- the beta, shows no ward, but Slaghammer tells the player to dispel one; this is the dispellable aura that
-- the client's data has next to the ritual's own spells.
DELETE FROM `creature_template_addon` WHERE `entry`=101875;
INSERT INTO `creature_template_addon` (`entry`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(101875, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '200921'); -- Twilight Ritualist - Void Touched

-- The ritualists carry staves and Slaghammer a hammer in the video. Which items they are is not known;
-- these are what other creatures of the Twilight's Hammer carry (the staff of the Twilight Darkcaller, the
-- hammer of the Twilight Endbringer).
DELETE FROM `creature_equip_template` WHERE `CreatureID` IN (101875, 101899) AND `ID`=1;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(101875, 1, 65102, 0, 0, 0, 0, 0, 0, 0, 0, 0), -- Twilight Ritualist
(101899, 1, 52057, 0, 0, 0, 0, 0, 0, 0, 0, 0); -- Shadowlord Slaghammer

-- What Slaghammer says in the hall, and the darkness when it wakes
DELETE FROM `creature_text` WHERE (`CreatureID`=101899 AND `GroupID` IN (0, 1, 2)) OR (`CreatureID`=102693 AND `GroupID` IN (0, 1));
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(101899, 0, 0, 'These buggers are suppressin'' Tyr''s wards so the Deacon can do his work below. We need ta take ''em down! Start by dispellin'' the wards on the ritualists!', 12, 0, 100, 0, 0, 63437, 0, 104272, 0, 'Shadowlord Slaghammer - the player has come into the hall'),
(101899, 1, 0, 'A trap! Destroy the creature before it kills us!', 12, 0, 100, 0, 0, 63438, 0, 106115, 0, 'Shadowlord Slaghammer - the last ritual is stopped'),
(101899, 2, 0, 'There we go! The path to the prison should be ope... Well, if it ain''t me old friend! We''d be doin'' the world a favor riddin'' it o'' this baddie!', 12, 0, 100, 0, 0, 63439, 0, 104275, 0, 'Shadowlord Slaghammer - the Amassing Darkness is gone'),
(102693, 0, 0, 'The ritual is interrupted and a dark energy amasses!', 41, 0, 100, 0, 0, 0, 0, 105085, 3, 'Amassing Darkness - the last ritual is stopped'),
(102693, 1, 0, 'Eliminate... all... life...', 12, 0, 100, 0, 0, 0, 0, 105084, 0, 'Amassing Darkness - wakes');

-- Slaghammer answers the same questions as at the door (the menus are in 2026_10_11_00_world.sql)
DELETE FROM `creature_template_gossip` WHERE `CreatureID`=101899;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(101899, 1539001, 0);

-- "Dampen the Light" is what a ritualist does to a ward. It is cast at a creature that the client's data
-- does not name; here the wards are General Purpose Stalkers.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=200923;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 200923, 0, 0, 31, 0, 3, 101294, 0, '', 0, 0, 0, '', 'Dampen the Light - is cast at a General Purpose Stalker');

-- The four ritualists, each north or south of the pedestal whose ward it dampens and turned to it, and a
-- creature that cannot be seen or chosen in each of the four orbs, which is what the spells on a ward are
-- on. The Amassing Darkness is not here: the scenario's script brings it onto the dais when the last ritual
-- is stopped.
DELETE FROM `creature` WHERE `guid` BETWEEN 15390016 AND 15390023;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15390016, 101875, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 1985, 2324, 75.67, 3.295, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Ritualist, north-east of the dais
(15390017, 101875, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 1985, 2349.4, 75.67, 3, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Ritualist, north-west
(15390018, 101875, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 1941.5, 2323.4, 75.42, 6.136, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Ritualist, south-east
(15390019, 101875, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 1941.5, 2348.8, 75.42, 0.158, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Ritualist, south-west
(15390020, 101294, 1539, 0, 0, '12', 0, 0, 0, -1, 11686, 0, 1975.3, 2322.5, 79.6, 0, 7200, 0, 0, 100, 0, NULL, 33555200, NULL, NULL, '', NULL, 0), -- General Purpose Stalker, the ward north-east of the dais
(15390021, 101294, 1539, 0, 0, '12', 0, 0, 0, -1, 11686, 0, 1974.5, 2350.9, 79.6, 0, 7200, 0, 0, 100, 0, NULL, 33555200, NULL, NULL, '', NULL, 0), -- General Purpose Stalker, the ward north-west
(15390022, 101294, 1539, 0, 0, '12', 0, 0, 0, -1, 11686, 0, 1951.6, 2321.9, 79.6, 0, 7200, 0, 0, 100, 0, NULL, 33555200, NULL, NULL, '', NULL, 0), -- General Purpose Stalker, the ward south-east
(15390023, 101294, 1539, 0, 0, '12', 0, 0, 0, -1, 11686, 0, 1950.9, 2350.3, 79.6, 0, 7200, 0, 0, 100, 0, NULL, 33555200, NULL, NULL, '', NULL, 0); -- General Purpose Stalker, the ward south-west

-- A ward is dark until its ritual is stopped. The aura is the spell "Statue", made with "Void Touched" and
-- "Dampen the Light" (its spell visual is the next after theirs). It is a stand-in: on retail a dark purple
-- orb covers the ward, and the spell of that is not found yet.
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 15390020 AND 15390023;
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(15390020, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '200924'),
(15390021, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '200924'),
(15390022, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '200924'),
(15390023, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '200924');
