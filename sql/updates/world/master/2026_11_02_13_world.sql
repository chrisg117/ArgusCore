-- The Scythe of Souls (37660): Allari stays phased away once Arev'naal is compelled. Phase 4462's
-- conditions (Illidari Stand) bring her back on objective 277159, but the core re-evaluates phase
-- conditions on an objective only when its completion effect asks for it.
DELETE FROM `quest_objectives_completion_effect` WHERE `ObjectiveID`=277159;
INSERT INTO `quest_objectives_completion_effect` (`ObjectiveID`, `GameEventID`, `SpellID`, `ConversationID`, `UpdatePhaseShift`, `UpdateZoneAuras`) VALUES
(277159, NULL, NULL, NULL, 1, 0); -- Arev'naal compelled
