-- Nythendra: Infested Ground pools show the blue debug decal (24) for now.
-- The client data gives spell 203044 a visual, but it does not render on a
-- spell-created area trigger here; finding the retail look is a follow-up.
UPDATE `areatrigger_create_properties` SET `DecalPropertiesId` = 24 WHERE `Id` = 5887 AND `IsCustom` = 0;
