-- Saving Stellagosa: its key objective is sequenced after Find Stellagosa, and a key looted earlier never counts, with no
-- second key to get. So Lykill drops it only once she has been found.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=1 AND `SourceGroup`=86535 AND `SourceEntry`=120359;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(1, 86535, 120359, 0, 0, 48, 0, 277290, 0, 1, 0, 'Lykill drops his key once Find Stellagosa is done');
