-- Azsuna (zone 7334) had no graveyard_zone rows, so a death anywhere in the zone sent the ghost to the
-- faction default (Sentinel Hill in Westfall). These are the client's own WorldSafeLocs for the zone
-- ("7.0 Azsuna - (NN) ..."). Left out: 5652 Timeworn Strand Underwater, 5675 Rhut'van Divide (z 498) and
-- 5856 Temple of a Thousand Lights (a second "(21)"), which look tied to special areas or phases.
DELETE FROM `graveyard_zone` WHERE `GhostZone`=7334;
INSERT INTO `graveyard_zone` (`ID`, `GhostZone`, `Comment`) VALUES
(4949, 7334, '7.0 Azsuna - (01) Azurewing Repose'),
(4950, 7334, '7.0 Azsuna - (02) Crumbled Palace'),
(4951, 7334, '7.0 Azsuna - (03) Ruins of Nar''thalas'),
(4952, 7334, '7.0 Azsuna - (04) Shipwreck Arena'),
(4953, 7334, '7.0 Azsuna - (05) Oceanus Cove'),
(4954, 7334, '7.0 Azsuna - (06) Illidari Stand'),
(4983, 7334, '7.0 Azsuna - (07) Rhut''van Passage'),
(4984, 7334, '7.0 Azsuna - (08) The Lost Orchard'),
(4985, 7334, '7.0 Azsuna - (09) Vanquish Point'),
(5010, 7334, '7.0 Azsuna - (10) Mak''rana'),
(5011, 7334, '7.0 Azsuna - (11) El''dranil Lagoon'),
(5012, 7334, '7.0 Azsuna - (12) Hatecoil Warcamp'),
(5013, 7334, '7.0 Azsuna - (13) Farondale'),
(5014, 7334, '7.0 Azsuna - (14) Llothien Woods'),
(5015, 7334, '7.0 Azsuna - (15) Timeworn Strand North'),
(5613, 7334, '7.0 Azsuna - (16) Isle of the Watchers Central'),
(5610, 7334, '7.0 Azsuna - (17) Isle of the Watchers East'),
(5612, 7334, '7.0 Azsuna - (18) Isle of the Watchers North'),
(5614, 7334, '7.0 Azsuna - (19) Timeworn Strand South'),
(5621, 7334, '7.0 Azsuna - (20) Illidari Perch'),
(5745, 7334, '7.0 Azsuna - (23) El''dranil Peak'),
(5780, 7334, '7.0 Azsuna - (24) Seabreak Isle'),
(5782, 7334, '7.0 Azsuna - (25) Eye of Azshara E'),
(5781, 7334, '7.0 Azsuna - (26) Eye of Azshara W');
