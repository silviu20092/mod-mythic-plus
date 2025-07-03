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

First, the Mythic Plus NPC must be spawned: **.npc add 200005**. Players can now choose a desired M+ level. Each level will have one or more affix, affix descriptions are available via the NPC. Players will not be able to change their M+ level while in a group. Use the NPC to buy Mythic Keystone. Mythic Keystone is an unique item that is used to transform a dungeon into a Mythic Plus dungeon. Players can acquire the keystone once every **MythicPlus.KeystoneBuyTimer** minutes (this is a config option, leave 0 to disable it). Only the group's leader can use the keystone while inside of a Mythic Plus capable dungeon.

### Timer

Each M+ level will have a time limit to beat. If the group beats the timer, then rewards will be given. If timer is not beat, then no rewards will be given, but group can still try to finish the dungeon.
As soon as the group's leader uses a Mythic Keystone, 10 seconds will pass and the dungeon will become Mythic Plus. The dungeon timer will start as soon as the dungeon becomes Mythic Plus. Whenever a Mythic Plus dungeon is completed and timer is beat, each player in the group will receive a Mythic Keystone (configurable).

### M+ dungeons tracking

The system features complex tracking of players that complete M+ dungeons. Each boss kill is saved (with info like total combat time). Players can then check M+ standings for each dungeon and check top timers for example.

### Dungeons that can become Mythic Plus

Use table **mythic_plus_capable_dungeon** to add dungeons that are capable of becoming Mythic Plus. **map** is the ID of the map (like 70 - Uldaman), **mapdifficulty** is the minimum difficulty that player is required to have in order to join Mythic Plus for this specific map (can be either 0 - Normal or 1 - Heroic, adding 1 means the dungeon can only become Mythic Plus on **heroic** difficulty) and **final_boss_entry** is the entry (from creature_template) of the final boss in the dungeon.
Adding old dungeons (for example Ragefire Chasm) is possible, and mobs will scale to max level when the dungeon becomes Mythic Plus. For older dungeons the damage scale can be further adjusted using table **mythic_plus_map_scale**, the columns should be self-explanatory.

### Adding new Mythic Plus levels

You can easily add or customize levels.
To add a new level, simply insert a line into **mythic_plus_level** (world database). The fields should be self-explanatory, **timelimit** is expressed in seconds and represents dungeon's time limit (players will try to beat this timer to get loot). **random_affix_count** is the number of random affixes (see section below) that will be set for this specific level.
Now you can add the rewards, simply insert lines in **mythic_plus_level_rewards**. mythic_plus_level_rewards.lvl links this table with **mythic_plus_level** table. **rewardtype** can either be 0 (in which case **val1** represents the amount of money (copper) that players will get) or 1 (**val1** now is the item entry and **val2** is the amount of items)
To add affixes to a M+ level, insert lines in **mythic_plus_affix**. For **affixtype**, see **enum MythicAffixType** from mythic_affix.h. For **val1**, this represents the specific value for each affix (for example, in case of **AFFIX_TYPE_MORE_CREATURE_DAMAGE** this represents the damage increase percent)

### Random affixes

You can set a random affix count for specific levels (**mythic_plus_level.random_affix_count**). Put 0 if you don't want any random affix. Each time the server is restarted, the mythic level will receive **random_affix_count** random affixes from a predefined pool of random affixes.

### Mythic Plus spell damage scaling

You can further scale spell damage in M+ dungeons using **mythic_plus_spell_override** table. **map** is the ID of the map where the spell will be scaled. **spellid** is the ID of the spell that will receive the scaling. **modpct** is the scale factor for the initial effect of the spell (some spells like Immolate deal initial damage and then DOT damage, the spell id is the same). **dotmodpct** is the scale factor for the DOT damage of the spell.

### Commands

Use **.mythic info** to find information about the current Mythic Plus dungeon. This command is available to all players.
Use **.mythic reload** to reload tables related to Mythic Plus system, not all tables are hot reloadable though.

## Some photos

![pic1](https://github.com/silviu20092/mod-mythic-plus/blob/master/pics/pic1.png?raw=true)
![pic2](https://github.com/silviu20092/mod-mythic-plus/blob/master/pics/pic2.png?raw=true)
![pic3](https://github.com/silviu20092/mod-mythic-plus/blob/master/pics/pic3.png?raw=true)
![pic4](https://github.com/silviu20092/mod-mythic-plus/blob/master/pics/pic4.png?raw=true)

## Credits
- silviu20092