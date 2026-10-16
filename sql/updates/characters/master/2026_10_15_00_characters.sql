-- The equipment of class hall champions: which trait fills each equipment slot.
CREATE TABLE IF NOT EXISTS `character_class_hall_follower_equipment` (
  `guid` bigint unsigned NOT NULL,
  `dbId` bigint unsigned NOT NULL,
  `slot` tinyint unsigned NOT NULL,
  `abilityId` int unsigned NOT NULL,
  PRIMARY KEY (`dbId`,`slot`),
  KEY `idx_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
