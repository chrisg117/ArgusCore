-- Nythendra: Infested Ground pools use decal 31, a green plague puddle that
-- matches the retail look (found and checked in game, 2026-10-17). It replaces
-- the blue debug decal 24 from 2026_10_17_04.
UPDATE `areatrigger_create_properties` SET `DecalPropertiesId` = 31 WHERE `Id` = 5887 AND `IsCustom` = 0;
