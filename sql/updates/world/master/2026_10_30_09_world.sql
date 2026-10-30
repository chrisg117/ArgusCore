-- The blue middle of Dalaran's two teleport pads stood a little above their rings. 0.65 below the floor it lies inside
-- them; at 0.75 it is gone.
UPDATE `gameobject` SET `position_z`=735.23 WHERE `guid`=12200003; -- the light of the Teleport Pad in the middle of the city
UPDATE `gameobject` SET `position_z`=601.83 WHERE `guid`=12200005; -- the light of the Teleport Pad in Aegwynn's Gallery
