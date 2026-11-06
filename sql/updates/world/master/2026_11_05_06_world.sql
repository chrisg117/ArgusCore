-- DK campaign: The Call of Icecrown, the gargoyle gate's plate rings (Task 3b-2a fix).
-- The client does not draw the custom areatrigger markers (pass 3: no rings). 203570 "Cosmetic - Frosty Circle (Small)"
-- is a self aura (dummy, infinite, SpellVisual 53207) made to sit on a unit, so each plate gets a stock Invisible Stalker
-- that wears it. The plate kill check is in the Gatekeeper AI and does not depend on these.

DELETE FROM `areatrigger` WHERE `SpawnId` IN (14800004, 14800005);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=14800004 AND `IsCustom`=1;
DELETE FROM `areatrigger_template` WHERE `Id`=14800004 AND `IsCustom`=1;

DELETE FROM `creature` WHERE `guid` IN (14800019, 14800020);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14800019, 15214, 1480, 0, 0, '12', 0, 0, 0, 0, 4221.56, 2761.36, 350.96, 0, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Invisible Stalker, south plate ring
(14800020, 15214, 1480, 0, 0, '12', 0, 0, 0, 0, 4221.47, 2777.25, 350.96, 0, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Invisible Stalker, north plate ring
DELETE FROM `creature_addon` WHERE `guid` IN (14800019, 14800020);
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(14800019, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '203570'),
(14800020, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '203570');
