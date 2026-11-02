-- Kor'vas Bloodthorn at Faronaar (spawn 12201032, phase 4356) stands in area 7354, where phase 4356 was never applied,
-- so Fel Machinations (37656) had nobody to turn in to. The phase applies there under the same condition as in 7349.
DELETE FROM `phase_area` WHERE `AreaId`=7354 AND `PhaseId`=4356;
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(7354, 4356, 'Cosmetic - Azsuna - See Kor''vas Bloodthorn in Faronaar');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=4356 AND `SourceEntry`=7354;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 4356, 7354, 0, 0, 47, 0, 37656, 74, 0, 0, 0, 0, '', 'Apply Phase 4356 in Faronaar once Quest 37656 is taken');
