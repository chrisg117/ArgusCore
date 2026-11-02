-- Fel Machinations (37656): Kor'vas gives it where she takes From Within (36920), as in retail videos and in
-- the quest text ("My force wiped out the demons' forward base here"). She stays at Vanquish Point until it is
-- accepted; the demon hunter at Faronaar still takes it in.
DELETE FROM `creature_queststarter` WHERE (`id`=90474 AND `quest`=37656) OR (`id`=90255 AND `quest`=37656);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(90255, 37656, 0); -- Kor'vas Bloodthorn: Fel Machinations

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=4356 AND `SourceEntry` IN (7349, 7356);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 4356, 7349, 0, 0, 47, 0, 36920, 64, 0, 1, 0, 0, '', 'Apply Phase 4356 if Quest 36920 is not rewarded'),
(26, 4356, 7349, 0, 1, 47, 0, 37656, 74, 0, 1, 0, 0, '', 'Apply Phase 4356 if Quest 37656 is not taken or rewarded'),
(26, 4356, 7356, 0, 0, 47, 0, 36920, 64, 0, 1, 0, 0, '', 'Apply Phase 4356 if Quest 36920 is not rewarded'),
(26, 4356, 7356, 0, 1, 47, 0, 37656, 74, 0, 1, 0, 0, '', 'Apply Phase 4356 if Quest 37656 is not taken or rewarded');
