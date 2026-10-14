-- The class hall order (CharShipment) a gossip option of type 28 (shipment crafter) offers.
ALTER TABLE `gossip_menu_option_addon` ADD COLUMN `CharShipmentID` INT DEFAULT NULL AFTER `GarrTalentTreeID`;
