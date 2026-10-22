-- Before We're Overrun (38766) had no quest giver, so it could never be taken. Orders for Your Captains (38813)
-- needs both quests of exclusive group -38765 (this one and Enter the Illidari: Shivarra), so the chain stopped there.
-- Jace Darkweaver (93759) gives it on retail (Wowhead), next to Shivarra.
DELETE FROM `creature_queststarter` WHERE `id` = 93759 AND `quest` = 38766;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(93759, 38766, 0);
