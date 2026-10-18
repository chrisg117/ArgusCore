-- Nythendra yells with their voice lines (broadcast text Text1 = female, SoundKitID2)
DELETE FROM `creature_text` WHERE `CreatureID` = 102672 AND `GroupID` BETWEEN 3 AND 7;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(102672, 3, 0, 'The Dream has changed. Yet I will protect it still.', 14, 0, 100, 0, 0, 58664, 0, 105004, 3, 'Nythendra - Aggro'),
(102672, 4, 0, 'I dreamed the Dream. Now I wake...', 14, 0, 100, 0, 0, 58665, 0, 105002, 3, 'Nythendra - Death'),
(102672, 5, 0, 'Breathe deeply. Embrace your fate.', 14, 0, 50, 0, 0, 58667, 0, 109011, 3, 'Nythendra - Infested Breath'),
(102672, 5, 1, 'You have no more need of this life!', 14, 0, 50, 0, 0, 58668, 0, 109013, 3, 'Nythendra - Infested Breath'),
(102672, 6, 0, 'No... I still have much to do... RETURN TO ME!', 14, 0, 100, 0, 0, 58669, 0, 109014, 3, 'Nythendra - Heart of the Swarm'),
(102672, 7, 0, 'Consumed from within!', 14, 0, 50, 0, 0, 58670, 0, 109016, 3, 'Nythendra - Volatile Rot'),
(102672, 7, 1, 'Your wakened life is a disease!', 14, 0, 50, 0, 0, 58671, 0, 109017, 3, 'Nythendra - Volatile Rot');
