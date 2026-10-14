-- How many more followers the character may activate today in its class hall.
-- Set back to 1 with the daily quest reset.
ALTER TABLE `character_class_hall` ADD COLUMN `followerActivationsRemaining` INT UNSIGNED NOT NULL DEFAULT '1' AFTER `siteLevelId`;
