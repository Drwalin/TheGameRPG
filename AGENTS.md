Developing fantasy MMO RPG game and a custom server and engine.

Definitions:
 - static object - are not subject to physics simulation, (no per tick simulation scripts available; but there can be a script with executed on random tick - very rarely). Static objects can be moved, destroyed and placed by players or npcs. Static objects can move, but it is relatively very rare.
 - dynamic object - differst from static object only by having the ability to have script executed each frame or on timer. Can move each frame.
 - character object - any type of: player, mount, mob, npc, etc.; character objects can move with character controller script.

The following rule-set does not include information about gameplay parts such as
permissions, guilds, aliances, building plots, etc.

Basic game world rules:
 - single world - every players is connected to the same logical world
 - open world - each part of the world is accessible by each player (subject to story or ability restrictions of player)
 - sandbox - world is fully interactive and editable by players (subject to mining, gathering, crafting and building rules)
 - 3d world - fully realized three-dimensional world. Characters can jump or fly
 - every object in the world can be destroyed, placed, activated, inspected, or used in a meaningful way (e.g., open door, pull lever, log a tree, mine a stone)
 - world is divided into regions - each region is around 1 km^2
 - region borders - regions are connected with each other on borders, where are teleports with loading screens (and potentiall reconnection to another physical server)
 - no physics based rotation - objects cannot spin or tuble from forces
 - objects can fall, slide or float on water - very limited physics: gravity applies to characters and some scripted objects
 - most objectes ae unmovable unless scripted - by default objects stay in place until destroyed or moved by scripts
 - collision only between character and static/dynamic objects (character cannot walk through walls, doors, or trees), but can overlap other characters
 - first person camera - player view world through character's eyes
 - game pace - not fast but also not very slow; not twitch-reflex; not turn-based; between Minecraft and Albion Online
 - world size - world size is infinite (or limited by server resources), new regions are generated when players reach them
 - character-character collision - in future instead of true character-character collision can be implemented with push-out force between characters during combat

Technical constraints:
 - engine - custom built mmo engine optimised exclusively for the game
 - server tick rate - 20 frames (ticks) per second
 - region threading - each region runs on a single thread, no parallel simulation inside a region
 - worker threads for regions - server can have loaded for example 50 regions, and have 4 worker threads. Worker threads execute simulations for regions with round robin based distribution and priority (by last time execution) queue to ensure 20 tps for each region
 - networking - each region receives and sends data to a dedicated networking thread when communication is needed (e.g. player position change)
 - physics scope - no rigid body rotation. gravity only for characters, NPCs, mos and optionally scripted objects or projectiles, which can collide with both world and characters
 - character collision representation - collision and targeting use cylinder shape (simplifies algorithgms for continous collision detection for simple shapes)
 - collision engine - collision detection implemented with ray-tracing for CCD where tested object is expanded by the shape of character's cylinder shape (some collision shapes use approximation instead of rounded corners for further optimisations)
 - no character-character collision - intentionally disabled to reduce simulation complexity
 - client program - is used only for rendering world state and senting player input to the server, assuring 100% server authority and preventing most forms of cheating

Other constraints, limitations and predictions:
 - world size / regions count limit - logically infinite, but can be limited to few thousands or around 1 TB of active data stored (old static world state is purged when updated)
 - number of static/dynamic objects in a single region:
    - average: 50000-100000
    - maximum: soft limit of 1000000, no hard limit
    - minimum: 10000
 - player capacity in single region:
    - most regions will be unloaded from memory, due to no players active on them
    - most loaded reagions would have around 5-20 players active
    - popular regions may have 200-500 players
    - hard limit defined by hardware/software infrastructure limitations
 - permissions - initially free for all, but later players can claim plots
 - PvP - some regions are PvE, but some have enable PvP
 - teleport triggers loading screen for clients
 - combat is a mix of target based and melee/area distance checks
 - AI - initially path finding is very simple, npcs go directly toward enemies, maybe simple papth finding with few raycasts to go around very simple walls or barriers
 - AI path finding updates each 10-20 times per second during combat
 - AI path finding updates once every few second outside of combat, except when following players, can be around 1-5 times per second
 - each object in a world can have an invetory component, but this is outside of scope of this document
 - server sends delta updates to clients of nearby characters and dynamic objects
 - server sends to every player changes in static objects (which should be less frequent)
 - to reduce frequency of destruction and placement of static objects, each such action has a timer (for example, requires at least 500 milliseconds)
 - server is fully authoritative, clients receive valid state and render old (by 3 ticks = 150 milliseconds) interpolated state, clients send to server only their movement and actions (such as pressing a button or hitting character or a tree)
 - other types of interactions with inventories or crafting - request based just like a web page, but using internal netowrking and rendering systems
 - world persistence - everything is stored in distributed database (for example ScyllaDB), world state is stored in batches - all changes (since last checkpoint) to world and charactters are stored in a single transaction - this allows for high consistency, but can be slow
 - full state synchronisation - clients cache static objects of the world with timestamp (tick id) of their last update. When client joins, the server sends only changes - newer objects, changed objects and destroyed objects
 - expected latency: the game is expected to work fine even with round trip time up to 150 milliseconds
 - database checkpoint can be executed at most once per second
