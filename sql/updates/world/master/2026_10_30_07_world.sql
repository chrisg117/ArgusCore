-- Play Teleport Dalaran Scene (227861) requires client area 7881 (AreaGroup 4517), but the server reads the city's spot
-- over Deadwind Pass as Karazhan (2562), so the cast fails silently. Khadgar plays scene 1449 directly instead.
UPDATE `smart_scripts` SET `action_type` = 129, `action_param1` = 1449, `action_param2` = 0, `comment` = 'Archmage Khadgar - On gossip option 1 selected - Play Teleport Dalaran Scene' WHERE `entryorguid` = 113986 AND `source_type` = 0 AND `id` = 1;
