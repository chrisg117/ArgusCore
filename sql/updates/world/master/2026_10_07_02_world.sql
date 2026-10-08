DELETE FROM `command` WHERE `name` IN ('scenario', 'scenario event', 'scenario completestep');
INSERT INTO `command` (`name`, `help`) VALUES
('scenario', ''),
('scenario event', 'Syntax: .scenario event #gameEventId [#count]\nTriggers the game event from your character while in a scenario, up to 100 times'),
('scenario completestep', 'Syntax: .scenario completestep\nTriggers every game event the current scenario step is waiting for');
