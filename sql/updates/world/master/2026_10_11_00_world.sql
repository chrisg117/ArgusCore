-- "Blade in Twilight" (scenario 991): its third and fourth stages, "The Desecrated Tomb" and "The Tomb of
-- Tyr", at the door of the tomb under the lake.
--
-- The tomb had nothing in it. Where its two guards and Shadowlord Slaghammer stand was marked in game against
-- a video of the retail scenario; the door itself was measured on the tomb's collision. The lines are
-- Blizzard's (broadcast_text 103843 to 104112).

-- What the Twilight Shadowcaster at the door and Slaghammer say
DELETE FROM `creature_text` WHERE (`CreatureID`=101122 AND `GroupID` IN (0, 1)) OR (`CreatureID`=101430 AND `GroupID` IN (0, 1, 2));
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(101122, 0, 0, 'Master Slaghammer! We have an intruder!', 14, 0, 100, 0, 0, 63432, 0, 103843, 0, 'Twilight Shadowcaster - a player has come into the tomb'),
(101122, 1, 0, 'What?! You traitor!', 14, 0, 100, 0, 0, 63434, 0, 103845, 0, 'Twilight Shadowcaster - Slaghammer strikes'),
(101430, 0, 0, 'Excellent! I was expecting ''im, but I don''t think you were...', 14, 0, 100, 0, 0, 63433, 0, 103844, 0, 'Shadowlord Slaghammer - turns on the guards of the door'),
(101430, 1, 0, 'Moira said ye''d be comin''. I''m Slaghammer, payin'' me family''s debt by hangin'' oot wi'' these buggers. Lemme take down this barrier.', 12, 0, 100, 0, 0, 63435, 0, 104103, 0, 'Shadowlord Slaghammer - the guards have fallen'),
(101430, 2, 0, 'Th'' Shadow Deacon is already below startin'' his ritual. Ta get there, we''ll need ta do a bit o'' work. If ye have any questions lemme know, but we best hurry!', 12, 0, 100, 0, 0, 63436, 0, 104104, 0, 'Shadowlord Slaghammer - takes the barrier down');

-- Slaghammer is a friend of the player who fights the Twilight's Hammer, has the script that keeps him alive
-- and in his place, and is of the level of the player who looks at him. His template had the placeholder
-- level 1 and a faction that fights no one.
UPDATE `creature_template` SET `faction`=495, `ScriptName`='npc_shadowlord_slaghammer' WHERE `entry`=101430;
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `DifficultyID`=0 AND `Entry`=101430;

-- He carries a hammer in the video. Which item it is is not known; this is the two-handed hammer that
-- other creatures of the Twilight's Hammer carry (the Twilight Endbringer's).
DELETE FROM `creature_equip_template` WHERE `CreatureID`=101430 AND `ID`=1;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(101430, 1, 52057, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- What he answers when asked, once the barrier is down. The texts are Blizzard's; the ids of the menus and of
-- their `npc_text` are not known and are the server's own.
DELETE FROM `creature_template_gossip` WHERE `CreatureID`=101430;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(101430, 1539001, 0);

DELETE FROM `npc_text` WHERE `ID` BETWEEN 1539001 AND 1539004;
INSERT INTO `npc_text` (`ID`, `Probability0`, `Probability1`, `Probability2`, `Probability3`, `Probability4`, `Probability5`, `Probability6`, `Probability7`, `BroadcastTextID0`, `BroadcastTextID1`, `BroadcastTextID2`, `BroadcastTextID3`, `BroadcastTextID4`, `BroadcastTextID5`, `BroadcastTextID6`, `BroadcastTextID7`) VALUES
(1539001, 1, 0, 0, 0, 0, 0, 0, 0, 104106, 0, 0, 0, 0, 0, 0, 0), -- We dinna hae much time, but ye deserve some answers...
(1539002, 1, 0, 0, 0, 0, 0, 0, 0, 104108, 0, 0, 0, 0, 0, 0, 0), -- Can ye believe they found the Tomb of Tyr himself?!...
(1539003, 1, 0, 0, 0, 0, 0, 0, 0, 104109, 0, 0, 0, 0, 0, 0, 0), -- The Hammer has been in disarray...
(1539004, 1, 0, 0, 0, 0, 0, 0, 0, 104110, 0, 0, 0, 0, 0, 0, 0); -- That blade... I knew Moira would want ta know...

DELETE FROM `gossip_menu` WHERE `MenuID` BETWEEN 1539001 AND 1539004;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(1539001, 1539001, 0),
(1539002, 1539002, 0),
(1539003, 1539003, 0),
(1539004, 1539004, 0);

DELETE FROM `gossip_menu_option` WHERE `MenuID` BETWEEN 1539001 AND 1539004;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(1539001, 0, 0, 'This was sort of sudden, where are we? What''s going on?', 104107, 1539002, 0, 0, 0, NULL, 0, 0),
(1539001, 1, 0, 'The Shadow Deacon? Who is this new leader?', 104111, 1539003, 0, 0, 0, NULL, 0, 0),
(1539001, 2, 0, 'You mentioned a magical blade, can you tell me more?', 104112, 1539004, 0, 0, 0, NULL, 0, 0);

-- The guards of the door, and Slaghammer off to their side, from where he falls on them
DELETE FROM `creature` WHERE `guid` BETWEEN 15390013 AND 15390015;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15390013, 101122, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2023.06, 2332.39, 75.21, 0, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Shadowcaster, left of the door
(15390014, 101127, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2022.97, 2343.16, 75.21, 0, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Twilight Bladetwister, right of the door
(15390015, 101430, 1539, 0, 0, '12', 0, 0, 0, -1, 0, 1, 2022.43, 2308.8, 75.89, 1.53, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Shadowlord Slaghammer

-- The barrier in the door. What it is on retail is not known. This is the wall of shadow that Shadowmoon
-- Burial Grounds has ("Wall of Darkness", 231727), made a size that fills the door's opening of 7 by 13.5
-- yards: the server's own template, since a gameobject's size is its template's.
DELETE FROM `gameobject_template` WHERE `entry`=1539001;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `Data24`, `Data25`, `Data26`, `Data27`, `Data28`, `Data29`, `Data30`, `Data31`, `Data32`, `Data33`, `RequiredLevel`, `AIName`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(1539001, 0, 24101, 'Twilight Barrier', '', '', '', 1.25, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', NULL, 0);

DELETE FROM `gameobject_template_addon` WHERE `entry`=1539001;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`, `mingold`, `maxgold`, `artkit0`, `artkit1`, `artkit2`, `artkit3`, `artkit4`, `WorldEffectID`, `AIAnimKitID`) VALUES
(1539001, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0); -- not to be clicked on

DELETE FROM `gameobject` WHERE `guid`=15390004;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(15390004, 1539001, 1539, 0, 0, '12', 0, 0, 0, -1, 2011.5, 2337.7, 75.2, 0, 0, 0, 0, 1, 7200, 255, 1, '', NULL, 0); -- Twilight Barrier
