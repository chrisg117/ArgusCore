-- Fel Machinations (37656): each hostage is freed by clicking the Soul Harvester crystal that hangs over it
-- (gameobjects 240075, 240121-240123), not by killing a Soul Harvester creature. Each crystal's spell summons the
-- freed demon hunter (90532/90534/90535/90536) and hits the kneeling hostage (90487), which gives the credit.

-- the Soul Harvester creatures were a stand-in
DELETE FROM `creature` WHERE `guid` BETWEEN 12201006 AND 12201012 AND `id`=94745;
DELETE FROM `smart_scripts` WHERE `entryorguid`=94745 AND `source_type`=0;
UPDATE `creature_template` SET `AIName`='' WHERE `entry`=94745;

-- each hostage wears the model of the demon hunter its crystal frees
UPDATE `creature` SET `modelid`=61909 WHERE `guid` IN (12201013, 12201017); -- blood elf female (240075)
UPDATE `creature` SET `modelid`=61908 WHERE `guid` IN (12201014, 12201018); -- night elf female (240121)
UPDATE `creature` SET `modelid`=61912 WHERE `guid` IN (12201015, 12201019); -- blood elf male (240122)
UPDATE `creature` SET `modelid`=60087 WHERE `guid`=12201016;                -- night elf male (240123)

-- the crystals work only while the quest is in the log
UPDATE `gameobject_template` SET `Data1`=37656 WHERE `entry` IN (240075, 240121, 240122, 240123);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 12201111 AND 12201117;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(12201111, 240075, 1220, 7334, 7349, '0', 0, 0, -247.00, 7290.00, 12.50, 0, 0, 0, 0, 1, 60, 255, 1, 0),
(12201112, 240121, 1220, 7334, 7349, '0', 0, 0, -217.00, 7300.00, 14.60, 0, 0, 0, 0, 1, 60, 255, 1, 0),
(12201113, 240122, 1220, 7334, 7349, '0', 0, 0, -257.00, 7340.00, 19.30, 0, 0, 0, 0, 1, 60, 255, 1, 0),
(12201114, 240123, 1220, 7334, 7349, '0', 0, 0, -297.00, 7380.00, 33.10, 0, 0, 0, 0, 1, 60, 255, 1, 0),
(12201115, 240075, 1220, 7334, 7349, '0', 0, 0, -327.00, 7400.00, 35.20, 0, 0, 0, 0, 1, 60, 255, 1, 0),
(12201116, 240121, 1220, 7334, 7349, '0', 0, 0, -337.00, 7340.00, 35.20, 0, 0, 0, 0, 1, 60, 255, 1, 0),
(12201117, 240122, 1220, 7334, 7349, '0', 0, 0, -287.00, 7300.00, 17.50, 0, 0, 0, 0, 1, 60, 255, 1, 0);

-- the spells' summon point and dummy hit are the hostage under the crystal
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (178934, 180174, 180175, 180176);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 6, 178934, 0, 0, 31, 0, 3, 90487, 0, 0, 0, 0, '', 'Fel Machinations: Summon Demon Hunter - Blood Elf Female targets the hostage'),
(13, 6, 180174, 0, 0, 31, 0, 3, 90487, 0, 0, 0, 0, '', 'Fel Machinations: Summon Demon Hunter - Night Elf Female targets the hostage'),
(13, 6, 180175, 0, 0, 31, 0, 3, 90487, 0, 0, 0, 0, '', 'Fel Machinations: Summon Demon Hunter - Blood Elf Male targets the hostage'),
(13, 6, 180176, 0, 0, 31, 0, 3, 90487, 0, 0, 0, 0, '', 'Fel Machinations: Summon Demon Hunter - Night Elf Male targets the hostage');

-- the hostage credits the player who freed it and leaves; the freed demon hunter stands in its place
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=90487;
DELETE FROM `smart_scripts` WHERE `entryorguid`=90487 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_param1`, `action_type`, `action_param1`, `target_type`, `comment`) VALUES
(90487, 0, 0, 0, 8, 178934, 33, 90487, 7, 'Demon Hunter - On spellhit Summon Demon Hunter - Credit Demon Hunters freed'),
(90487, 0, 1, 0, 8, 180174, 33, 90487, 7, 'Demon Hunter - On spellhit Summon Demon Hunter - Credit Demon Hunters freed'),
(90487, 0, 2, 0, 8, 180175, 33, 90487, 7, 'Demon Hunter - On spellhit Summon Demon Hunter - Credit Demon Hunters freed'),
(90487, 0, 3, 0, 8, 180176, 33, 90487, 7, 'Demon Hunter - On spellhit Summon Demon Hunter - Credit Demon Hunters freed'),
(90487, 0, 4, 0, 8, 178934, 41, 0, 1, 'Demon Hunter - On spellhit Summon Demon Hunter - Despawn'),
(90487, 0, 5, 0, 8, 180174, 41, 0, 1, 'Demon Hunter - On spellhit Summon Demon Hunter - Despawn'),
(90487, 0, 6, 0, 8, 180175, 41, 0, 1, 'Demon Hunter - On spellhit Summon Demon Hunter - Despawn'),
(90487, 0, 7, 0, 8, 180176, 41, 0, 1, 'Demon Hunter - On spellhit Summon Demon Hunter - Despawn');
