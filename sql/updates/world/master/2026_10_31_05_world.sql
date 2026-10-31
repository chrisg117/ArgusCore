-- A Cracked Ley Crystal restocked one second after looting, so one crystal gave all four. Now each is used up and returns later.
UPDATE `gameobject_template` SET `Data2`=0, `Data3`=1 WHERE `entry`=240267;
