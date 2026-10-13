-- Followers (champions and troops) of a Legion class hall. followerId is a GarrFollower.db2 id,
-- dbId comes from the generator the Warlords garrison followers use, status holds the flags
-- sent to the client. A follower's abilities are not saved; they follow from its quality.
-- No unique key on (guid, followerId): a character can have several troops of one kind.
-- No foreign key to character_class_hall: that row is saved by delete and insert,
-- so a cascading link would wipe the followers on every save.
CREATE TABLE IF NOT EXISTS `character_class_hall_follower` (
  `dbId` BIGINT UNSIGNED NOT NULL,
  `guid` BIGINT UNSIGNED NOT NULL,
  `followerId` INT UNSIGNED NOT NULL,
  `quality` INT UNSIGNED NOT NULL DEFAULT '0',
  `level` INT UNSIGNED NOT NULL DEFAULT '0',
  `itemLevelWeapon` INT UNSIGNED NOT NULL DEFAULT '0',
  `itemLevelArmor` INT UNSIGNED NOT NULL DEFAULT '0',
  `xp` INT UNSIGNED NOT NULL DEFAULT '0',
  `durability` INT UNSIGNED NOT NULL DEFAULT '0',
  `status` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`),
  KEY `idx_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall followers';
