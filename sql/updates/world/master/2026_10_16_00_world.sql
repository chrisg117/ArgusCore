-- A class hall mission's bonus rewards: given with its rewards when the team's chance over 100% wins
-- the bonus roll. Like the rewards, they are content: the client data has none.
ALTER TABLE `class_hall_mission_reward`
  ADD COLUMN `Overmax` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `MissionID`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`MissionID`, `Overmax`, `Idx`);
