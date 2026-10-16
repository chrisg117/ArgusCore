-- Whether a successful class hall mission won its bonus roll; given when its chest is opened.
ALTER TABLE `character_class_hall_mission` ADD COLUMN `bonusWon` tinyint unsigned NOT NULL DEFAULT '0' AFTER `missionDuration`;
