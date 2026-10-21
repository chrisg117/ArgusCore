-- Elerethe Renferal flight: the Eye of the Storm (152011) and the Shimmering Feather (152012), custom area
-- triggers made by the boss script, and the Dark Storm tick's target filter.
-- 152003-152010 are the Emerald Nightmare hub portals, so the ids here start at 152011.
DELETE FROM `areatrigger_template` WHERE `Id` IN (152011, 152012) AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(152011, 1, 0, 0, 0, 0),
(152012, 1, 0, 0, 0, 0);

-- Spheres: the Eye radius 15 (decal 24, the blue debug decal, picked by eye), the Feather radius 1.5 (no decal: gameobject 1520001 is its visual)
DELETE FROM `areatrigger_create_properties` WHERE `Id` IN (152011, 152012) AND `IsCustom` = 1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `AnimId`, `Speed`, `DecalPropertiesId`, `Shape`, `ShapeData0`, `ShapeData1`, `ScriptName`, `VerifiedBuild`) VALUES
(152011, 1, 152011, 1, 0, -1, 1, 24, 0, 15, 15, 'at_en_eye_of_the_storm', 0),
(152012, 1, 152012, 1, 0, -1, 1, 0, 0, 1.5, 1.5, 'at_en_shimmering_feather', 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_en_dark_storm_damage';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(213936, 'spell_en_dark_storm_damage');

-- Gathering Clouds: 212708 is the 8 s periodic dummy that carries the wind's push
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_en_gathering_clouds_wind';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(212708, 'spell_en_gathering_clouds_wind');

-- The Shimmering Feather visual: a custom generic gameobject (type 5) the feather area trigger summons, display 13727
-- (spells/priest_angelicfeather_state.m2). Flags: GO_FLAG_NOT_SELECTABLE, so it is only a visual.
DELETE FROM `gameobject_template` WHERE `entry` = 1520001;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `VerifiedBuild`) VALUES
(1520001, 5, 13727, 'Shimmering Feather', 1, 0);
DELETE FROM `gameobject_template_addon` WHERE `entry` = 1520001;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`) VALUES
(1520001, 0, 0x10);

-- Venomous Pool (7078, both the spiderling pools and Necrotic Venom pools): radius 7, the size of the ring a player
-- wears under 215449 (kit 69606: spells/target_nature_state_rimonly_projected.m2, a 9.6 yd quad at attach scale 1.5,
-- so a 7.2 yd rim), and of 215459 splash radius (7 yd). Was 3 (2026_10_20_01).
UPDATE `areatrigger_create_properties` SET `ShapeData0` = 7, `ShapeData1` = 7 WHERE `Id` = 7078 AND `IsCustom` = 0;
