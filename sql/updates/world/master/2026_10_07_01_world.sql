-- Scaffold for the Legion intro scenario "The Battle for Broken Shore" (map 1460, Broken Shore Scenario).

-- Attach the client's scenarios to the map: 786 for Alliance, 1189 for Horde.
-- MapDifficulty.db2 gives map 1460 a single difficulty, 12.
DELETE FROM `scenarios` WHERE `map`=1460 AND `difficulty`=12;
INSERT INTO `scenarios` (`map`, `difficulty`, `scenario_A`, `scenario_H`) VALUES
(1460, 12, 786, 1189);

DELETE FROM `instance_template` WHERE `map`=1460;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1460, 0, 'instance_broken_shore_scenario');

-- Captain Angelica's gossip option "I am ready to face the Legion." sends players into the scenario
UPDATE `creature_template` SET `ScriptName`='npc_captain_angelica_broken_shore' WHERE `entry`=108920;
