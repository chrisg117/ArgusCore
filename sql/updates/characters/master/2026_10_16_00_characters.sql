-- The duration of a started class hall mission, as its team changed it; 0 is the mission's own.
ALTER TABLE `character_class_hall_mission` ADD COLUMN `missionDuration` int unsigned NOT NULL DEFAULT '0' AFTER `successChance`;
