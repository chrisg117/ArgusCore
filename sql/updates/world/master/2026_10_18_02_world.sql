-- The Emerald Nightmare: graveyard at the raid's entrance (WorldSafeLocs 5533, map 1520, (1810.1, 1424.2, 355.2))
DELETE FROM `graveyard_zone` WHERE `ID` = 5533 AND `GhostZone` = 8026;
INSERT INTO `graveyard_zone` (`ID`, `GhostZone`, `Comment`) VALUES
(5533, 8026, 'The Emerald Nightmare - The Emerald Nightmare');
