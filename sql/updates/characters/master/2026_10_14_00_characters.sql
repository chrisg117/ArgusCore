-- The troop orders (shipments) a character has placed in its class hall and not collected yet.
-- shipmentId only has to be unique for the character: the client never sees another
-- character's orders. duration is kept with the order so that a later change of the data
-- does not move orders that are already paid for.
-- No foreign key to character_class_hall: its row is saved with REPLACE (delete and insert),
-- which would cascade. The core deletes these rows itself.
CREATE TABLE IF NOT EXISTS `character_class_hall_shipment` (
  `guid` BIGINT UNSIGNED NOT NULL,
  `shipmentId` BIGINT UNSIGNED NOT NULL,
  `charShipmentId` INT UNSIGNED NOT NULL,
  `creationTime` BIGINT NOT NULL DEFAULT '0',
  `duration` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`, `shipmentId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall troop orders';
