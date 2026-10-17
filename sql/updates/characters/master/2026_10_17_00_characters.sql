-- The troops a character's class hall missions have used up, kept for a revive: their kinds, oldest first.
CREATE TABLE IF NOT EXISTS `character_class_hall_expired_troop` (
  `guid` bigint unsigned NOT NULL,
  `idx` tinyint unsigned NOT NULL,
  `followerId` int unsigned NOT NULL,
  `expireTime` bigint NOT NULL,
  PRIMARY KEY (`guid`,`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
