-- DH campaign, The Hunt: stage 4 of scenario 900, "Unleashed Havoc". From the client: the fight's lines and Kil'jaeden's recall,
-- the scene ids of two scenario spells and their packages. Ours: which line goes at which health, and the scene flags, the most
-- common value in this table. Scene 1005, the camp explosion, has no row on purpose: its aura never ends and it holds the
-- camera until the player cancels it. The reveal scene says line 97080 itself.
UPDATE `creature_template` SET `ScriptName`='boss_varedis_felsoul' WHERE `entry`=94836;

DELETE FROM `scene_template` WHERE `SceneId` IN (1054, 1063);
INSERT INTO `scene_template` (`SceneId`, `Flags`, `ScriptPackageID`, `ScriptName`) VALUES
(1054, 16, 1452, ''),
(1063, 16, 1462, '');

DELETE FROM `creature_text` WHERE `CreatureID`=94836;
DELETE FROM `creature_text` WHERE `CreatureID`=25608 AND `GroupID`=50;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(94836, 1, 0, 'Do you crave death, elf? Do you dare face my lord''s wrath alone?', 14, 0, 100, 0, 0, 0, 0, 97374, 0, 'Varedis Felsoul: aggro'),
(94836, 2, 0, 'I am through with this charade! Your soul will feed the Twinblades.', 14, 0, 100, 0, 0, 0, 0, 97376, 0, 'Varedis Felsoul: 75%'),
(94836, 3, 0, 'Join me! Unleash your true power. Do not let the weak hold you back!', 14, 0, 100, 0, 0, 0, 0, 97083, 0, 'Varedis Felsoul: 50%'),
(94836, 4, 0, 'Behold the might of the Deceiver! Behold the glory of the demon within!', 14, 0, 100, 0, 0, 0, 0, 97084, 0, 'Varedis Felsoul: 25%'),
(94836, 5, 0, 'I am a GOD! KNEEL BEFORE ME!', 14, 0, 100, 0, 0, 0, 0, 97085, 0, 'Varedis Felsoul: 10%'),
(25608, 50, 0, 'It is not yet your time, Varedis.', 14, 0, 100, 0, 0, 0, 0, 104225, 0, 'Kil''jaeden: calls Varedis back');
