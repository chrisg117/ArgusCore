-- Il'gynoth forcefield: not selectable (it cannot be clicked shut with a player inside), never despawns (48 = NOT_SELECTABLE | NODESPAWN)
DELETE FROM `gameobject_template_addon` WHERE `entry` = 251555;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`, `mingold`, `maxgold`, `artkit0`, `artkit1`, `artkit2`, `artkit3`, `artkit4`, `WorldEffectID`, `AIAnimKitID`) VALUES
(251555, 114, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0);
