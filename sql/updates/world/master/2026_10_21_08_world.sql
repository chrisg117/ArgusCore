-- Hidden No More (39495): Fel Lord Caza (96441) had no script, so killing him never gave "Caza slain & power taken"
-- (106014). Like Beliash, his attackers now channel Taking Power (210105) on his corpse and get the credit when it
-- finishes. He also gets his Wowhead abilities, Sweeping Slash and Fel Infusion.
UPDATE `creature_template` SET `ScriptName` = 'npc_fel_lord_caza' WHERE `entry` = 96441;

DELETE FROM `spell_script_names` WHERE `spell_id` IN (210105, 197181) AND `ScriptName` IN ('spell_hidden_no_more_taking_power', 'spell_mardum_caza_fel_infusion');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210105, 'spell_hidden_no_more_taking_power'),
(197181, 'spell_mardum_caza_fel_infusion');

-- The channel's periodic dummy (210107) pulses at his corpse
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 210107;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 210107, 0, 0, 31, 0, 3, 96441, 0, '', 0, 0, 0, '', 'Hidden No More: Taking Power 02 - target Fel Lord Caza');
