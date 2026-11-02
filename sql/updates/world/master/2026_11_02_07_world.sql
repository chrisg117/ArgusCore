-- The Fel Hammer's mission table at twice its size in the middle of the hall's floor, as in the user's retail screenshot; the
-- middle is the floor mesh's (x 1528..1595, y 1383..1446). Its origin is its feet, on the floor.
UPDATE `gameobject_template` SET `size`=2 WHERE `entry`=1519003;
UPDATE `gameobject` SET `position_x`=1561.00, `position_y`=1414.50, `position_z`=237.23 WHERE `guid`=15191003;
UPDATE `creature` SET `position_x`=1561.00, `position_y`=1414.50 WHERE `guid`=15191006;
