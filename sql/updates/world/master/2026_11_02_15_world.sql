-- Fel Machinations (37656) / Saving Stellagosa (37450): Kor'vas follows the player to Faronaar, takes in Fel
-- Machinations and gives Saving Stellagosa there (retail footage), in place of the hand-placed demon hunters.
-- Phase 4356 is per area: at Vanquish Point (7356) until 37656 is taken, at Faronaar (7349) from then on.
DELETE FROM `creature_questender` WHERE (`id`=90474 AND `quest`=37656) OR (`id`=90255 AND `quest`=37656);
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(90255, 37656, 0); -- Kor'vas Bloodthorn: Fel Machinations
DELETE FROM `creature_queststarter` WHERE (`id`=90474 AND `quest`=37450) OR (`id`=90255 AND `quest`=37450);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(90255, 37450, 0); -- Kor'vas Bloodthorn: Saving Stellagosa

DELETE FROM `creature` WHERE `guid` IN (12201001, 12201002, 12201032);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12201032, 90255, 1220, 7334, 7349, '0', 4356, 0, 0, 0, -300.00, 7334.00, 18.26, 0, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Kor'vas Bloodthorn at Faronaar
DELETE FROM `creature_addon` WHERE `guid`=12201032;
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(12201032, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, '222589 42459');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=4356 AND `SourceEntry`=7349;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 4356, 7349, 0, 0, 47, 0, 37656, 74, 0, 0, 0, 0, '', 'Apply Phase 4356 in Faronaar once Quest 37656 is taken');
