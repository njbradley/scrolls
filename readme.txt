This is version 0.4 (in dev) of scrolls alpha: the adventure game!

Getting started:

Precompiled distributions:
Right now, there is a windows exe release if you want to play the game without setting up the compilation environment.
Just download the zip file and skip to the section Playing the game.

Compiling:
To compile scrolls, you will need a c++14 compatable compiler, and the libraries: glfw, glew, glm, stb_image, boost
On windows, most of these libraries can be downloaded in precomplied states, and only glfw, glew and boost need libraries,
the rest are header only. I use Mingw for compiling, which works well.
On windows, msys2 can be used to install the libraries:
	mingw-w64-x86_64-gcc
	mingw-w64-x86_64-make
	mingw-w64-x86_64-freealut
	mingw-w64-x86_64-openal
	mingw-w64-x86_64-glm
	mingw-w64-x86_64-glfw
	mingw-w64-x86_64-glew
	mingw-w64-x86_64-boost
	mingw-w64-x86_64-dwarfstack
On linux, apt can be used:
	libglfw3-dev
	libglew-dev
	libalut-dev
	libglm-dev
	libboost-all-dev
On mac and linux, the libraries can be installed with atp-get or brew.
The compile command is rather simple with make, run the command "make main" in the
root directory. Specify the platform by adding PLAT=MAC or PLAT=LINUX.
on windows with mingw the command is "mingw32-make main" but otherwise
it is the same.
Thats it! now you should have an executable! However, it has to be kept with the resources folder wherever you run it.

Playing the game:
To play the game, double click on main.exe. A window should pop up with a new world open.
The controls are listed below, so walk around and explore! I am working on a tutorial, but for now, there is not
much guidance for what to do. A few pointers:
 -You can create a crafting table by collecting crafting table blocks (normally in the terrain structures)
  and place them in a square. A 2x2 crafting table is level 2, a 3x3 table is level 3, and so on. Each level unlocks more
  and more recipes.
 -Skeletons spawn in the snow, so watch out for them sneaking up on you, they blend right in
 -Pigs are a great source of food, they spawn in the grassy areas
 -The night is pitch black, so make sure to craft some lanterns and place them down when it starts to get dark
 
Controls:
WASD - movement
Mouse - look around
Scroll - change holding item
Number Keys - select holding item
Left click - destroy item (takes time)
Right click - place item or interact with special blocks
E - inventory
C - hand crafting
M - main menu
Ctr-Q - quit

Debug:
O/P - show/hide debug menu
R - force cleaning of memory vectors
F/G - enter/exit spectator mode

Version 0.4:
 Additions:
 -implemented transparent blocks, including water!
 -water has physics and flows (almost working)

Version 0.3:
 Additions:
 -many new randomly generated terrain structures
 -block physics is now working! now blocks fall if they are not attached to another block they stick to
 -monsters have been added! When they die they become part of the landscape, and you can mine their bodies to get loot
 -player movement has been improved, you can now go up one block steps automatically without losing your velocity
 -you can also construct ladders out of blocks that the player can climb, and the player can stand in gaps in the wall
 -biomes have been added, right now there are only two but soon there will be much more
 -lighting is finally working! there is a day/night cycle, and at night you need lamps to be able to see
 -tools have been overhauled, with a new durability system. every tool has a weight level and a sharpness level
  when you mine, the sharpness decreases. you can craft a grindstone to sharpen the tool, but that wears away
  and decreases the weight
 -chests have been changed, now every block holds one item, but if you place chests next to each other they combine up to 10 spots

Version 0.2:
 Additions:
 -full multithreading of loading and saving chunks, so no lag while playing
 -overhaul of terrain generation, much more expandable for future updates
 -world is now infinite in all directions with no build limits
 -worlds are encapsulated in a zip folder for efficient storage
 -settings file for adjustment
 -more blocks and items
 -trees!
 -lots of bugs from the multithreading

Version 0.1:
 Right now the game is barely in a playable state, only the base features are in place.
