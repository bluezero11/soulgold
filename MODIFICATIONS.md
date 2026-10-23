# SoulGold-mod Changes

This mod is intended to make a pure-doubles run easier. Lots of personal choices that should be gated behind a cheat code so that they are opt-in rather than default, but cheat code system was added much later.


## 1. Quick rundown of changes

- General ease of play
   - Increased starting money
   - First Poké Ball gift gives 90
   - Catch rate x8
   - Ether now available when Great Balls available
   - Halved prices for Revive, Max Revive, Ether
   - Megastones available with Elm aide when megas unlock
- Additional/adjusted gift mons
   - Second starter offered after first selection, from all 9 gens.
   - Cherrygrove gift changed to Sinistea, Poltchageist.
   - Added Violet City gift, choice of several bug mons.
- Rival teams adjusted to account for doubles
   - Silver: Starly in the first encounter, Pidgeotto swapped out for Staravia.
   - Gold/Crystal: Added Mareep initially and Flaaffy later
- Cheat code system
   - MONO[TYPE] gives random mon of that type
   - ROLLTHEDIE gives random mon
   - Add 3 or 6 to above to get batch of randoms with no duplicates. ROLLTHEDIE[3/6] ensures different types for all within batch.
   - GIVEALLQOL to get QOL items
- QOL items
   - Stat editor: EV, IV, nature, ability, gender
   - Repel charm
   - Box link
   - Pokevial



## 2. Source attribution and lineage


- **Original EV/IV Stat Editor** 
   - **Archie / TeamAquasHideout:** [original March 2024 stat-editor commit](https://github.com/TeamAquasHideout/pokeemerald/commit/317d345a33ae) and [installation tutorial](https://github.com/pret/pokeemerald/wiki/Add-a-EV---IV-Stat-Editor-UI). **Jaizu:** original editor graphics ([Team Aqua's feature credits](https://github.com/TeamAquasHideout/Team-Aquas-Asset-Repo/wiki/Feature-Branches)). 
   - Ported and adapted, sprite flip fixes. 
- **Expanded Stat Editor** 
   - Expands Archie's editor foundation, referenced against [PCG06/pokeemerald's `feat/stat_editor`](https://github.com/PCG06/pokeemerald/tree/feat/stat_editor) 
   - Added nature/ability changer (reference-assisted), gender changer and PID-preservation algorithm (custom).
- **Poké Vial** 
   - [evilchinesefood/PKMN-World](https://github.com/evilchinesefood/PKMN-World), [`src/pokevial.c`](https://github.com/evilchinesefood/PKMN-World/blob/master/src/pokevial.c)
   - Ported with minor adjustments. 
- **Repel Charm** 
   - [pokeemerald-expansion Infinite Repel Item guide](https://github.com/rh-hideout/pokeemerald-expansion/wiki/Infinite-Repel-Item), contributed by `jtebbe`
   - Ported with minor adjustments. 
- **Pokémon Box Link** 
   - Already implemented in SoulGold and the [pokeemerald-expansion Box Link feature](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/FEATURES.md) 
   - Gifted on cheat code entry.
- **Cheat code system** 
   - **Creative inspiration:** [Nemo622's Pokémon Lazarus](https://ko-fi.com/nemo622/shop). 
   - **Implementation foundation:** existing [SoulGold `master` `src/field_specials.c`](https://github.com/bluezero11/soulgold/blob/master/src/field_specials.c), with `EnterCode`, `GetCodeFeedback`, naming-screen input and replay-option codes. 
   - Implemented gifts to help mono-type runs, and random start runs. Option to batch 3 or 6 non-duplicated gifts. Gifts QOL items and stat editor on code redeem. 




---



---



---

## 3. Detailed notes for personal reference


### 3.1 Git / build workflow notes

For GitHub Codespaces:

```bash
git pull
make -j$(nproc)
```

If dependencies need to be installed in a fresh Codespace:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  binutils-arm-none-eabi \
  gcc-arm-none-eabi \
  libnewlib-arm-none-eabi \
  git \
  libpng-dev \
  python3
```

Output ROM:

```text
Soulgold-mod.gba
```


### 3.2 Flags

Many errors were caused when assigning new flags to existing ones. Before reusing a flag in future, check defined numeric IDs.

Existing ones included:

- `0x297` --> `FLAG_WONDERTRADE1`
- `0x298` --> `FLAG_WONDERTRADE2`
- `0x299` --> `FLAG_WONDERTRADE3`
- `0x29B` --> `FLAG_FRONTIER_SECOND_CLERK`
- `0x46D` --> `FLAG_ITEM_ROUTE2_NUGGET`
- `0x21` --> `FLAG_SYS_NO_ENCOUNTER` (aliased by `FLAG_REPEL_CHARM_ACTIVE`) 

New flags used:
- `0x103E` --> `FLAG_RECEIVED_VIOLET_ACADEMY_GIFT`
- `0x103F` --> `FLAG_QOLUNLOCK_REDEEMED`

Potential free custom flags that could be used:
- `0x1040` 


### 3.3 Second starter selection

Immediately after choosing the normal starter, Elm now offers a **second starter**.

The list contains:

- "No thanks!" first
- All Generation 1–9 starter Pokémon
- The already-selected primary starter is automatically omitted

There are therefore 27 menu entries at runtime:

- 1 decline option
- 26 remaining starters

The selected second starter is given at **Lv. 5**.

Declining ends the offer; it is not intended to become a repeatable choice later.

Relevant files:

```text
data/maps/NewBarkTown_Lab/scripts.pory
data/maps/NewBarkTown_Lab/scripts.inc
```

The selector uses the existing dynamic multichoice system and `VAR_PLAYER_STARTER_SPECIES` to skip the first starter.

Elm's prompt references the intended doubles-focused playthrough:

> Many trainers favour Double Battles. Would you like to pick a second Pokémon?

### 3.4 Gift mon preview support

New dynamic-menu callback was implemented to display a Pokémon's **front sprite while the cursor is highlighting that species**.

New callback constant:

```c
DYN_MULTICHOICE_CB_SHOW_POKEMON
```

Relevant files:

```text
include/constants/script_menu.h
src/script_menu.c
```

Callback creates an 8×8-tile window beside the dynamic menu and creates/destroys the selected Pokémon front sprite as the cursor moves.

This preview system is used by:

- Elm's second starter selector
- Cherrygrove tea-Pokémon gift
- Violet City gift selector

`SPECIES_NONE` produces a blank preview for "No thanks" / decline options.

The implementation follows the repo's existing dynamic item-preview callback as a model.

Potential visual concern to keep in mind: very wide menus plus the 8×8 preview window could theoretically approach the GBA screen width, so these menus should still receive emulator/UI testing.

---

### 3.5 Cherrygrove gift Pokémon

Cherrygrove gift choice changed to tea mons, which are both reasonably strong. Poltchageist was intentionally a choice to help players with doubles matches from early on.

**Gift Sinistea (Phony)**, moves Astonish, Withdraw, Will-O-Wisp, and Confusion.

**Gift Poltchageist (Counterfeit)**, moves Astonish, Withdraw, Stun Spore, and Grassy Terrain.

For level-up progression, both Sinistea and Polteageist now learn Hex at level 30 instead of Sweet Scent. 

Overworld sprites were also changed to match the new gift mons.

The menu was converted from index-based results to species-valued dynamic menu entries so the front-sprite preview callback could work directly.

Relevant files:

```text
data/maps/CherrygroveCity/map.json
data/maps/CherrygroveCity/scripts.pory
data/maps/CherrygroveCity/scripts.inc
```

Declining still follows the original decline/shame dialogue route.

---

### 3.6 Violet City gift Pokémon

The one-time Violet Academy / Violet City boy NPC gift now offers twelve enabled **Bug-type first-stage Pokémon**, each at **Lv. 5**.

| Pokémon | Gift moves |
| --- | --- |
| Caterpie | Tackle, String Shot, Bug Bite, Electroweb |
| Cutiefly | Absorb, Fairy Wind, Sticky Web, Powder |
| Grubbin | Vise Grip, Mud Slap, String Shot, Electroweb |
| Joltik | Normal level-up starting moves (default) |
| Nincada | Sand Attack, Scratch, Harden, Bug Bite |
| Sewaddle | Tackle, String Shot, Camouflage, Grassy Terrain |
| Sizzlipede | Ember, Smokescreen, Wrap, Bug Bite |
| Skorupi | Normal level-up starting moves (default) |
| Spinarak | Poison Sting, Twineedle, Toxic Spikes, Rage Powder |
| Venipede | Poison Sting, Rollout, Toxic Spikes, Spikes |
| Weedle | Poison Sting, String Shot, Bug Bite, Electroweb |
| Wimpod | Struggle Bug, Aqua Jet, Wide Guard, Spikes |


Relevant files:

```text
data/maps/VioletCity/scripts.pory
data/maps/VioletCity/scripts.inc
include/constants/flags.h
```


---

### 3.7 Rival team changes

Silver's early teams were adjusted to establish a second Pokémon from the beginning. Added Starly, changed Pidgeotto to Staravia.

Later teams already progress through Staravia and Staraptor.

Gold + Crystal early teams were expanded with Mareep then Flaaffy.

Relevant trainer changes are in:

```text
src/data/trainers.party
```

---

### 3.8 Mega Stone shop

Elm's permanently-present female lab aide (`AIDE2`) becomes a Mega Stone seller once:

```text
VAR_NEWBARKTOWN_LABSTATE >= 8
```

All 18 type-based Mega Stones are sold, followed by Bondstone at the bottom:

Relevant files:

```text
data/maps/NewBarkTown_Lab/scripts.pory
data/maps/NewBarkTown_Lab/scripts.inc
src/data/items.h
```

### 3.9 Stat editor

New files:

```text
src/ui_stat_editor.c
include/ui_stat_editor.h
graphics/ui_menu/a_button.png
graphics/ui_menu/b_button.png
graphics/ui_menu/background_pal.pal
graphics/ui_menu/background_tileset.bin
graphics/ui_menu/background_tileset.png
graphics/ui_menu/dpad_button.png
graphics/ui_menu/r_button.png
graphics/ui_menu/selector.pal
graphics/ui_menu/selector.png
graphics/summary_screen/bw/shadow.pal
```

Party-menu integration touches the party-menu source/data variants present in this project, including:

```text
src/party_menu.c
src/data/party_menu.h
src/swsh_party_menu.c
src/data/swsh_party_menu.h
```

Compatibility adaptations:

1. **Ability-info declaration**

   The imported editor's header declared:

   ```c
   extern const struct Ability gAbilitiesInfo[];
   ```

   SoulGold uses:

   ```c
   extern const struct AbilityInfo gAbilitiesInfo[];
   ```

   The declaration was updated accordingly. The incorrect donor declaration conflicted with SoulGold's `enum Ability`.

2. **Pokémon summary animation API**

   The imported editor called an older four-argument form:

   ```c
   PokemonSummaryDoMonAnimation(sprite, sprite->sSpecies, isEgg, sprite->sIsShadow);
   ```

   SoulGold's current API is:

   ```c
   PokemonSummaryDoMonAnimation(sprite, species, oneFrame);
   ```

   The editor now calls:

   ```c
   PokemonSummaryDoMonAnimation(sprite, sprite->sSpecies, isEgg);
   ```

3. **Imported scrolling-background dependency**

   The imported variant referenced `gScrollBgTiles`, `gScrollBgTilemap`, and `gScrollBgPalette` globals not in SoulGold. Scrolling layer was removed. The stat editor retains its own primary background and interface graphics.

4. **Donor source cleanup**

   Corrected missing semicolon in setup code. Removed unused secondary tilemap buffer associated with the removed scrolling background.

UI adjustments: 

Current background-accent palette choices:

```text
Large triangle:       RGB 92, 103, 122
Light grey accent:    RGB 205, 209, 216
Mid grey accent:      RGB 166, 172, 182
```



- **Gender editing:** Male/female species can toggle gender; genderless and fixed-gender species have editing blocked. The editor calculates a staged 32-bit personality using a bounded, deterministic search: it preserves the original PID's **upper 16 bits**, **natural nature** (PID % 25) and, whenever possible, the lower two bits of each byte (for Unown-letter logic). If an extreme custom gender ratio prevents preserving the lowest two bits of the bottom byte, the search relaxes that restriction but still preserves the upper half, original nature, and bits 8-9.

- **Animation flip fix:** The Pokémon Summary animation engine can leave an old affine matrix index in `oam.matrixNum` when a sprite stops using affine rendering. In normal rendering these same bits serve as horizontal/vertical flip flags, sometimes leaving Pokémon sprites mirrored or upside down, especially after party switching. The SwSh party menu already works around this in `RunMonAnimTimer`. The stat editor now also sanitizes `oam.matrixNum` from each sprite's `hFlip`/`vFlip`, **only after the animation has finished and affine mode is off**. This runs after `AnimateSprites` and before `BuildOamBuffer` for both Pokémon and shadow, without affecting active animations.


### 3.10 Cheat code

- Forms
   - Shellos, Flabébé, Tatsugiri use one master-list ticket each and roll cosmetic forms only after the eligible ticket is selected. Thus 1 ticket shared for all cosmetic forms. 
   - Other forms are not just cosmetic changes (e.g., Tauros, Ratatta), so each gets 1 separate ticket.
- ROLLTHEDIE3 / 6 batch type diversity 
   - When selected, a mon's types are masked from next selections. 
   - All mons are preselected before the SNES gift script starts.
   - Current pool guarantees batch of 6 will always complete. If an edited future pool cannot complete a batch, the code fails rather than delivering a partial batch.

- **MONO[TYPE] gifts:** Uses the same full curated `sRandomizerMons` list as `ROLLTHEDIE`, filtered by the requested primary **or** secondary type (no separate monotype pools). Each eligible entry has an equal chance; suffix `3` or `6` selects distinct entries without replacement.
- **Relevant files:** `src/field_specials.c` (species list, type filters, selection logic); `data/maps/NewBarkTown_PlayersHouse_2F/scripts.inc` (SNES code entry and gift delivery).

### 3.11 Mart changes

Ether made available in shops once Great Balls are available.

Relevant file:

```text
src/shop.c
```

Halved item prices to:

- Revive: **₽1,000**
- Max Revive: **₽2,000**
- Ether: **₽1,500**

Relevant file:

```text
src/data/items.h
```


### 3.12 Mini economy changes


- Original: ₽3,000
- Current value: **₽150,000**

Implemented in:

```text
src/new_game.c
```

Current logic:

```c
SetMoney(&gSaveBlock1Ptr->money, 150000);
```

The first Poké Ball gift in Elm's lab was increased to:

- **90 Poké Balls**

Implemented in both:

```text
data/maps/NewBarkTown_Lab/scripts.pory
data/maps/NewBarkTown_Lab/scripts.inc
```

### 3.13 Catch rates

Catch rates were increased globally by **8×**, capped at 255. 

Multiplier is applied during capture-odds calculation after the flat ball bonus and before the later HP/ball calculations.

Implemented in:

```text
src/battle_script_commands.c
```


### 3.14 Oak's intro and player selection

- Oak now displays **Rufflet** instead -- both the Poké Ball release and displayed front sprite. 
- Player selection now uses **Left / Right** instead, with prompts before and after adjusted to match. 

Relevant files:

```text
src/main_menu.c
data/text/birch_speech.inc
```
