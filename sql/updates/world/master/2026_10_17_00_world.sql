-- The Emerald Nightmare (map 1520): attach the instance script, so the raid
-- gets an InstanceScript and `.go instance emerald_nightmare` can find it.
DELETE FROM `instance_template` WHERE `map` = 1520;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1520, 0, 'instance_emerald_nightmare');
