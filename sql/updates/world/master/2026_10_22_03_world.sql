-- Phase 5077 (the Cryptic Hollow NPCs for Hidden No More) was set on the whole of Mardum (zone 7705). It is not a
-- cosmetic phase and it stays on once Hidden No More is accepted, so from then on the player lost sight of every
-- unphased spawn in the zone (the Stop the Bombardment banners and devastators, Kor'vas, Inferno Peak). All of its
-- spawns stand in Cryptic Hollow (7754), next to phase 5084, so the phase now applies there only.
UPDATE `phase_area` SET `AreaId` = 7754 WHERE `AreaId` = 7705 AND `PhaseId` = 5077;
