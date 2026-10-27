-- 39742 "Vault of the Wardens" (bonus objective): drop the zone-only grant and auto-reward scripts
-- (player_ / quest_vault_of_the_wardens_bonus_objective). Task quests are to be handled by the engine instead.
-- Its objectives, kill credits and the Legion Portals' spellclick stay.
UPDATE `quest_template_addon` SET `ScriptName`='' WHERE `ID`=39742;
DELETE FROM `quest_poi` WHERE `QuestID`=39742;
DELETE FROM `quest_poi_points` WHERE `QuestID`=39742;
