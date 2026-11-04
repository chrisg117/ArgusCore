-- The death knight's campaign scenes: 1076 is what 190789 The Blades of the Fallen Prince plays, 1085 the order speech
-- at The Call of Icecrown's turn-in (191426). The client keeps the packages by name only; 1466 (Looting the Blades)
-- was confirmed on the client, 1472 is unseen until the turn-in is built.
DELETE FROM `scene_template` WHERE `SceneId` IN (1076, 1085);
INSERT INTO `scene_template` (`SceneId`, `Flags`, `ScriptPackageID`, `ScriptName`) VALUES
(1076, 16, 1466, ''),
(1085, 16, 1472, '');
