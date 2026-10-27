-- Frozen in Time's lava patches (4919) had areatrigger template 1468001, which Beam Me Up's server-side trigger around
-- Glazer now uses (IsServerSide, so the patches were never sent to clients). They get their own template.
DELETE FROM `areatrigger_template` WHERE `Id` = 1468101 AND `IsCustom` = 1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1468101, 1, 0, 0, 0, 0);
UPDATE `areatrigger_create_properties` SET `AreaTriggerId` = 1468101 WHERE `Id` = 4919 AND `IsCustom` = 0;
