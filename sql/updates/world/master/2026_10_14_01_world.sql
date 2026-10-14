-- What a class hall mission gives when it succeeds and its chest is opened: items, currencies,
-- and money as CurrencyID 0 with the amount in copper. Every row of a mission is given.
-- The client data has no mission rewards: the rows are content.
CREATE TABLE IF NOT EXISTS `class_hall_mission_reward` (
  `MissionID` INT UNSIGNED NOT NULL,
  `Idx` TINYINT UNSIGNED NOT NULL,
  `ItemID` INT UNSIGNED NOT NULL DEFAULT '0',
  `ItemQuantity` INT UNSIGNED NOT NULL DEFAULT '0',
  `CurrencyID` INT UNSIGNED NOT NULL DEFAULT '0',
  `CurrencyQuantity` INT UNSIGNED NOT NULL DEFAULT '0',
  `VerifiedBuild` INT NOT NULL DEFAULT '0',
  PRIMARY KEY (`MissionID`, `Idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall mission rewards';
