-- "Blade in Twilight" (scenario 991): its ninth stage, "The Blade of the Black Empire": the player takes
-- the blade, Xal'atath, which floats in the prison chamber since the Deacon's death.
--
-- What is taken is Blizzard's object "Xal'atath" (247690), a box that cannot be seen around the floating
-- blade and opens like a chest, as a video of the retail scenario shows. The scenario's script puts it
-- there. It had nothing in it.
DELETE FROM `gameobject_loot_template` WHERE `Entry`=64250;
INSERT INTO `gameobject_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(64250, 0, 128827, 100, 0, 1, 0, 1, 1, 'Xal''atath - Xal''atath, Blade of the Black Empire');

-- The cutscene of a player holding up the blade: scene 1187, which the spell "Blade of the Black Empire"
-- (201780) plays. A player can skip it, as on retail.
DELETE FROM `scene_template` WHERE `SceneId`=1187;
INSERT INTO `scene_template` (`SceneId`, `Flags`, `ScriptPackageID`, `ScriptName`) VALUES
(1187, 58, 1553, '');
