-- Dalaran over Deadwind Pass (246854, spawn 15190002) is map-wide: a player on In the Blink of an Eye (44663) already
-- has the city at their client in Stormwind, so they land on it after the Portal to Dalaran instead of falling through
-- while it is sent from 533 yd.
INSERT INTO `gameobject_addon` (`guid`, `Flags`) VALUES (15190002, 1)
  ON DUPLICATE KEY UPDATE `Flags` = 1;
