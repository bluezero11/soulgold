# SoulGold Modifications

This file documents the custom changes made on the `dev` branch of `bluezero11/soulgold`, plus important implementation findings and build/debugging notes discovered while compiling the project. It also records feature work developed on short-lived branches before merge.

The intention is to keep `master` as the untouched/reference branch, keep `dev` as the main custom gameplay branch, and isolate larger experimental features on dedicated branches until they compile and test cleanly.

## Build status

A full GBA ROM build has now completed successfully in GitHub Codespaces after fixing several flag-definition conflicts described below.

Build command used:

```bash
make -j$(nproc)
```

Output ROM:

```text
Soulgold.gba
```

During the successful-link debugging process, memory usage was approximately:

```text
EWRAM: 255428 B / 256 KB   (97.44%)
IWRAM:  24156 B / 32 KB    (73.72%)
ROM: 32295928 B / 32 MB    (96.25%)
```

These values are high, especially EWRAM and ROM usage, but they were not the cause of the linker failures encountered.

---

## Feature branch: stat editor

Branch:

```text
feature/stat-editor
```

This branch was created from a clean, compiling `dev` state so the stat-editor transplant could be developed and debugged without destabilizing `dev`.

### Overview

A reusable Pokémon stat editor was transplanted from `fakuzatsu/verdant`, a pokeemerald-expansion-based project.

The editor is accessible directly from the normal field party menu. After selecting a Pokémon, the action list now includes:

```text
Edit stats
```

The option is intentionally placed **second-last**, immediately before **Cancel**, regardless of the other available party actions.

Selecting it opens the editor for that specific party Pokémon. Exiting the editor returns to the party menu.

The current editor supports direct editing of all six:

- EVs
- IVs

It also displays the Pokémon's:

- actual calculated stats
- ability
- nature
- level
- gender
- sprite

Nature and ability are currently display-only in this transplanted version.

### Main implementation files

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

### SoulGold compatibility adaptations

The Verdant editor was not copied completely unchanged because SoulGold has diverged from the donor's pokeemerald-expansion revision.

Important compatibility fixes made during the transplant:

1. **Ability-info declaration**

   Verdant's header declared:

   ```c
   extern const struct Ability gAbilitiesInfo[];
   ```

   SoulGold uses:

   ```c
   extern const struct AbilityInfo gAbilitiesInfo[];
   ```

   The declaration was updated accordingly. The incorrect donor declaration conflicted with SoulGold's `enum Ability` and caused cascading compile errors in party-menu code.

2. **Pokémon summary animation API**

   Verdant called an older four-argument form:

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

3. **Verdant scrolling-background dependency**

   The donor editor relied on Verdant-specific `gScrollBgTiles`, `gScrollBgTilemap`, and `gScrollBgPalette` globals that SoulGold does not contain.

   Rather than importing unrelated UI systems, that scrolling layer was removed. The stat editor retains its own primary background and interface graphics.

4. **Donor source cleanup**

   A missing semicolon in the transplanted setup code was corrected, and an unused secondary tilemap buffer associated with the removed scrolling background was removed.

### Stat editor UI adjustments

The initial transplant worked functionally but several palette choices did not fit SoulGold cleanly.

The following visual refinements were made after runtime testing:

- The **A** and **START** button graphics use a clear **magenta outline**.
- START lettering was brightened so the button reads clearly.
- The **magenta selection cursor** remains unchanged as a strong interaction cue.
- The original large bright-magenta triangle behind the Pokémon was replaced with a muted **slate-grey-blue** so the Pokémon sprite is visually dominant.
- The donor's teal accent colors were replaced with cool/light greys to better match the rest of the interface.
- Dark text on the light stat panel now uses a light-grey accent shadow for improved legibility.
- Magenta button/cursor accents were deliberately preserved while the background accents were desaturated.

Current background-accent palette choices include approximately:

```text
Large triangle:       RGB 92, 103, 122
Light grey accent:    RGB 205, 209, 216
Mid grey accent:      RGB 166, 172, 182
```

### Build and runtime status

The feature branch has completed a full successful ROM build.

Observed successful-link memory usage:

```text
EWRAM: 255432 B / 256 KB   (97.44%)
IWRAM:  24156 B / 32 KB    (73.72%)
ROM: 32303852 B / 32 MB    (96.27%)
```

The build completed through:

```text
arm-none-eabi-ld
gbafix Soulgold.elf
arm-none-eabi-objcopy -O binary Soulgold.elf Soulgold.gba
gbafix Soulgold.gba
```

Runtime testing has also confirmed that:

- **Edit stats** appears in the party Pokémon action menu.
- It is positioned immediately before **Cancel**.
- The editor opens successfully for the selected Pokémon.
- The EV/IV editor interface renders correctly.
- The compatibility fixes compile and run.
- The revised button/text palettes display correctly.
- The later slate-grey-blue / grey accent palette is working well in emulator testing.

At the time this section was written, `feature/stat-editor` was **18 commits ahead of `dev` and 0 behind**, and was considered ready to document before merging.

---

## New game and economy

### Starting money

Starting money was first raised dramatically for testing/economy experimentation, then reduced to the current value:

- Original: ₽3,000
- Earlier custom value: ₽300,000
- Current value: **₽150,000**

Implemented in:

```text
src/new_game.c
```

Current logic:

```c
SetMoney(&gSaveBlock1Ptr->money, 150000);
```

### Poké Ball gift

The first Poké Ball gift in Elm's lab was increased to:

- **90 Poké Balls**

Implemented in both:

```text
data/maps/NewBarkTown_Lab/scripts.pory
data/maps/NewBarkTown_Lab/scripts.inc
```

### Catch rates

Catch rates were increased globally by **8×**, capped at 255.

Implemented in:

```text
src/battle_script_commands.c
```

The multiplier is applied during capture-odds calculation after the flat ball bonus and before the later HP/ball calculations.

### Shiny odds

Base shiny odds were changed to approximately:

- **1 in 64**

Relevant changes include:

```text
include/constants/pokemon.h
src/pokemon.c
src/option_menu.c
```

The visible highest shiny-rate option was also relabeled to reflect 1-in-64 odds while internal enum names were left unchanged.

---

## Player selection labels

The opening player-character choice labels were changed to:

- **Kid A**
- **Kid B**

This is a display/text change only; the game's underlying gender/player-character mechanics were not removed.

Relevant files include:

```text
data/text/birch_speech.inc
src/main_menu.c
```

---

## Starter changes

### Original starter selection

The custom nine-Pokémon starter screen itself was left intact.

The nine primary starters remain:

- Chespin
- Fennekin
- Froakie
- Chikorita
- Cyndaquil
- Totodile
- Sprigatito
- Torchic
- Popplio

The idea of expanding the initial custom starter UI to all 27 starters was considered but intentionally not implemented.

### Second starter selection

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

### Starter preview support

A new dynamic-menu callback was implemented to display a Pokémon's **front sprite while the cursor is highlighting that species**.

New callback constant:

```c
DYN_MULTICHOICE_CB_SHOW_POKEMON
```

Relevant files:

```text
include/constants/script_menu.h
src/script_menu.c
```

The callback creates an auxiliary 8×8-tile window beside the dynamic menu and creates/destroys the selected Pokémon front sprite as the cursor moves.

This preview system is used by:

- Elm's second starter selector
- Cherrygrove tea-Pokémon gift
- Violet City gift selector

`SPECIES_NONE` produces a blank preview for "No thanks" / decline options.

The implementation follows the repo's existing dynamic item-preview callback as a model.

Potential visual concern to keep in mind: very wide menus plus the 8×8 preview window could theoretically approach the GBA screen width, so these menus should still receive emulator/UI testing.

---

## Cherrygrove gift Pokémon

The early Cherrygrove gift choice was changed from the previous regional rodents to a tea-themed pair:

- **Sinistea (Phony Form)**
- **Poltchageist (Counterfeit Form)**

Both are given at **Lv. 5**.

The corresponding overworld sprites were also changed to match the new gift Pokémon.

The menu was converted from index-based results to species-valued dynamic menu entries so the front-sprite preview callback could work directly.

Relevant files:

```text
data/maps/CherrygroveCity/map.json
data/maps/CherrygroveCity/scripts.pory
data/maps/CherrygroveCity/scripts.inc
```

Declining still follows the original decline/shame dialogue route.

---

## Violet City gift Pokémon

A one-time Pokémon choice was added to the Violet Academy / Violet City boy NPC.

Available choices:

- Sneasel
- Hisuian Sneasel
- Mienfoo
- Rufflet

All are given at **Lv. 5**.

The player may decline and return later. The completion flag is set only after taking a Pokémon.

The menu was converted to species-valued dynamic entries and uses the Pokémon front-sprite preview callback.

Relevant files:

```text
data/maps/VioletCity/scripts.pory
data/maps/VioletCity/scripts.inc
include/constants/flags.h
```

Current completion flag:

```c
#define FLAG_RECEIVED_VIOLET_ACADEMY_GIFT 0x103E
```

The history of this flag is important because earlier attempts to reuse apparently-unused legacy flags caused linker errors. See **Build/debug history** below.

---

## Rival team changes

### Silver

Silver's early teams were adjusted to establish a second Pokémon from the beginning.

Early progression:

- First team: starter + **Starly Lv. 5**
- Next stage: **Staravia Lv. 21** replaces the previous Pidgeotto slot

The Lv. 21 Staravia moveset includes:

- Quick Attack
- Wing Attack
- Feather Dance
- Helping Hand

Later Silver teams already progress through Staravia and Staraptor.

### Gold / Crystal friendly rival

The friendly rival is gender-dependent:

- Male player → Crystal
- Female player → Gold

Their early teams were expanded with the Mareep line.

First stage:

- Marill Lv. 4
- **Mareep Lv. 4**

Later stage includes:

- Zorua Lv. 22
- Oricorio-Sensu Lv. 22
- **Flaaffy Lv. 22**
- Azumarill

Flaaffy was inserted before Azumarill so Azumarill remains the last party member, preserving the intended interaction with Zorua's Illusion.

Relevant trainer changes are in:

```text
src/data/trainers.party
```

---

## Mega Stone shop

Elm's permanently-present female lab aide (`AIDE2`) becomes a Mega Stone seller once:

```text
VAR_NEWBARKTOWN_LABSTATE >= 8
```

This corresponds to the point after the Mega Ring / Elm Mega-related sequence.

Dialogue theme:

> Our experiments have helped us create some Mega Stones!  
> Would you like to buy one and help fund our research?

### Inventory

All 18 type-based Mega Stones are sold, alphabetically, followed by Bondstone at the bottom:

1. Bugtite
2. Darktite
3. Dragotite
4. Electrite
5. Fairytite
6. Fightite
7. Firetite
8. Flyingite
9. Ghostite
10. Grasstite
11. Groundite
12. Icetite
13. Normalite
14. Poisontite
15. Psychite
16. Rocktite
17. Steeltite
18. Watertite
19. Bondstone

Relevant files:

```text
data/maps/NewBarkTown_Lab/scripts.pory
data/maps/NewBarkTown_Lab/scripts.inc
src/data/items.h
```

### Mega Stone pricing

All 18 type Mega Stones and Bondstone cost:

- **₽60,000 each**

The type-based stones are defined through the `TYPE_MEGA_STONE` item macro; Bondstone has its own item definition.

### Mega Stone system finding

This SoulGold project primarily uses **type-generic Mega Stones**, not normal species-specific stones.

Examples:

- Swampert → Watertite
- Sceptile → Grasstite
- Ampharos → Electrite
- Heracross → Bugtite
- Tyranitar → Rocktite
- Mewtwo X → Fightite
- Mewtwo Y → Psychite

The original nine starter final evolutions all use **Bondstone**.

This was why an earlier idea to replace Elm's Bondstone reward with a giant stone selector was postponed; the lab shop now provides the broader type-stone access instead.

---

## Mart / restoration changes

### Ether availability

The project already contained some upstream Ether shop support, and an additional custom change on `dev` ensured Ether availability tracks the normal badge-scaled Mart progression.

Current intended behavior:

- Great Balls begin appearing after the first badge
- Ether is available from that same general tier onward
- Ether is placed at the **bottom of the purchasable item list** in the relevant badge-scaled inventories
- PC/PokéCenter alternate shop lists were normalized so Ether is also last where Great/Ultra Balls are available

Relevant file:

```text
src/shop.c
```

### Restoration-item prices

Current custom prices:

- Revive: **₽1,000**
- Max Revive: **₽2,000**
- Ether: **₽1,500**

These were halved from the then-current values:

- Revive: ₽2,000
- Max Revive: ₽4,000
- Ether: ₽3,000

Relevant file:

```text
src/data/items.h
```

For reference, current potion-family prices observed during this work were:

- Potion: ₽200
- Super Potion: ₽700
- Hyper Potion: ₽1,500
- Max Potion: ₽2,500
- Full Restore: ₽3,000

---

## Doubles-focused run

The overall design direction is a playthrough where battles are intended to be heavily or entirely doubles-focused.

Changes already made in support of that direction include:

- A second starter
- Early rival teams with at least two Pokémon
- Accessible PP restoration
- More generous catching resources
- Greater starting funds
- Broad Mega Stone access

However, a **global "all battles are doubles" mechanism has not yet been verified or implemented as part of these edits**.

Trainer entries such as Silver/Gold/Crystal still may contain normal single-battle flags. Before changing every trainer individually, the preferred next step is to inspect whether the engine has a global battle-format override.

---

## Build/debug history

### 1. Missing `FLAG_WONDERTRADE1`

First full Codespaces build reached the linker and failed with errors such as:

```text
undefined reference to `FLAG_WONDERTRADE1'
```

Affected Wonder Trade scripts included:

- `EventScript_DoWonderTrade`
- `EventScript_DoWonderTradeStart`
- `EventScript_WTChallenges`
- `EventScript_DoWonderTradeFirstTrade`
- `EventScript_DoWonderTradeCancelSaveFirstTrade`

#### Cause

When the Violet Academy gift was first added, flag `0x297` was repurposed as:

```c
FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
```

But `master` showed that `0x297` originally belonged to:

```c
FLAG_WONDERTRADE1
```

The symbol was therefore removed while compiled Wonder Trade script data still referenced it.

#### Fix

`FLAG_WONDERTRADE1` was restored to `0x297`.

The Violet gift flag was temporarily moved elsewhere.

---

### 2. Missing `FLAG_FRONTIER_SECOND_CLERK`

The next link attempt failed with:

```text
undefined reference to `FLAG_FRONTIER_SECOND_CLERK'
```

#### Cause

The first attempt to move the Violet gift flag reused `0x29B`, replacing:

```c
FLAG_FRONTIER_SECOND_CLERK
```

Although code search did not reveal a straightforward source reference, compiled script/data still required that symbol.

#### Fix

`FLAG_FRONTIER_SECOND_CLERK` was restored to `0x29B`.

The Violet gift was moved again.

---

### 3. Missing `FLAG_ITEM_ROUTE2_NUGGET`

The next link attempt failed with:

```text
undefined reference to `FLAG_ITEM_ROUTE2_NUGGET'
```

#### Cause

The Violet gift was temporarily moved to `0x46D`, replacing:

```c
FLAG_ITEM_ROUTE2_NUGGET
```

That flag was labeled in the header as an "Unused Flag, leftover from R/S", but compiled `.rodata` still referenced the symbol.

This demonstrated an important lesson: **a comment saying "unused" is not enough evidence that a named flag can safely be deleted or repurposed.**

#### Fix

`FLAG_ITEM_ROUTE2_NUGGET` was restored at `0x46D`.

The Violet gift was finally assigned to:

```c
FLAG_RECEIVED_VIOLET_ACADEMY_GIFT 0x103E
```

The custom-flag table had an actual numeric gap:

- `0x103D` already used
- **`0x103E` free**
- `0x103F` free
- `0x1040` free
- `0x1041` already used

This is a substantially safer place for new persistent custom flags.

### Flag-allocation lesson

For future additions, do not repurpose an existing named constant merely because it is commented as unused.

Preferred approach:

1. Check the flag table for a truly unassigned numeric slot.
2. Prefer the explicit custom-flag range for new hack-specific persistent state.
3. Keep legacy named constants intact unless all generated/compiled references have been conclusively checked.
4. A successful full link is an important validation step because source-code search alone may miss references generated into script/data objects.

---

## Dynamic Pokémon preview implementation notes

The new dynamic menu Pokémon-preview callback lives in:

```text
src/script_menu.c
```

It uses:

```c
CreateMonSprite_PicBoxShiny(...)
```

and destroys the previous sprite with:

```c
FreeResourcesAndDestroySprite(...)
```

The callback stores its auxiliary window ID and sprite ID in the dynamic menu scratchpad.

A theoretical edge case remains: `CreateMonSprite_PicBoxShiny` can potentially fail and return `MAX_SPRITES`; the implementation follows existing repo precedent and does not add an extra guard before dereferencing the new sprite. This has not caused a compile problem, but runtime menu testing is still worthwhile.

---

## Files currently differing from master

At the time this document was created, the custom `dev` branch differed from `master` in these main files:

```text
data/maps/CherrygroveCity/map.json
data/maps/CherrygroveCity/scripts.inc
data/maps/CherrygroveCity/scripts.pory
data/maps/NewBarkTown_Lab/scripts.inc
data/maps/NewBarkTown_Lab/scripts.pory
data/maps/VioletCity/scripts.inc
data/maps/VioletCity/scripts.pory
data/text/birch_speech.inc
include/constants/flags.h
include/constants/pokemon.h
include/constants/script_menu.h
src/battle_script_commands.c
src/data/items.h
src/data/trainers.party
src/main_menu.c
src/new_game.c
src/option_menu.c
src/pokemon.c
src/script_menu.c
src/shop.c
```

---

## Changes considered but not implemented

These ideas were discussed but intentionally left undone or still require investigation:

- Expand the initial custom 9-starter UI to all 27 starters
- Replace Elm's Bondstone reward with a Mega Stone selector
- Force every trainer/battle globally into doubles
- Confirm all Pokémon-preview menu layouts visually in an emulator
- Comprehensive runtime testing of all new gifts, shops, and rival encounters

The Mega Stone shop largely supersedes the need for an Elm Mega Stone selector.

---

## Suggested testing checklist

Now that the ROM compiles, useful runtime checks include:

### New game

- Starting money is ₽150,000
- Kid A / Kid B labels display correctly
- Initial 9-starter UI still works
- Second-starter selector appears after the first starter
- "No thanks!" is first
- Chosen first starter is absent from the second list
- Pokémon previews update correctly
- Declining does not give a Pokémon

### Early-game items

- First Poké Ball gift gives 90
- Catch rates feel substantially increased
- Shiny option/base rate behaves as intended

### Cherrygrove

- Sinistea and Poltchageist choices preview correctly
- Both gifts are Lv. 5
- Decline path still works

### Violet City

- Sneasel, Hisuian Sneasel, Mienfoo, and Rufflet preview correctly
- Gifts are Lv. 5
- Declining allows returning later
- Taking a Pokémon prevents taking another
- `FLAG_RECEIVED_VIOLET_ACADEMY_GIFT` behaves correctly at `0x103E`

### Rivals

- Silver has Starly in the first encounter
- Staravia appears in the intended next stage
- Gold/Crystal have Mareep initially and Flaaffy later
- Zorua Illusion still presents as Azumarill where intended

### Marts

- Ether appears in the desired Great Ball / later shop tiers
- Ether appears at the bottom of relevant shop lists
- Revive costs ₽1,000
- Max Revive costs ₽2,000
- Ether costs ₽1,500

### Mega Stones

- Female Elm aide remains normal before the Mega unlock
- At lab state 8+, she opens the Mega Stone shop
- 18 type stones appear alphabetically
- Bondstone appears last
- Every stone costs ₽60,000
- Elm's existing free Bondstone reward still works

### UI / stability

- Pokémon preview windows do not clip off-screen
- Repeated cursor movement does not leave old sprites behind
- Menus close cleanly before gift Pokémon picture sequences
- No obvious EWRAM/runtime instability occurs despite the high static EWRAM usage reported by the linker

---

## Git / build workflow notes

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

After a successful build:

```bash
ls -lh Soulgold.gba
```

The ROM is produced in the repository root as `Soulgold.gba`.


## Poké Vial (feature/pokevial)

Ported Pokémon World's Poké Vial mechanic to SoulGold. A refillable key item heals the entire party's HP, PP, and status (including fainted Pokémon), using one charge each time. It starts with five charges, refilled at every Pokémon Center, and retains support for a 15-charge capacity cap. Pokémon Center nurse healing refills the item and grants it on the first visit (including existing saves). A follow-up explanation appears only after the vial is successfully given: "Here's a vial to heal up when out on journeys. Refill it at Pokémon Centers!" The bag description uses a shorter line without "whole", and successful healing now reports remaining heals instead of doses (with singular grammar for one heal). Choosing Use in the Bag or through a registered shortcut first shows the number of remaining heals and asks for confirmation. Choosing No or pressing B does not consume a charge. Only a confirmed Yes heals the party. Existing two-charge saves are upgraded to five without losing track of doses already spent. Charges and capacity use permanent event variables `0x4122` and `0x4123` rather than modifying SaveBlock structures. The Powder Jar icon is temporarily reused instead of importing donor binary artwork.

**Test checklist:** Receive the item on the first nurse visit; heal HP/PP/status/fainting; use via Bag and registered shortcut; expend two charges and observe the empty message; refill at a nurse; save/reload; check that the stat editor still works. These PokéVial changes have not been compiled or playtested.


## Select follower (select-follower)

- Added a **Follow** action to the field party selection menu, third-last (immediately above **Edit stats** and **Cancel**) in all three party-menu styles: SWSH, HGSS, and BW. The HGSS and BW styles share the `src/party_menu.c` implementation; SWSH uses `src/swsh_party_menu.c`.
- Adapted Pokémon World's explicit follower selection: choose a non-Egg party Pokémon to follow regardless of whether it leads the party. Selecting the same Pokémon again returns to the automatic first-eligible follower.
- Saved the selected party slot in the previously unused `SaveBlock2` byte at offset `0x90`, retaining save-block size and older-save compatibility (zero means automatic). Fainted, Egg, absent, or otherwise ineligible selections fall back to the first eligible Pokémon without clearing the stored preference.
- When party members swap, the selected follower slot follows the chosen Pokémon. Followers continue respecting Soulgold's existing overworld rules, sprite limits, and Followers option.
- Extended the field-menu action buffers to fit the extra action. No changes to `dev` until the feature is compiled, tested, and explicitly merged.
