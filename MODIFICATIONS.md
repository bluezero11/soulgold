# SoulGold Modifications

This file documents the custom changes made on the `dev` branch of `bluezero11/soulgold`, plus important implementation findings and build/debugging notes discovered while compiling the project. It also records feature work developed on short-lived branches before merge.

The intention is to keep `master` as the untouched/reference branch, keep `dev` as the main custom gameplay branch, and isolate larger experimental features on dedicated branches until they compile and test cleanly.

## Source attribution and implementation lineage

These notes distinguish **ported**, **adapted/reference-based**, **already-present**, and **custom SoulGold** functionality. Inspiration does not imply a verbatim port.

| Feature | Donor / reference | How SoulGold uses it |
| --- | --- | --- |
| **Original EV/IV Stat Editor** | **Archie / TeamAquasHideout:** [original March 2024 stat-editor commit](https://github.com/TeamAquasHideout/pokeemerald/commit/317d345a33ae) and [installation tutorial](https://github.com/pret/pokeemerald/wiki/Add-a-EV---IV-Stat-Editor-UI). **Jaizu:** original editor graphics ([Team Aqua's feature credits](https://github.com/TeamAquasHideout/Team-Aquas-Asset-Repo/wiki/Feature-Branches)). | **Ported and adapted** editor controls, graphics and EV/IV editing for SoulGold's party menus, ability data and animation/graphics APIs. |
| **Expanded Stat Editor** | Archie's original editor foundation; nature/ability UX cross-checked against [PCG06/pokeemerald's `feat/stat_editor`](https://github.com/PCG06/pokeemerald/tree/feat/stat_editor) | **Reference-assisted and custom** nature/ability controls, gender and PID-preservation algorithm, staged changes, sprite preview and UI fixes. The gender algorithm is not attributed to PCG06. |
| **Poké Vial** | [evilchinesefood/PKMN-World](https://github.com/evilchinesefood/PKMN-World), [`src/pokevial.c`](https://github.com/evilchinesefood/PKMN-World/blob/master/src/pokevial.c) | **Adapted mechanic** with SoulGold event-variable charge storage, item confirmations, nurse refills and redemption. |
| **Repel Charm** | [pokeemerald-expansion Infinite Repel Item guide](https://github.com/rh-hideout/pokeemerald-expansion/wiki/Infinite-Repel-Item), contributed by `jtebbe` | **Adapted no-encounter flag and reusable item toggle**, with SoulGold-specific texts, art choice and reward script. |
| **Cheat-code foundation** | **Creative inspiration:** [Nemo622's Pokémon Lazarus](https://ko-fi.com/nemo622/shop), especially its in-game cheat-code rewards and gifts. **Implementation foundation:** existing [SoulGold `master` `src/field_specials.c`](https://github.com/bluezero11/soulgold/blob/master/src/field_specials.c), with `EnterCode`, `GetCodeFeedback`, naming-screen input and replay-option codes. | **Lazarus inspired the idea; the implementation is based on existing SoulGold code**, not imported or reverse-engineered from Lazarus. The original SNES was flavor text only. |
| **SNES cheats and gifts** | Inspired by [Nemo622's Pokémon Lazarus](https://ko-fi.com/nemo622/shop); independently implemented with existing SoulGold code entry and the user-curated `soulgold_stage1.xlsx` Pokémon pool | **Custom SoulGold work:** SNES script UI, `ROLLTHEDIE`/monotype selection, 3/6 gifting, cosmetic-form rolls, gift-to-PC handling, and `giveallqol` unlock/redeem logic. |
| **Pokémon Box Link** | Already implemented in SoulGold and the [pokeemerald-expansion Box Link feature](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/FEATURES.md) | **New distribution only:** reused existing item ID, field-use callback and storage script, adding code redemption and nurse catch-up; no Box Link mechanics ported. |

The chronological feature sections below describe SoulGold-specific modifications separately from the original authors' work. Crediting creative inspiration is not the same as attributing borrowed code.

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

The original EV/IV Stat Editor was developed by **Archie (TeamAquasHideout)**, with original graphics credited to **Jaizu**. SoulGold ports and adapts their work. Verified references: the [March 2024 implementation commit](https://github.com/TeamAquasHideout/pokeemerald/commit/317d345a33ae) and the [installation tutorial](https://github.com/pret/pokeemerald/wiki/Add-a-EV---IV-Stat-Editor-UI).

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

The imported EV/IV editor required changes because SoulGold has diverged from the older source's pokeemerald APIs.

Important compatibility fixes made during the transplant:

1. **Ability-info declaration**

   The imported editor's header declared:

   ```c
   extern const struct Ability gAbilitiesInfo[];
   ```

   SoulGold uses:

   ```c
   extern const struct AbilityInfo gAbilitiesInfo[];
   ```

   The declaration was updated accordingly. The incorrect donor declaration conflicted with SoulGold's `enum Ability` and caused cascading compile errors in party-menu code.

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

   The imported variant referenced `gScrollBgTiles`, `gScrollBgTilemap`, and `gScrollBgPalette` globals that SoulGold does not contain. This describes the imported variant's compatibility issue rather than attributing those symbols to the original author.

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

The opening character-selection question is now written as **one line**: "Which kid do you want to play as?" (33 characters, with no explicit `\\n` break). The two choice labels read:

- **Left one** (the character displayed on the left)
- **Right one** (the character displayed on the right)

This is a display/text change only; the underlying gender/player-character assignment and naming flow are unchanged. The follow-up name question uses the same updated labels. In `src/main_menu.c`, the choice window is widened from 6 to 11 tiles to fit "Right one", and the following window's VRAM base is moved from `0x85` to `0x99` to prevent the two window allocations from overlapping. The next window still ends at `0xF3`, leaving the existing menu-border tiles undisturbed.

Relevant files include:

```text
data/text/birch_speech.inc (character-selection question now one line)
src/main_menu.c (labels and window dimensions)
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

**Gift Sinistea (Phony Form)** is given with the exact moves **Astonish, Withdraw, Will-O-Wisp, and Confusion**.

**Gift Poltchageist (Counterfeit Form)** is given with the exact moves **Astonish, Withdraw, Stun Spore, and Grassy Terrain**. These gift-only moves do not alter Poltchageist's normal level-up learnset.

For regular level-up progression, **both Sinistea and Polteageist now learn Hex at level 30 instead of Sweet Scent**. This is a species-wide Gen 9 learnset adjustment and affects wild-caught Pokémon as well as the gift; Nasty Plot remains at level 42.


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

The sprite-preview dynamic menu displays five entries at a time and scrolls through the twelve species plus a **No thanks!** option. Selecting a species still gives only one Pokémon; the player may decline and return later. The completion flag is set only after taking a Pokémon.

After the shared `Common_EventScript_GiftMon` routine, every Violet gift restores `STR_VAR_1` from `VAR_TEMP_TRANSFERRED_SPECIES` before the final NPC dialogue. The shared PC-transfer routine overwrites `STR_VAR_1` with the destination box name when a full-party gift is deposited in the PC; restoring it prevents lines such as “Take good care of Box1!”. Cherrygrove's Sinistea and Poltchageist gift scripts already restore `STR_VAR_1` after the shared call and need no changes.

Each customized moveset is supplied directly to the existing `givemon` script macro through the `move1`–`move4` parameters. **Joltik and Skorupi** deliberately use the default `givemon` moveset instead. Regular Pokémon learnsets remain unchanged.

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
- The question "Which kid do you want to play as?" displays on a single line without clipping; Left one / Right one labels display correctly and select the corresponding left/right character
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


## Stat editor nature preview and ordering (feature/stat-editor-natures)

- **Nature stat colors:** The Stat Editor's **Actual** numbers now mirror the Pokémon Summary screen's convention: boosted stat in red, reduced stat in blue, neutral stats in charcoal. HP, actual stat totals, EVs, IVs and labels stay unchanged. Changes are visible immediately while scrolling because the preview already recomputes from staged `MON_DATA_HIDDEN_NATURE` (effective/Mint nature), matching the Summary screen's source of nature modifiers.
- **Palette integration:** Two new font styles use previously unused indices 11–14 in the Stat Editor's 16-color text palette (red/blue faces with pale shadows). No new graphics files, BG palettes or Pokémon data structures are required.
- **Grouped nature order:** Left/Right steps through these 25 nature IDs instead of the underlying enum order, sorted by boosted stat then reduced stat (Attack, Defense, Sp. Atk, Sp. Def, Speed). The sequence is:
  - **+Attack:** Lonely (-Def), Adamant (-SpA), Naughty (-SpD), Brave (-Speed)
  - **+Defense:** Bold (-Atk), Impish (-SpA), Lax (-SpD), Relaxed (-Speed)
  - **+Sp. Atk:** Modest (-Atk), Mild (-Def), Rash (-SpD), Quiet (-Speed)
  - **+Sp. Def:** Calm (-Atk), Gentle (-Def), Careful (-SpA), Sassy (-Speed)
  - **+Speed:** Timid (-Atk), Hasty (-Def), Jolly (-SpA), Naive (-SpD)
  - **Neutral, alphabetically:** Bashful, Docile, Hardy, Quirky, Serious
- **Controls and save behavior:** Nature wraps from the final neutral nature back to Lonely. Existing Up/Down first/last shortcuts are preserved, as are A-to-edit/A-to-stage, START confirmation, B cancellation and L/R party switching. Only display/selection order changes; existing game-wide nature IDs remain intact.

**Testing:** Compile and inspect colored boosted/reduced stats for several natures (including Speed and neutral natures); verify EV/IV columns and HP stay uncolored, nature cycling order/wrap and first/last shortcuts, save persistence, and UI appearance for both shiny and non-shiny Pokémon. This branch is not yet compiled or playtested.

## Expanded stat editor (feature/expanded-editor)

Built on the original EV/IV editor by Archie / TeamAquasHideout (graphics credited to Jaizu), with the nature/ability cycling approach cross-checked against [PCG06/pokeemerald's `feat/stat_editor` branch](https://github.com/PCG06/pokeemerald/tree/feat/stat_editor). Gender/PID-preservation and additional SoulGold UI fixes are not attributed as direct copies from that branch. This branch extends the existing SoulGold stat editor, preserving its graphics, original EV/IV grid, party switching, and START confirmation behavior.

- **Natural D-pad movement:** From the EV column, press Left to enter the left-hand details panel; press Right to return to EVs, then Right again for IVs. Up/Down cycles between **Gender**, **Ability**, and **Nature**, ordered top-to-bottom to match the UI. The new gender cursor brackets the Lv./gender row at y=113, with the working magenta arrow positions unchanged.
- **A-to-edit, A-to-stage:** Press A on a detail to enter edit mode, adjust with Left/Right (Up/Down jumps to first/last ability or nature; either toggles gender), and press A to return to selection mode. Gender, ability, nature, EV and IV changes remain staged until START or party switching.
- **Ability:** Cycles only through distinct, nonempty abilities actually available to that Pokémon's species/form. Editing is blocked if there is only one distinct ability.
- **Nature:** Uses `MON_DATA_HIDDEN_NATURE` (the effective/Mint nature) so stat adjustments change without altering the Pokémon's personality, gender, shininess or forms. Both the displayed nature and stat preview use the staged effective nature.
- **Save behavior:** START commits all staged EVs, IVs, ability and nature and recalculates stats. B from the main selection mode discards pending changes. Like the original editor, changing to another party member with L/R commits pending edits to the previous Pokémon.
- **Gender editing:** Male/female species can toggle gender; genderless and fixed-gender species have editing blocked. The editor calculates a staged 32-bit personality using a bounded, deterministic search: it preserves the original PID's **upper 16 bits** (including Wurmple's evolution branching), **natural nature** (PID % 25) and, whenever possible, the lower two bits of each byte (for Unown-letter logic). If an extreme custom gender ratio prevents preserving the lowest two bits of the bottom byte, the search relaxes that restriction but still preserves the upper half, original nature, and bits 8-9.
- **Preserved shiny/nature/type:** On START or party switching, SoulGold's existing `UpdateMonPersonality(&mon->box, personality)` updates the PID while preserving shiny status, effective/Mint nature and Tera type. Afterwards, staged nature and ability changes are applied. The gender icon updates in the info panel while editing, and on leaving edit mode the front sprite is reloaded to preview gender-dependent appearances without writing the staged changes.
- **Inherent PID limitation:** Because Spinda's spots derive directly from the full PID, changing Spinda's gender can also change its spot pattern. However, toggling back to the original gender restores the **exact original PID**, including original Spinda spots. Custom PID-based species/form mechanics beyond the preserved natural nature, upper PID and Unown-related bits warrant testing.
- **Sprite fidelity fix:** The front Pokémon sprite (and its shadow copy) are created using the current Pokémon's `MON_DATA_IS_SHINY` and complete `MON_DATA_PERSONALITY`, preserving shiny palettes and personality-based sprite variations.
- **Gender-only cursor alignment:** Fine-tuned after gameplay testing: the gender left arrow moves 1 pixel right to sprite center x=63 (visible approximately x=48–52) and the gender right arrow moves 2 pixels right to center x=57 (visible approximately x=68–72). The gender symbol is approximately x=57–63. Ability/Nature retain the verified centers x=19 and x=63, with no changes to graphics or vertical alignment.
- **Details cursor fix and PNG build repair:** A previously added 6-frame selector PNG contained an invalid PNG IDAT CRC and caused `tools/gbagfx` to fail during clean builds. Restored the original, verified four-frame selector PNG from `dev` and reused its existing single-arrow animations: index 2 for the left arrow (local pixels x=1–5), index 1 for the right arrow (local pixels x=27–31). With centers at x=19 and x=63, the arrows should appear at screen pixels x=4–8 and x=74–78, outside the detail labels beginning at x=12. Row centers remain y=130/146. EV/IV cursor graphics remain unchanged, and the shiny Pokémon sprite fix is retained.
- **Animation flip fix:** The Pokémon Summary animation engine can leave an old affine matrix index in `oam.matrixNum` when a sprite stops using affine rendering. In normal rendering these same bits serve as horizontal/vertical flip flags, sometimes leaving Pokémon sprites mirrored or upside down, especially after party switching. The SwSh party menu already works around this in `RunMonAnimTimer`. The stat editor now also sanitizes `oam.matrixNum` from each sprite's `hFlip`/`vFlip`, **only after the animation has finished and affine mode is off**. This runs after `AnimateSprites` and before `BuildOamBuffer` for both Pokémon and shadow, without affecting active animations.
- **ROM filename:** The Makefile's `FILE_NAME` is now `Soulgold-mod`, making a normal `make` output `Soulgold-mod.gba` (and correspondingly named ELF and map files). Existing older `Soulgold.gba` files may remain in the working directory and should not be mistaken for the newly built ROM.
- **Compatibility:** No new save fields, Pokémon data layout changes, or graphics. Changes in this feature branch are confined to `src/ui_stat_editor.c`, `Makefile`, and this documentation.

**Test plan:** Test both D-pad transitions from EV to details and back, A edit/stage and START save, nature stat-preview matching the in-game Summary screen, all three ability slots where present, hidden/duplicate/missing ability slots, one-ability species, genderless/single-gender species, high- and low-female-ratio Pokémon (including starters), shiny gender changes, natural and effective nature, Wurmple evolution, gender-dependent sprite preview, B discard, L/R party switching, save/reload, and existing EV/IV editing. The user has confirmed the arrow alignment, gender-specific sprites, gender restrictions and persistent edits in gameplay. The new animation flip fix has not yet been compiled or playtested: test entry animations and repeated L/R switches, especially during animations.

## Poké Vial (feature/pokevial)

Adapted [Pokémon World's Poké Vial](https://github.com/evilchinesefood/PKMN-World/blob/master/src/pokevial.c) to SoulGold. A refillable key item heals the entire party's HP, PP, and status (including fainted Pokémon), using one charge each time. It starts with five charges, refilled at every Pokémon Center, and retains support for a 15-charge capacity cap. Pokémon Center nurse healing refills existing vials. Originally it granted a vial automatically on the first visit; as of feature/repel-charm, a new vial is given at the first nurse visit *after* the `giveallqol` code is redeemed, while older saves with a vial keep theirs. A follow-up explanation appears only after the vial is successfully given: "Here's a vial to heal up when out on journeys. Refill it at Pokémon Centers!" The bag description uses a shorter line without "whole", and successful healing now reports remaining heals instead of doses (with singular grammar for one heal). Choosing Use in the Bag or through a registered shortcut first shows the number of remaining heals and asks for confirmation. Choosing No or pressing B does not consume a charge. Only a confirmed Yes heals the party. Existing two-charge saves are upgraded to five without losing track of doses already spent. Charges and capacity use permanent event variables `0x4122` and `0x4123` rather than modifying SaveBlock structures. The Powder Jar icon is temporarily reused instead of importing donor binary artwork.

**Test checklist (updated unlock behavior):** Redeem `giveallqol` to claim the vial on the next nurse visit; confirm new games without the code do not receive a vial and older saves retain/refill one; heal HP/PP/status/fainting; use via Bag and registered shortcut; expend two charges and observe the empty message; refill at a nurse; save/reload; check that the stat editor still works. These PokéVial changes have not been compiled or playtested.

## QOL unlock bundle / Repel Charm and Box Link (feature/repel-charm)

- **Creative inspiration:** Playing [Nemo622's Pokémon Lazarus](https://ko-fi.com/nemo622/shop) inspired the idea of using in-game cheat codes for useful tools and themed Pokémon gifts. SoulGold's code entry, SNES integration and reward logic were built from its own codebase; no Lazarus source code was ported.
- **SNES cheat:** After receiving a Pokémon, enter `giveallqol` (case-insensitive, ten characters) at the SNES in the upstairs New Bark Town bedroom. The former `waytoobusy` code no longer matches. On first redemption, a **single unlock-summary message** announces **Stat Editor, Poké Vial, Repel Charm, and Pokémon Box Link** before either item is awarded. The game's standard item-acquisition messages then appear for the Repel Charm followed by the Box Link. A final message explains that the **Poké Vial must be claimed from a Pokémon Center nurse**.
- **Redemption and retry:** `FLAG_QOLUNLOCK_REDEEMED = 0x103F` tracks the QOL bundle. The SNES checks `ITEM_REPEL_CHARM` and `ITEM_POKEMON_BOX_LINK` before awarding them, so either item can already exist in the Bag (including a Repel Charm from the older code). It sets the flag **only after both items are present** and then shows the Poké Vial collection reminder. The unlock summary appears before the item grants, so if a Bag-full failure interrupts redemption the player may see the summary again when retrying. If either `giveitem` fails due to bag space, it leaves the flag unset so the code can be retried; any item already received remains in the Bag. Re-entering after successful redemption normally says **You've already entered that cheat code.** The previous `waytoobusy` flag name remains removed. All three Wonder Trade flags have been preserved at `0x297` (`FLAG_WONDERTRADE1`), `0x298` (`FLAG_WONDERTRADE2`) and `0x299` (`FLAG_WONDERTRADE3`); each is still referenced by Wonder Trade scripts despite earlier misleading `Unused Flag` comments. QOL redemption instead uses the previously unassigned **persistent custom flag `0x103F`**, between `FLAG_RECEIVED_VIOLET_ACADEMY_GIFT = 0x103E` and `FLAG_PYRAMID_ACHIEVEMENT_MIGRATION_COMPLETE = 0x1041`. No migration from the temporary `0x298`/`0x299` QOL flag versions is implemented, because neither was redeemed on any saves.
- **Poké Vial:** The charm and Box Link are awarded at the SNES, but the Poké Vial is not. On the next Pokémon Center nurse heal after QOL redemption, a missing `ITEM_POKE_VIAL` is granted and its charges refilled. A failed nurse grant is retried later. Players who owned the vial before this change keep it and keep their nurse refills even if their old save has no QOL redemption flag. Persistent `VAR_POKEVIAL_*` charge/capacity storage and prior two-to-five-charge migration are unchanged. Adapted from [Pokémon World](https://github.com/evilchinesefood/PKMN-World/blob/master/src/pokevial.c), not a direct copy of its save format.
- **Stat Editor in all three party menus:** `src/party_menu.c` is shared by BW and HGSS; `src/swsh_party_menu.c` implements SwSh. Both show Edit Stats when `FLAG_QOLUNLOCK_REDEEMED` is set **or** when the save predates this QOL gating. New games set `FLAG_QOL_NEW_GAME_REQUIRES_UNLOCK` in `NewGameInitData` so they must redeem the code; preexisting saves lack that marker and retain the editor without returning to New Bark Town.
- **Repel Charm mechanics and credit:** Adapted from [jtebbe's Infinite Repel Item guide for pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion/wiki/Infinite-Repel-Item). The Key Item toggles all standard walking/surfing random encounters on and off indefinitely without a lead-level requirement. Fishing and deliberate/scripted encounters remain available. The saved `FLAG_REPEL_CHARM_ACTIVE` aliases `FLAG_SYS_NO_ENCOUNTER` (0x21), used by `OW_FLAG_NO_ENCOUNTER`. The item reuses Oval Charm artwork for now and works from the Bag or a registered shortcut.
- **Pokémon Box Link mechanics and source:** The existing `ITEM_POKEMON_BOX_LINK = 719` is a non-consumable Key Item with its own graphics, `ItemUseOutOfBattle_PokemonBoxLink` field callback, and `EventScript_AccessPokemonBoxLink` storage script. This branch **does not** port new PC/storage logic or alter its existing usage restrictions. It only adds distribution to the QOL bundle. Preexisting Box Link ownership is respected. The existing functionality is also described in [pokeemerald-expansion's feature list](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/FEATURES.md).
- **Already-redeemed save catch-up:** A save whose `FLAG_QOLUNLOCK_REDEEMED` was set by an older build but which lacks the Box Link automatically receives it from a Pokémon Center nurse on the next successful healing interaction. No return to the SNES is needed, despite the repeat-code message. The nurse checks the Bag to avoid duplication and retries if a gift fails. A new game without the code does not receive Box Link at a nurse. The existing nurse script already handled delayed Poké Vial distribution, so both checks run during the same heal.
- **Save compatibility:** No changes to SaveBlock structs, Pokémon formats, item IDs, or item-use implementations. Existing vials, Repel Charms and Box Links remain in the Bag. Old saves retain Stat Editor access. New redemptions and upgrades use previously allocated persistent flags and the existing item slots. The temporary `0x298` and `0x299` QOL flags were never redeemed on any saves, so no migration is necessary; the final, isolated QOL flag is `0x103F`.
- **Modified source files:** QOL redemption in `data/maps/NewBarkTown_PlayersHouse_2F/scripts.inc`; nurse delivery in `data/scripts/pkmn_center_nurse.inc`; `src/field_specials.c`, `src/new_game.c`, `src/party_menu.c`, `src/swsh_party_menu.c`, and Repel Charm item/flag/config files. Box Link's existing `src/data/items.h`, `src/item_use.c`, and `data/scripts/pc.inc` implementations were **not changed** to add this reward.

**Future QOL rewards:** `FLAG_QOLUNLOCK_REDEEMED` is a durable feature entitlement, not automatically a delivery service. New gated UIs should check the flag, while new Key Items need explicit Bag checks and distribution/migration on a suitable interaction or save load. This release's Box Link nurse catch-up is an example: existing players get a later-added item without re-entering the code, even though the SNES still says it was already redeemed.

**Wonder Trade linker fix and unused-flag audit:** The sequential linker errors for `FLAG_WONDERTRADE1`, `FLAG_WONDERTRADE2`, then `FLAG_WONDERTRADE3` revealed that all three are used by the DoWonderTrade scripts. A placeholder annotated `Unused Flag` is not reliable evidence of nonuse; do not repurpose `0x297`–`0x299`. Both `0x103F` and `0x1040` were unassigned in the existing persistent custom-flag block (`CUSTOM_FLAGS_END = 0x1041`), and `0x103F` was allocated exclusively to QOL redemption. Before reusing a flag in future, check defined numeric IDs, references in compiled scripts and source, and the lifetime of its flag region (temporary/daily/system/custom).

**Build and linker regression test:** After pulling this change in Codespaces, run `make clean` followed by `make -j$(nproc)` (or the existing build target) so previously generated event-script objects cannot retain references to undefined symbols. Confirm there are no errors for **any of `FLAG_WONDERTRADE1`, `FLAG_WONDERTRADE2`, or `FLAG_WONDERTRADE3`**, and that Wonder Trade works. The branch was not compiled by this documentation update.

**Test checklist (not yet compiled or playtested):** Start a new game without the code: nurse should not give either item and Edit Stats should be hidden in BW, HGSS, and SwSh menus. Redeem `giveallqol` and confirm one unlock summary before the item-grant animations/messages, Repel Charm then Box Link acquisition, and a final nurse/Poké Vial reminder, Box Link storage access via Bag/registered use, nurse delivery of Poké Vial, its refills, and Repel Charm on/off behavior. Repeat the code; verify no duplicate items. Test full Key Items pocket while giving either gift and retry behavior. Load an older save already carrying a Poké Vial or Repel Charm: confirm existing items and Stat Editor remain available. Test an already-redeemed QOL save missing Box Link: nurse should grant it once without an SNES visit. Check save/reload persistence and ordinary random, fishing, and scripted encounters.

## ROLLTHEDIE batch type diversity (feature/repel-charm)

- **Scope:** `ROLLTHEDIE3` and `ROLLTHEDIE6` now require every Pokémon in that code's batch to have **no type in common** with any earlier selection. Both the primary and secondary types in the curated `sRandomizerMons[].typeMask` count, including both types of dual-type Pokémon. Example: after a Dragon/Water Tatsugiri, no later Pokémon can have Dragon or Water as either type.
- **Unaffected codes:** A single `ROLLTHEDIE` still picks one random ticket as before. `MONO...`, `MONO...3`, and `MONO...6` remain based on their requested shared type and only prevent duplicate master-list tickets; they do **not** disallow type overlap.
- **Selection:** `SetRandomizerCheatGift` in `src/field_specials.c` now accepts an explicit `disallowSharedTypes` argument, enabled only for multi-Pokémon `ROLLTHEDIE` codes. It maintains a cumulative bitmask of chosen types and removes incompatible tickets before each subsequent random choice. Each remaining compatible ticket is sampled uniformly without replacement. The current pool contains 292 tickets across all 18 types, with at least one single-type ticket for each type, so batches of three and six remain completable. Selection is uniform among choices *at each step*, not uniformly across every possible valid complete combination.
- **Forms and delivery:** Shellos, Flabébé, and Tatsugiri still use one master-list ticket each and roll cosmetic forms only **after** the eligible ticket is selected, so forms do not affect species odds or type filtering. All species are preselected before the SNES gift script starts; if an edited future pool cannot complete a batch, the code fails rather than delivering a partial batch. Gift-to-party/PC messages, levels, code parsing and per-batch EWRAM storage are unchanged.
- **Testing (not yet compiled or playtested):** Redeem `ROLLTHEDIE3` and `ROLLTHEDIE6` repeatedly, including cases with dual-type picks, and verify every pair in a batch shares zero types and each gift arrives. Verify `ROLLTHEDIE`, `MONOFIRE3`, `MONOWATER6` and other `MONO...` codes retain their original rules; cosmetic forms have normal odds; gift-to-PC still works with a full party; and batches finish at exactly three or six Pokémon.
