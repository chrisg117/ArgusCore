-- The Fel Hammer's mission table is 7af_demonhunter_missiontable01.m2 (display 39588), which replaces the table and the flat map.
-- Its model reaches 1.27 yd below its origin, so the origin stands that far over the floor.
UPDATE `gameobject_template` SET `displayId`=39588, `size`=1 WHERE `entry`=1519003;
UPDATE `gameobject` SET `position_z`=238.50 WHERE `guid`=15191003;
DELETE FROM `gameobject` WHERE `guid`=15191004;
DELETE FROM `gameobject_template_addon` WHERE `entry`=1519004;
DELETE FROM `gameobject_template` WHERE `entry`=1519004;
