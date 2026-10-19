-- Il'gynoth: Nightmare Corruption pools (area trigger 6514, from Spew Corruption and the
-- Nightmare Horror) use decal 20, the Val'sharah nightmare roots: blood-red veins on black.
-- Retail draws the pool with spells/7fx_nightmare_nova_impact.m2, which no decal uses; 20 is
-- the closest match (checked in game, 2026-10-19). Replaces decal 31.
UPDATE `areatrigger_create_properties` SET `DecalPropertiesId` = 20 WHERE `Id` = 6514 AND `IsCustom` = 0;
