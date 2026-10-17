-- The temporary abilities of class hall champions: each lasts until the champion's next mission is completed.
CREATE TABLE IF NOT EXISTS `character_class_hall_follower_temporary_ability` (
  `guid` bigint unsigned NOT NULL,
  `dbId` bigint unsigned NOT NULL,
  `abilityId` int unsigned NOT NULL,
  PRIMARY KEY (`dbId`,`abilityId`),
  KEY `idx_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
