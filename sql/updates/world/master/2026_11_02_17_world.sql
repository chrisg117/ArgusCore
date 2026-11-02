-- Fel Machinations (37656): the crystal's own spell summons the freed demon hunter but its dummy hit never reaches
-- the hostage, so the crystal credits the player and clears the hostage itself when used. The crystal and the
-- hostage come back after a minute; the freed demon hunter leaves after a few seconds.
UPDATE `creature_template` SET `AIName`='' WHERE `entry`=90487;
DELETE FROM `smart_scripts` WHERE `entryorguid`=90487 AND `source_type`=0;

UPDATE `gameobject_template` SET `AIName`='SmartGameObjectAI' WHERE `entry` IN (240075, 240121, 240122, 240123);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (240075, 240121, 240122, 240123) AND `source_type`=1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `action_type`, `action_param1`, `target_type`, `target_param1`, `target_param2`, `comment`) VALUES
(240075, 1, 0, 0, 64, 33, 90487, 7, 0, 0, 'Soul Harvester - On use - Credit Demon Hunters freed'),
(240075, 1, 1, 0, 64, 41, 1000, 19, 90487, 6, 'Soul Harvester - On use - Despawn the hostage after its spell'),
(240075, 1, 2, 0, 64, 41, 2000, 1, 0, 0, 'Soul Harvester - On use - Despawn after its spell'),
(240121, 1, 0, 0, 64, 33, 90487, 7, 0, 0, 'Soul Harvester - On use - Credit Demon Hunters freed'),
(240121, 1, 1, 0, 64, 41, 1000, 19, 90487, 6, 'Soul Harvester - On use - Despawn the hostage after its spell'),
(240121, 1, 2, 0, 64, 41, 2000, 1, 0, 0, 'Soul Harvester - On use - Despawn after its spell'),
(240122, 1, 0, 0, 64, 33, 90487, 7, 0, 0, 'Soul Harvester - On use - Credit Demon Hunters freed'),
(240122, 1, 1, 0, 64, 41, 1000, 19, 90487, 6, 'Soul Harvester - On use - Despawn the hostage after its spell'),
(240122, 1, 2, 0, 64, 41, 2000, 1, 0, 0, 'Soul Harvester - On use - Despawn after its spell'),
(240123, 1, 0, 0, 64, 33, 90487, 7, 0, 0, 'Soul Harvester - On use - Credit Demon Hunters freed'),
(240123, 1, 1, 0, 64, 41, 1000, 19, 90487, 6, 'Soul Harvester - On use - Despawn the hostage after its spell'),
(240123, 1, 2, 0, 64, 41, 2000, 1, 0, 0, 'Soul Harvester - On use - Despawn after its spell');

-- only a player on the quest frees anyone
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry` IN (240075, 240121, 240122, 240123) AND `SourceId`=1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, 240075, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 2, 240075, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 3, 240075, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 1, 240121, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 2, 240121, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 3, 240121, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 1, 240122, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 2, 240122, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 3, 240122, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 1, 240123, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 2, 240123, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken'),
(22, 3, 240123, 1, 0, 9, 0, 37656, 0, 0, 0, 0, 0, '', 'Soul Harvester frees a hostage only with Fel Machinations taken');

-- the freed demon hunter leaves after a few seconds
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (90532, 90534, 90535, 90536);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (90532, 90534, 90535, 90536) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `action_type`, `action_param1`, `target_type`, `comment`) VALUES
(90532, 0, 0, 0, 54, 41, 8000, 1, 'Demon Hunter - Just summoned - Despawn after 8 s'),
(90534, 0, 0, 0, 54, 41, 8000, 1, 'Demon Hunter - Just summoned - Despawn after 8 s'),
(90535, 0, 0, 0, 54, 41, 8000, 1, 'Demon Hunter - Just summoned - Despawn after 8 s'),
(90536, 0, 0, 0, 54, 41, 8000, 1, 'Demon Hunter - Just summoned - Despawn after 8 s');
