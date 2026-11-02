-- The leader at the Fel Hammer's door leaves as the rally starts: the scene's own actor jumps down in his place, so the spawn must not stay behind
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (108571, 108572) AND `ElseGroup`=0 AND `ConditionTypeOrReference`=1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 108571, 0, 0, 1, 0, 203269, 0, 0, 1, 'Altruis at the door: not during the rally'),
(32, 3, 108572, 0, 0, 1, 0, 203269, 0, 0, 1, 'Kayn at the door: not during the rally');
