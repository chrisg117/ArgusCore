-- Ritssyn dies in the Dreadscar prison (The New Blood) and is only raised again in Rebuilding the Council, so his
-- Circle of Wills spawn is seen only until The New Blood is rewarded (mask 11 = none, complete, incomplete).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry`=101456 AND `SourceId`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 101456, 0, 0, 47, 0, 40729, 11, 0, 0, 'Ritssyn: seen until The New Blood is rewarded');
