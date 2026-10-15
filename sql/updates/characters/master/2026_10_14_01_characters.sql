-- The missions of a character's class hall: those on offer at its mission table, those its
-- followers are on, and those finished whose reward is not collected yet.
-- dbId is the mission's id for the client and only has to be unique for the character.
-- startTime is 0 while the mission is only offered; successChance is set when it starts.
-- state: 0 offered, 1 in progress, 2 completed.
-- No foreign key to character_class_hall: its row is saved with REPLACE (delete and insert),
-- which would cascade. The core deletes these rows itself.
CREATE TABLE IF NOT EXISTS `character_class_hall_mission` (
  `guid` BIGINT UNSIGNED NOT NULL,
  `missionId` INT UNSIGNED NOT NULL,
  `dbId` BIGINT UNSIGNED NOT NULL,
  `offerTime` BIGINT NOT NULL DEFAULT '0',
  `startTime` BIGINT NOT NULL DEFAULT '0',
  `successChance` INT NOT NULL DEFAULT '0',
  `state` TINYINT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`, `missionId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall missions';

-- The mission a class hall follower is on, 0 while it is free.
DROP PROCEDURE IF EXISTS character_class_hall_follower_mission_2026_10_14_01;

DELIMITER ;;
CREATE PROCEDURE character_class_hall_follower_mission_2026_10_14_01() BEGIN
  IF NOT EXISTS (SELECT * FROM `information_schema`.`columns` WHERE `table_schema`=SCHEMA() AND `table_name`='character_class_hall_follower' AND `column_name`='missionId') THEN
    ALTER TABLE `character_class_hall_follower` ADD COLUMN `missionId` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `status`;
  END IF;
END;;

DELIMITER ;
CALL character_class_hall_follower_mission_2026_10_14_01();

DROP PROCEDURE IF EXISTS character_class_hall_follower_mission_2026_10_14_01;
