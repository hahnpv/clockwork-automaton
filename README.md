This repository contains the code used for the 1lb autonomous robot CLOCKWORK AUTOMOTON at Robogames 2018. This is my first autonomous combat robot entry and was designed to "not suck" but mostly to collect LIDAR and video to post-process and come up with a better concept for next year.

(fuller story: I've been a bit of a road warrior this spring, traveling for work leading up to the day of Robogames. I printed a test platform earlier in the year to test concepts to downselect, but ran out of time and used the test platform as my bot)

The robot I built has a XV-11 LIDAR module and a Raspberry Pi Zero W with a camera. The camera has a 360 degree lense, partially occluded aft by the LIDAR module. The initial concept was a very simple LIDAR algorithm based on the assumed battle box: a 6'x6' box with a sumo ring or square (3' diameter circle  or 2'x4' rectangle). The LIDAR would then look at returns between zero and a foot and navigate in the direction with the most returns - any returns over a foot in the latter case might be the walls.

Unfortunately the day before Robogames I discovered that the 2'x4' rectangle had push-outs: polycarbonate walls with holes to push out another bot. This messes up the LIDAR algorithm because clean polycarbonate should allow LIDAR to pass through but oblique hits and scuffed up polycarb would give scattered returns. I could no longer trust the nearest returns to be the opposing bot. So, I ditched the algorithm and just record data for future use.

The architecture is a Arduino which reads reflectance sensors on the bottom corners of the bot and drives the motors. The default algorithm is very simple:
- Go forward
- If front left reflectance sensor senses edge, latch left motor in reverse
- If front right reflectance sensor senses edge, latch right motor in reverse
- If front left reflectance sensor senses edge, latch left motor forward
- If front right reflectance sensor senses edge, latch right motor forward

It would also receive i2c heading commands from the Pi (disabled this time around) 

I competed against four other competitors and got third place (out of 5). I won 2 matches and lost the other two.
