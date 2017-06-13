Twinstick Sumo
==============

A game made in C over the course of (primarily) a single day using an engine built 'from scratch'.

[Download here](https://github.com/azmr/twinstick-sumo/releases).

Engine
------

This was made largely based on code and ideas from the [Handmade Hero](https://www.youtube.com/user/handmadeheroarchive) series. These were added to with some more basic drawing capabilities, among a few other features.

I'm not 100% sure if there are any licensing issues here, as despite typing all the code myself, there is significant overlap, so I'm not including those files here for the moment to be on the safe side.

The engine is by no means finished, but it was at a stage where something could be made. Some features are still in a fairly simplistic form, and would significantly benefit from optimisation. Given the fairly slow single-threaded software rendering, you need a relatively fast CPU to play this at full speed (30fps).

Development
-----------

The full local multiplayer aspect of the game was made in 1 day over roughly 10-12 hours. The next day I played it with a friend and made a few tweaks. I also added an AI for when only one controller is plugged in. This AI has a number of metrics controlling its behaviour that are randomised at the beginning of each round. I was time-limited on the day, and it could probably do with some more tweaking, as there are definitely cases it doesn't handle well.

The aim was to make something from start to finish very quickly, so a few compromises were made. The most noticeable is that I made no attempt to add any audio. Some abstractions would probably have to evolve were many more features to be added to the game.
I'm still pretty happy with the final result - it's the first time I've set deadlines for my personal projects, it's the first 'complete' game I've made, it's my first attempt at AI, and it's actually fun to play for 1 or 2 people.

Game
----

### Aim
Try to knock the other player out of the ring while remaining in it yourself.

### Controls
XInput Gamepad (e.g. Xbox 360 controller) required. Plugging in/unplugging a second controller will switch player 2 between AI and human-controlled.

 - Move with the left stick
 - Aim in the direction you are moving or with the right stick if moved
 - RB (R1) to throw projectiles
 - Start to view pause menu, to reset/add extra obstacle
 - Alt + Enter to enter/exit full screen (expects a 1080p screen, untested on any others).

The rest you can figure out by playing!
