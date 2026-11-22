-- The Death of the Eldest (37853): Toss Crystals, the override bar Senegos gives at his pool (179916 -> 179915)
DELETE FROM `spell_script_names` WHERE `spell_id` = 179915;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(179915, 'spell_death_of_the_eldest_toss_crystals');

-- Toss Crystal (179913): the crystals land in Senegos's pool
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 179913;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 179913, 0, 0, 31, 0, 3, 89975, 0, '', 0, 0, 0, '', 'Toss Crystal - target Senegos');
