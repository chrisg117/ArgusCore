-- Emerald Nightmare hub portals, first in-game test (2026-10-20):
-- the portal objects sat half way through the floor, so they are raised 3 yards; and the
-- return triggers were only 1 yard clear of the arrival points, so arriving fired the
-- return and bounced the player back. They now sit 12 yards behind the arrival point
-- with a radius of 3.
UPDATE `gameobject` SET `position_z` = `position_z` + 3 WHERE `guid` BETWEEN 1520201 AND 1520204;

UPDATE `areatrigger_create_properties` SET `ShapeData0` = 3, `ShapeData1` = 3 WHERE `Id` BETWEEN 152007 AND 152010 AND `IsCustom` = 1;

UPDATE `areatrigger` SET `PosX` = -13159.51, `PosY` = 12093.49 WHERE `SpawnId` = 1520205;
UPDATE `areatrigger` SET `PosX` = 450.52, `PosY` = -12749.73 WHERE `SpawnId` = 1520206;
UPDATE `areatrigger` SET `PosX` = -12329.33, `PosY` = -12750.57 WHERE `SpawnId` = 1520207;
UPDATE `areatrigger` SET `PosX` = 11330.35, `PosY` = 11044.1 WHERE `SpawnId` = 1520208;
