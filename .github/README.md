# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore

# Mythic Plus system for Azerothcore

## Overview

Adds the possibility to transform certain dungeons into Mythic Plus dungeons. This module aims to increase the difficulty of these dungeons by adding certain affixes that players can choose before.

## How to install

1. Clone this repository to your AzerothCore repo modules folder. You should now have mod-mythic-plus there.
2. Re-run cmake to generate the solution.
3. Re-build your project.
4. You should have mod_mythic_plus.conf.dist copied in configs/modules after building, copy this to configs/modules in your server's base directory.
5. Start the server, .sql files should automatically be imported in DB, if not, apply them manually.

## How it works

First, the Mythic Plus NPC must be spawned: **.npc add 200005**. Players can now choose a desired M+ level (for now, only 5 levels are available). Each level will have one or more affix, affix descriptions are available via the NPC.
You will always need to be in a group for a dungeon to become M+, and the Mythic level chosen will always be taken from the group's leader. While in a group, players will not be able to change their M+ level.
If in a group and the leader has a M+ level set, then as soon as a dungeon is joined (no matter by which player in the group), then it will become Mythic Plus and it will be saved as such (until reset).

### Timer

Each M+ level will have a time limit to beat. If the group beats the timer, then rewards will be given (check **MythicPlus::CreateMythicLevels()** and you can modify the rewards or levels to your liking). If timer is not beat, then no rewards will be given, but group can still try to finish the dungeon.
Timer countdown starts as soon as a **creature inside the dungeon** is killed. Until a creature is killed, players can stay inside (perform buffs, checks etc) and the countdown won't begin.

### M+ dungeons tracking

The system features complex tracking of players that complete M+ dungeons. Each boss kill is saved (with info like total combat time). Players can then check M+ standings for each dungeon and check top timers for example.


### Dungeons that can become Mythic Plus

1. Pit of Saron (normal/HC)
2. Forge of Souls (normal/HC)
3. Ahn'Kahet the Old Kingdom (HC)
4. Azjol Nerub (HC)
5. Drak'Tharon Keep (HC)
6. Gundrak (HC)
7. Halls of Lightning (HC)
8. Halls of Stone (HC)
9. The Nexus (HC)
10. The Oculus (HC)
11. Utgarde Keep (HC)
12. Utgarde Pinnacle (HC)

## Some photos

![pic1](https://github.com/silviu20092/mod-mythic-plus/blob/master/pics/pic1.png?raw=true)
![pic2](https://github.com/silviu20092/mod-mythic-plus/blob/master/pics/pic2.png?raw=true)
![pic3](https://github.com/silviu20092/mod-mythic-plus/blob/master/pics/pic3.png?raw=true)

## Credits
- silviu20092