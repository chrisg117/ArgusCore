-- The King Rises: the raise is an action button, as Nazgrim's. The client holds the button spell (215173 Raise Horseman,
-- which casts the 10 s 215174) but no override bar for it; the only Raise Horseman bars are 719 (Nazgrim, 213614) and
-- 755, an unused second Nazgrim variant (220560, granted by 220847). Bar 755 is pointed at 215173, and pushed to the
-- client through hotfix_data (OverrideSpellData, table hash 0xCA75DF1C), so the button shows Thoras's raise. The other
-- columns keep the client file's values.
DELETE FROM `override_spell_data` WHERE `ID`=755;
INSERT INTO `override_spell_data` (`ID`, `Spells1`, `Spells2`, `Spells3`, `Spells4`, `Spells5`, `Spells6`, `Spells7`, `Spells8`, `Spells9`, `Spells10`, `PlayerActionBarFileDataID`, `Flags`, `VerifiedBuild`) VALUES
(755, 215173, 0, 0, 0, 0, 0, 0, 0, 0, 0, 629199, 12, 0);

DELETE FROM `hotfix_data` WHERE `Id`=12207001;
INSERT INTO `hotfix_data` (`Id`, `TableHash`, `RecordId`, `Deleted`, `VerifiedBuild`) VALUES
(12207001, 3396722460, 755, 0, 0); -- OverrideSpellData 755
