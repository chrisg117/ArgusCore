-- gameobject_addon flags. 0x1 (GAMEOBJECT_ADDON_FLAG_MAP_WIDE): the spawn is kept loaded and sent to every player on
-- its map who can see it (phasing and conditions source 32 still apply) wherever they are, like a transport, and the
-- range-based visibility passes never remove it from their client.
ALTER TABLE `gameobject_addon`
  ADD COLUMN `Flags` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `AIAnimKitID`;
