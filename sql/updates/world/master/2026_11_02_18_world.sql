-- Fel Machinations (37656): the Soul Harvester crystal frees its hostage when the opening cast completes, not on
-- the click that starts it (the client reports that click as a use too).
UPDATE `smart_scripts` SET `event_param1`=1 WHERE `entryorguid` IN (240075, 240121, 240122, 240123) AND `source_type`=1 AND `event_type`=64;
