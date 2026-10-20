-- Elerethe Renferal Roc form: Twisting Shadows marks (210864 picks the players, 210863 is the mark that
-- leaves a tornado behind).
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_en_twisting_shadows', 'spell_en_twisting_shadows_mark');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210864, 'spell_en_twisting_shadows'),
(210863, 'spell_en_twisting_shadows_mark');
