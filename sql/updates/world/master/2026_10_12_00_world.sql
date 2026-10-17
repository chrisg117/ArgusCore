-- "Blade in Twilight" (scenario 991): corrections after a playthrough of the whole scenario, from the
-- player's notes and a second look at the video of the retail scenario.

-- Shadowlord Slaghammer is a shadow priest in the video: he carries a staff and is in Shadowform. Which
-- staff it is is not known; this is the one that the Twilight's Hammer's casters carry here (the Twilight
-- Darkcaller's). The client has many spells named "Shadowform"; this one (205128) is made like the
-- scenario's other looks. That it is his is a guess.
DELETE FROM `creature_equip_template` WHERE `CreatureID` IN (101430, 101899) AND `ID`=1;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(101430, 1, 65102, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(101899, 1, 65102, 0, 0, 0, 0, 0, 0, 0, 0, 0);

DELETE FROM `creature_template_addon` WHERE `entry` IN (101430, 101899);
INSERT INTO `creature_template_addon` (`entry`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(101430, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '205128'),
(101899, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '205128');

-- The barrier in the tomb's door is Blizzard's "Void Barrier" (248409), which was not placed anywhere. It
-- has the look of the server's own template that stood there, at a larger size: in the video the barrier
-- reaches well beyond the door's opening. The server's own template goes.
DELETE FROM `gameobject_template_addon` WHERE `entry` IN (248409, 1539001);
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`, `mingold`, `maxgold`, `artkit0`, `artkit1`, `artkit2`, `artkit3`, `artkit4`, `WorldEffectID`, `AIAnimKitID`) VALUES
(248409, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0); -- not to be clicked on

UPDATE `gameobject` SET `id`=248409 WHERE `guid`=15390004;
DELETE FROM `gameobject_template` WHERE `entry`=1539001;

-- A ward whose ritual lasts has a dark orb over it in the video, which bursts when the ritual is stopped.
-- That is "Dampened" (202925): a ball of shadow that ends in a burst.
UPDATE `creature_addon` SET `auras`='202925' WHERE `guid` BETWEEN 15390020 AND 15390023;

-- The ball is as high as it is wide around the creature that has it, so that creature is where the middle
-- of the ward's orb is: half the ball's height lower than it stood (tried in game).
UPDATE `creature` SET `position_z`=77.7 WHERE `guid` BETWEEN 15390020 AND 15390023;

-- Zakajz lies in the pool of the prison chamber. At the size his look gives him his body is wider than the
-- pool, which it is not in the video: he is half that size. Where a creature is, is where it stood: his
-- body has fallen forward from there, and its middle lay 15.4 yards in front of that place at the full
-- size (marked in game), so 7.7 at half. He stood that far behind the pool's middle, which is at
-- 1878.5, 2174.5 (measured on the tomb's collision).
UPDATE `creature_template` SET `scale`=0.5 WHERE `entry`=102039;
UPDATE `creature` SET `position_x`=1878.5, `position_y`=2166.8, `position_z`=35.84 WHERE `guid`=15390041;

-- When the lines of the Deacon's end begin, read from the video again: the blade answers him sooner, and
-- speaks to the player sooner
UPDATE `conversation_template` SET `LastLineEndTime`=33500 WHERE `Id`=1126;
UPDATE `conversation_line_template` SET `StartTime`=5000 WHERE `Id`=2742;
UPDATE `conversation_line_template` SET `StartTime`=16000 WHERE `Id`=2743;
UPDATE `conversation_line_template` SET `StartTime`=21500 WHERE `Id`=2744;

-- A circle of blue light is under the blade in the video when it can be taken. No spell was found for it.
-- This is a world effect on the object by which the blade is taken: the blue glow on the ground that flag
-- stands have (WorldEffect 735, "NightElfCTFFlagPlaceGlow"). It is a guess.
DELETE FROM `gameobject_template_addon` WHERE `entry`=247690;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`, `mingold`, `maxgold`, `artkit0`, `artkit1`, `artkit2`, `artkit3`, `artkit4`, `WorldEffectID`, `AIAnimKitID`) VALUES
(247690, 0, 0, 0, 0, 0, 0, 0, 0, 0, 735, 0);

-- The blade's beam on the Deacon is "Cosmetic - Artifact Soul Attack" (201899), which is cast at him.
-- "Chained" stood in for it and needs him no longer.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=201792;
