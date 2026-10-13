-- Legion class hall: one row per character that has one. siteLevelId is a GarrSiteLevel.db2 id
-- (560 = site 161, level 1). Kept apart from the character_garrison tables, which hold the
-- Warlords garrison and its plots and buildings; a class hall has neither.
CREATE TABLE IF NOT EXISTS `character_class_hall` (
  `guid` BIGINT UNSIGNED NOT NULL,
  `siteLevelId` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall, one row per character';
