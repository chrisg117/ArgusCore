-- One Khadgar on Krasus' Landing. Retail phases 86563 and 91172 apart, but a spawn in a plain DB phase stays visible on
-- this core, so 86563 takes over The Tranquil Forest and its follow-up.
DELETE FROM `creature` WHERE `guid`=12200005;
DELETE FROM `creature_questender` WHERE `id`=91172 AND `quest`=39731;
DELETE FROM `creature_queststarter` WHERE `id`=91172 AND `quest`=39861;
DELETE FROM `creature_questender` WHERE `id`=86563 AND `quest`=39731;
DELETE FROM `creature_queststarter` WHERE `id`=86563 AND `quest`=39861;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES (86563, 39731, 0);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES (86563, 39861, 0);
