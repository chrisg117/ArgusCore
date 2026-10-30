-- Down to Azsuna: Khadgar on Krasus' Landing opens the sniffed menu 17085. Its option makes the player cast the quest taxi,
-- which flies client path 5643/5655 to the Illidari camp and credits the objective.
UPDATE `creature_template` SET `npcflag`=`npcflag`|1, `AIName`='SmartAI' WHERE `entry`=86563;
DELETE FROM `creature_template_gossip` WHERE `CreatureID`=86563 AND `MenuID`=17085;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES (86563, 17085, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=17085;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 17085, 0, 0, 0, 47, 0, 41220, 8, 0, '', 0, 0, 0, '', 'Down to Azsuna flight requires the quest incomplete'),
(15, 17085, 0, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Down to Azsuna flight, Alliance option'),
(15, 17085, 1, 0, 0, 47, 0, 41220, 8, 0, '', 0, 0, 0, '', 'Down to Azsuna flight requires the quest incomplete'),
(15, 17085, 1, 0, 0, 6, 0, 67, 0, 0, '', 0, 0, 0, '', 'Down to Azsuna flight, Horde option');

DELETE FROM `smart_scripts` WHERE `entryorguid`=86563 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_param1`, `event_param2`, `action_type`, `action_param1`, `action_param2`, `target_type`, `comment`) VALUES
(86563, 0, 0, 1, 62, 17085, 0, 72, 0, 0, 7, 'Archmage Khadgar - On gossip option 0 selected - Close gossip'),
(86563, 0, 1, 0, 61, 0, 0, 134, 205098, 2, 7, 'Archmage Khadgar - Linked - Invoker casts Down to Azsuna: Unlearned Taxi Alliance'),
(86563, 0, 2, 3, 62, 17085, 1, 72, 0, 0, 7, 'Archmage Khadgar - On gossip option 1 selected - Close gossip'),
(86563, 0, 3, 0, 61, 0, 0, 134, 205203, 2, 7, 'Archmage Khadgar - Linked - Invoker casts Down to Azsuna: Unlearned Taxi Horde');
