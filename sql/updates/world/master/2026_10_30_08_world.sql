-- The two teleport pads of Dalaran, in the middle of the city and in Aegwynn's Gallery, stood proud of the floor
-- and sent who came up straight back down.

-- The floor under the city's pad is at 735.88 and under the gallery's at 602.48 (.gps around each). The ring (display
-- 27762) stands on that floor. The light (display 16972, an Auchindoun teleporter) also holds a table on arches, whose
-- top is the blue middle of the pad in the video of the retail game, which shows no arches: half a yard down it lies
-- inside the ring and the arches are under the floor.
UPDATE `gameobject` SET `position_z`=735.88 WHERE `guid`=12200002; -- Teleport Pad, in the middle of the city
UPDATE `gameobject` SET `position_z`=735.38 WHERE `guid`=12200003; -- its light
UPDATE `gameobject` SET `position_z`=602.48 WHERE `guid`=12200004; -- Teleport Pad, in Aegwynn's Gallery
UPDATE `gameobject` SET `position_z`=601.98 WHERE `guid`=12200005; -- its light

-- "Teleport: Dalaran" (192295) arrives 2.95 yards from the middle of the city's pad, a place from the retail game that
-- stays as it is. The area trigger's radius of 2.2 reaches 3.7 yards on a player, because the check adds their combat
-- reach of 1.5, so it took them on arrival. At 0.5 each pad fires 2 yards from its middle, just outside the ring.
UPDATE `areatrigger_create_properties` SET `ShapeData0`=0.5, `ShapeData1`=0.5 WHERE `Id` IN (192293, 192295) AND `IsCustom`=1;
