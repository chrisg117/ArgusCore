-- Talents (hall upgrades) of a Legion class hall. talentId is a GarrTalent.db2 id,
-- researchStartTime a unix time, flags the state as sent to the client.
-- No foreign key to character_class_hall: that row is saved by delete and insert,
-- so a cascading link would wipe the talents on every save.
CREATE TABLE IF NOT EXISTS `character_class_hall_talent` (
  `guid` BIGINT UNSIGNED NOT NULL,
  `talentId` INT UNSIGNED NOT NULL,
  `researchStartTime` BIGINT NOT NULL DEFAULT '0',
  `flags` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`talentId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall talents';
