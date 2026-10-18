-- Nythendra root wall: not selectable (no tooltip, no clicking it up), never despawns (48 = NOT_SELECTABLE | NODESPAWN)
DELETE FROM `gameobject_template_addon` WHERE `entry` = 254985;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`, `mingold`, `maxgold`, `artkit0`, `artkit1`, `artkit2`, `artkit3`, `artkit4`, `WorldEffectID`, `AIAnimKitID`) VALUES
(254985, 114, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0);
