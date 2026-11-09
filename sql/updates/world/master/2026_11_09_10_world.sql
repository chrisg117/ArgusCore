-- Quest 39687 has no quest_template row in this core, so its text rows from 2026_11_09_04 only log
-- "such quest does not exist" at startup. Drop them.
DELETE FROM `quest_offer_reward` WHERE `ID` = 39687;
DELETE FROM `quest_request_items` WHERE `ID` = 39687;
DELETE FROM `quest_details` WHERE `ID` = 39687;
