-- Il'gynoth heart phase: Dark Reconstitution and Cursed Blood (target pick and burst).
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_ilgynoth_dark_reconstitution', 'spell_ilgynoth_cursed_blood', 'spell_ilgynoth_cursed_blood_aura');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210781, 'spell_ilgynoth_dark_reconstitution'),
(215128, 'spell_ilgynoth_cursed_blood'),
(215128, 'spell_ilgynoth_cursed_blood_aura');
