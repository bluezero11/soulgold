// Stat editor adapted from fakuzatsu/verdant (pokeemerald-expansion based).
// Original feature transplanted for SoulGold's feature/stat-editor branch.
#include "global.h"
#include "ui_stat_editor.h"
#include "strings.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "event_data.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "item.h"
#include "item_menu.h"
#include "item_menu_icons.h"
#include "list_menu.h"
#include "item_icon.h"
#include "item_use.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "palette.h"
#include "party_menu.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "overworld.h"
#include "event_data.h"
#include "constants/items.h"
#include "constants/field_weather.h"
#include "constants/songs.h"
#include "constants/rgb.h"
#include "pokemon_icon.h"
#include "pokedex.h"
#include "trainer_pokemon_sprites.h"
#include "field_effect.h"
#include "field_screen_effect.h"

/*
 * 
 */
 
//==========DEFINES==========//
struct StatEditorResources
{
    MainCallback savedCallback; // determines callback to run when we exit. e.g. where do we want to go after closing the menu
    u8 sBg1TilemapBuffer[BG_SCREEN_SIZE];
    u8 gfxLoadState;
    u8 mode;
    u8 monIconSpriteId;
    u8 monIconShadowId;
    u16 speciesID;
    u16 selectedStat;
    u16 selectorSpriteId;
    u8 detailRightSelectorId;
    u16 selector_x;
    u16 selector_y;
    u32 editingStat;
    u16 normalTotal;
    u16 evTotal;
    u16 ivTotal;
    u16 partyid;
    u16 inputMode;
    u8 selectedPanel;
    u8 selectedDetail;
    u8 newNature;
    u8 newAbilitySlot;
    u32 newPersonality;
    u8 newEVs[6];
    u8 newIVs[6];
    bool8 monAnimPlayed;
};

#define INPUT_SELECT_STAT 0
#define INPUT_EDIT_STAT 1

// The existing stat grid and the lower-left info panel share one cursor.
// Left from the EV column enters info; Right returns to the stat grid.
#define PANEL_STATS   0
#define PANEL_DETAILS 1
// Rows reflect their physical top-to-bottom position in WINDOW_3.
#define DETAIL_GENDER  0
#define DETAIL_ABILITY 1
#define DETAIL_NATURE  2
#define DETAIL_COUNT   3

#define UI_STAT_HP 0
#define UI_STAT_ATK 1
#define UI_STAT_DEF 2
#define UI_STAT_SPE 3
#define UI_STAT_SPATK 4
#define UI_STAT_SPDEF 5

#define TAG_MON_SHADOW 30010

static const u16 sMonShadowPalette[] = INCBIN_U16("graphics/summary_screen/bw/shadow.gbapal");

static const struct SpritePalette sSpritePal_MonShadow =
{
    sMonShadowPalette, TAG_MON_SHADOW
};

enum WindowIds
{
    WINDOW_1,
    WINDOW_2,
    WINDOW_3,
    WINDOW_4,
};

enum GetOrSetStat
{
    GET_STAT,
    SET_STAT,
};

//==========EWRAM==========//
static EWRAM_DATA struct StatEditorResources *sStatEditorDataPtr = NULL;

//==========STATIC=DEFINES==========//
static void StatEditor_RunSetup(void);
static bool8 StatEditor_DoGfxSetup(void);
static void StatEditor_SanitizeMonSpriteOam(void);
static bool8 StatEditor_InitBgs(void);
static void StatEditor_FadeAndBail(void);
static bool8 StatEditor_LoadGraphics(void);
static void StatEditor_InitWindows(void);
static void PrintTitleToWindowMainState();
static void Task_StatEditorWaitFadeIn(u8 taskId);
static void Task_StatEditorMain(u8 taskId);
static void Task_MenuEditingStat(u8 taskId);
static void SampleUi_DrawMonIcon(u16 dexNum);
static void PrintMonStats(void);
static void SelectorCallback(struct Sprite *sprite);
static void DetailRightSelectorCallback(struct Sprite *sprite);
static inline struct Pokemon *ReturnPartyMon();
static void ChangeAndUpdateStats(void);
static void ChangePartyPokemon(u8 taskId, s8 direction);
static void RefreshStatEditorMonSprite(void);
static void Task_MenuEditingDetail(u8 taskId);
static void ChangeDetailValue(bool8 increase, bool8 jumpToEnd);
static u8 GetValidAbilitySlots(u16 species, u8 *slots);
static bool8 CanEditGender(u16 species);
static bool8 FindGenderPersonality(u16 species, u32 original, u8 gender, u32 *result);
static u8 CreateSelector(void);
static void DestroySelector(void);
static void SpriteCB_Pokemon(struct Sprite *);

//==========CONST=DATA==========//
static const struct BgTemplate sStatEditorBgTemplates[] =
{
    {
        .bg = 0, // windows, etc
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .priority = 1
    }, 
    {
        .bg = 1, // this bg loads the UI tilemap
        .charBaseIndex = 3,
        .mapBaseIndex = 28,
        .priority = 2
    },
    {
        .bg = 2, // this bg loads the UI tilemap
        .charBaseIndex = 0,
        .mapBaseIndex = 26,
        .priority = 0
    },
    {
        .bg = 3, // scrolling bg
        .charBaseIndex = 2,
        .mapBaseIndex = 29,
        .priority = 2
    }
};

static const struct WindowTemplate sMenuWindowTemplates[] = 
{
    [WINDOW_1] = 
    {
        .bg = 0,          // which bg to print text on
        .tilemapLeft = 1, // position from left (per 8 pixels)
        .tilemapTop = 0,  // position from top (per 8 pixels)
        .width = 30,      // width (per 8 pixels)
        .height = 2,      // height (per 8 pixels)
        .paletteNum = 2,  // palette index to use for text
        .baseBlock = 1,   // tile start in VRAM
    },
    [WINDOW_2] = 
    {
        .bg = 0,             // which bg to print text on
        .tilemapLeft = 11,   // position from left (per 8 pixels)
        .tilemapTop = 2,     // position from top (per 8 pixels)
        .width = 18,         // width (per 8 pixels)
        .height = 17,        // height (per 8 pixels)
        .paletteNum = 2,     // palette index to use for text
        .baseBlock = 1 + 70, // tile start in VRAM
    },
    [WINDOW_3] = 
    {
        .bg = 0,                   // which bg to print text on
        .tilemapLeft = 1,          // position from left (per 8 pixels)
        .tilemapTop = 11,          // position from top (per 8 pixels)
        .width = 8,                // width (per 8 pixels)
        .height = 9,               // height (per 8 pixels)
        .paletteNum = 2,           // palette index to use for text
        .baseBlock = 1 + 70 + 306, // tile start in VRAM
    },
    DUMMY_WIN_TEMPLATE
};

static const u32 sStatEditorBgTiles[]   = INCBIN_U32("graphics/ui_menu/background_tileset.4bpp.smol");
static const u32 sStatEditorBgTilemap[] = INCBIN_U32("graphics/ui_menu/background_tileset.bin.lz");
static const u16 sStatEditorBgPalette[] = INCBIN_U16("graphics/ui_menu/background_pal.gbapal");

enum Colors
{
    FONT_BLACK,
    FONT_WHITE,
};

static const u16 sStatEditorTextPalette[] =
{
    RGB(0, 0, 0),       // transparent
    RGB(29, 29, 28),    // soft off-white
    RGB(29, 29, 28),    // light button lettering
    RGB(5, 5, 6),       // dark charcoal
    RGB(12, 22, 31),    // male symbol
    RGB(5, 12, 20),     // male shadow
    RGB(31, 12, 20),    // female symbol
    RGB(20, 6, 12),     // female shadow
    RGB(29, 5, 22),     // magenta button outline
    RGB(24, 25, 27),    // light grey shadow for dark text
    RGB(8, 8, 9),       // dark shadow for light text
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
};

static const u8 sMenuWindowFontColors[][3] = 
{
    [FONT_BLACK]  = {TEXT_COLOR_TRANSPARENT, 3, 9},
    [FONT_WHITE]  = {TEXT_COLOR_TRANSPARENT, 1, 10},
};

#define TAG_SELECTOR 30004

static const u16 sSelector_Pal[]   = INCBIN_U16("graphics/ui_menu/selector.gbapal");
static const u32 sSelector_Gfx[]   = INCBIN_U32("graphics/ui_menu/selector.4bpp.smol");
static const u8 sA_ButtonGfx[]     = INCBIN_U8("graphics/ui_menu/a_button.4bpp");
static const u8 sB_ButtonGfx[]     = INCBIN_U8("graphics/ui_menu/b_button.4bpp");
static const u8 sStart_ButtonGfx[] = INCBIN_U8("graphics/ui_menu/r_button.4bpp");
static const u8 sDPad_ButtonGfx[]  = INCBIN_U8("graphics/ui_menu/dpad_button.4bpp");

static const struct OamData sOamData_Selector =
{
    .size = SPRITE_SIZE(32x32),
    .shape = SPRITE_SHAPE(32x32),
    .priority = 0,
};

static const struct CompressedSpriteSheet sSpriteSheet_Selector =
{
    .data = sSelector_Gfx,
    .size = 32*32*4/2,
    .tag = TAG_SELECTOR,
};

static const struct SpritePalette sSpritePal_Selector =
{
    .data = sSelector_Pal,
    .tag = TAG_SELECTOR
};

static const union AnimCmd sSpriteAnim_Selector0[] =
{
    ANIMCMD_FRAME(0, 32),
    ANIMCMD_FRAME(0, 32),
    //ANIMCMD_FRAME(48, 10),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sSpriteAnim_Selector1[] =
{
    ANIMCMD_FRAME(32, 32),
    ANIMCMD_FRAME(32, 32),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sSpriteAnim_Selector2[] =
{
    ANIMCMD_FRAME(16, 32),
    ANIMCMD_FRAME(16, 32),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sSpriteAnim_Selector3[] =
{
    ANIMCMD_FRAME(0, 32),
    ANIMCMD_FRAME(0, 32),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sSpriteAnimTable_Selector[] =
{
    sSpriteAnim_Selector0,
    sSpriteAnim_Selector1,
    sSpriteAnim_Selector2,
    sSpriteAnim_Selector3,
};

static const struct SpriteTemplate sSpriteTemplate_Selector =
{
    .tileTag = TAG_SELECTOR,
    .paletteTag = TAG_SELECTOR,
    .oam = &sOamData_Selector,
    .anims = sSpriteAnimTable_Selector,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SelectorCallback
};

// Begin Generic UI Initialization Code
void Task_OpenStatEditorFromStartMenu(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        StatEditor_Init(CB2_ReturnToFieldWithOpenMenu, gSpecialVar_0x8004);
        DestroyTask(taskId);
    }
}

// This is our main initialization function if you want to call the menu from elsewhere
void StatEditor_Init(MainCallback callback, u16 partyIndex)
{
    u32 i;

    if ((sStatEditorDataPtr = AllocZeroed(sizeof(struct StatEditorResources))) == NULL)
    {
        SetMainCallback2(callback);
        return;
    }
    
    // initialize stuff
    sStatEditorDataPtr->gfxLoadState = 0;
    sStatEditorDataPtr->savedCallback = callback;
    sStatEditorDataPtr->selectorSpriteId = 0xFF;
    sStatEditorDataPtr->detailRightSelectorId = 0xFF;
    sStatEditorDataPtr->partyid = partyIndex;

    for (i = 0; i < 6; i++)
        sStatEditorDataPtr->newEVs[i] = GetMonData(ReturnPartyMon(), MON_DATA_HP_EV + i);

    for (i = 0; i < 6; i++)
        sStatEditorDataPtr->newIVs[i] = GetMonData(ReturnPartyMon(), MON_DATA_HP_IV + i);

    sStatEditorDataPtr->selectedPanel = PANEL_STATS;
    sStatEditorDataPtr->newNature = GetMonData(ReturnPartyMon(), MON_DATA_HIDDEN_NATURE);
    sStatEditorDataPtr->newAbilitySlot = GetMonData(ReturnPartyMon(), MON_DATA_ABILITY_NUM);
    sStatEditorDataPtr->newPersonality = GetMonData(ReturnPartyMon(), MON_DATA_PERSONALITY);

    SetMainCallback2(StatEditor_RunSetup);
}

static void StatEditor_RunSetup(void)
{
    while (1)
    {
        if (StatEditor_DoGfxSetup() == TRUE)
            break;
    }
}

// Pokémon summary animations can leave a freed affine matrix index in
// oam.matrixNum. Once affine mode is off, bits 3/4 become OAM flip flags
// and can leave the sprite randomly mirrored or upside-down. SoulGold's
// SwSh party menu already fixes this in RunMonAnimTimer.
static void StatEditor_SanitizeMonSpriteOam(void)
{
    u8 i;
    u8 spriteIds[2] = {
        sStatEditorDataPtr->monIconSpriteId,
        sStatEditorDataPtr->monIconShadowId
    };

    for (i = 0; i < ARRAY_COUNT(spriteIds); i++)
    {
        u8 spriteId = spriteIds[i];

        if (spriteId < MAX_SPRITES
         && gSprites[spriteId].inUse
         && gSprites[spriteId].callback == SpriteCallbackDummy
         && gSprites[spriteId].oam.affineMode == ST_OAM_AFFINE_OFF)
        {
            gSprites[spriteId].oam.matrixNum =
                (gSprites[spriteId].hFlip << 3) | (gSprites[spriteId].vFlip << 4);
        }
    }
}

static void StatEditor_MainCB(void)
{
    RunTasks();
    AnimateSprites();
    // Sanitize only after sprite callbacks and before rendering. RunTasks
    // can exit the editor and release its resources.
    if (sStatEditorDataPtr != NULL)
        StatEditor_SanitizeMonSpriteOam();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void StatEditor_VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static bool8 StatEditor_DoGfxSetup(void)
{
    switch (gMain.state)
    {
    case 0:
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        SetVBlankHBlankCallbacksToNull();
        ResetVramOamAndBgCntRegs();
        ClearScheduledBgCopiesToVram();
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        ResetSpriteData();
        ResetTasks();
        gMain.state++;
        break;
    case 2:
        if (StatEditor_InitBgs())
        {
            sStatEditorDataPtr->gfxLoadState = 0;
            gMain.state++;
        }
        else
        {
            StatEditor_FadeAndBail();
            return TRUE;
        }
        break;
    case 3:
        if (StatEditor_LoadGraphics() == TRUE)
            gMain.state++;
        break;
    case 4:
        sStatEditorDataPtr->speciesID = GetMonData(ReturnPartyMon(), MON_DATA_SPECIES);
        FreeMonIconPalettes();
        LoadMonIconPalettes();
        LoadCompressedSpriteSheet(&sSpriteSheet_Selector);
        LoadSpritePalette(&sSpritePal_Selector);
        SampleUi_DrawMonIcon(sStatEditorDataPtr->speciesID);
        gMain.state++;
        break;
    case 5:
        StatEditor_InitWindows();
        PrintTitleToWindowMainState();
        sStatEditorDataPtr->inputMode = INPUT_SELECT_STAT;
        PrintMonStats();
        CreateSelector();
        gMain.state++;
        break;
    case 6:
        CreateTask(Task_StatEditorWaitFadeIn, 0);
        BlendPalettes(0xFFFFFFFF, 16, RGB_BLACK);
        gMain.state++;
        break;
    case 7:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    default:
        SetVBlankCallback(StatEditor_VBlankCB);
        SetMainCallback2(StatEditor_MainCB);
        return TRUE;
    }
    return FALSE;
}

#define try_free(ptr) ({        \
    void ** ptr__ = (void **)&(ptr);   \
    if (*ptr__ != NULL)                \
        Free(*ptr__);                  \
})

#define POKEMON_SPRITE sStatEditorDataPtr->monIconSpriteId
#define SHADOW_SPRITE sStatEditorDataPtr->monIconShadowId

static void DestroyStatEditorMonSprite(u8 spriteId)
{
    // The foreground pic uses palette slot 0, reserved by LoadMonIconPalettes.
    // FreeResourcesAndDestroySprite would release that reservation, allowing the
    // shadow palette to claim slot 0 and turn the next Pokemon into a silhouette.
    FreeSpriteOamMatrix(&gSprites[spriteId]);
    FreeAndDestroyMonPicSpriteNoPalette(spriteId);
}

static void StatEditor_FreeResources(void)
{
    DestroySelector();
    DestroyStatEditorMonSprite(POKEMON_SPRITE);
    FreeResourcesAndDestroySprite(&gSprites[SHADOW_SPRITE], SHADOW_SPRITE);
    try_free(sStatEditorDataPtr);
    FreeAllWindowBuffers();
}


static void Task_StatEditorWaitFadeAndBail(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(sStatEditorDataPtr->savedCallback);
        StatEditor_FreeResources();
        DestroyTask(taskId);
    }
}

static void StatEditor_FadeAndBail(void)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_StatEditorWaitFadeAndBail, 0);
    SetVBlankCallback(StatEditor_VBlankCB);
    SetMainCallback2(StatEditor_MainCB);
}

static bool8 StatEditor_InitBgs(void)
{
    ResetAllBgsCoordinates();
    memset(sStatEditorDataPtr->sBg1TilemapBuffer, 0, sizeof(sStatEditorDataPtr->sBg1TilemapBuffer));
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sStatEditorBgTemplates, NELEMS(sStatEditorBgTemplates));
    SetBgTilemapBuffer(1, sStatEditorDataPtr->sBg1TilemapBuffer);
    ScheduleBgCopyTilemapToVram(1);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG3 | BLDCNT_TGT2_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT1_BG2 | BLDCNT_TGT1_BG1);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(14, 6));
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
    return TRUE;
}

static bool8 StatEditor_LoadGraphics(void)
{
    switch (sStatEditorDataPtr->gfxLoadState)
    {
    case 0:
        ResetTempTileDataBuffers();
        DecompressAndCopyTileDataToVram(1, sStatEditorBgTiles, 0, 0, 0);
        sStatEditorDataPtr->gfxLoadState++;
        break;
    case 1:
        if (FreeTempTileDataBuffersIfPossible() != TRUE)
        {
            DecompressDataWithHeaderWram(sStatEditorBgTilemap, sStatEditorDataPtr->sBg1TilemapBuffer);
            sStatEditorDataPtr->gfxLoadState++;
        }
        break;
    case 2:
        LoadPalette(sStatEditorBgPalette, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
        LoadPalette(sStatEditorTextPalette, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
        sStatEditorDataPtr->gfxLoadState++;
        break;
    default:
        sStatEditorDataPtr->gfxLoadState = 0;
        return TRUE;
    }
    return FALSE;
}

static void StatEditor_InitWindows(void)
{
    InitWindows(sMenuWindowTemplates);
    DeactivateAllTextPrinters();
    ScheduleBgCopyTilemapToVram(0);
    
    FillWindowPixelBuffer(WINDOW_1, 0);
    PutWindowTilemap(WINDOW_1);
    CopyWindowToVram(WINDOW_1, 3);
    
    ScheduleBgCopyTilemapToVram(2);
}

static void Task_StatEditorWaitFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_StatEditorMain;
}

static void Task_StatEditorTurnOff(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(sStatEditorDataPtr->savedCallback);
        StatEditor_FreeResources();
        DestroyTask(taskId);
    }
}

//
//       Stat Editor Code
//  End of UI setup code, beginning of stat editor specific code
//
#define sSpecies data[0]
#define sDontFlip data[1]
#define sDelayAnim data[2]
#define sIsShadow data[3]

static inline struct Pokemon *ReturnPartyMon()
{
    return &gPlayerParty[sStatEditorDataPtr->partyid];
}

#define MON_ICON_X     32 + 8
#define MON_ICON_Y     32 + 24

static void SampleUi_DrawMonIcon(u16 dexNum)
{
    u8 shadowPalette = 0;
    u16 speciesId = dexNum;
    // Match the real Pokémon's palette and PID (including shiny and gender
    // differences), rather than always drawing the normal palette and dummy PID.
    bool8 isShiny = GetMonData(ReturnPartyMon(), MON_DATA_IS_SHINY);
    u32 personality = sStatEditorDataPtr->newPersonality;

    POKEMON_SPRITE = CreateMonPicSprite_Affine(speciesId, isShiny, personality, TRUE, MON_ICON_X, MON_ICON_Y, 0, TAG_NONE);

    gSprites[POKEMON_SPRITE].oam.priority = 0;
    gSprites[POKEMON_SPRITE].sSpecies = sStatEditorDataPtr->speciesID;
    gSprites[POKEMON_SPRITE].callback = SpriteCB_Pokemon;
    gSprites[POKEMON_SPRITE].sIsShadow = FALSE;
    gSprites[POKEMON_SPRITE].sDelayAnim = 0;

    SHADOW_SPRITE = CreateMonPicSprite_Affine(speciesId, isShiny, personality, TRUE, MON_ICON_X + 5, MON_ICON_Y - 2, 0, TAG_NONE);

    gSprites[SHADOW_SPRITE].oam.priority = 1;
    gSprites[SHADOW_SPRITE].sSpecies = sStatEditorDataPtr->speciesID;
    gSprites[SHADOW_SPRITE].callback = SpriteCB_Pokemon;
    gSprites[SHADOW_SPRITE].sIsShadow = TRUE;
    gSprites[SHADOW_SPRITE].sDelayAnim = 0;

    FreeSpritePaletteByTag(TAG_MON_SHADOW); // reload the palette entirely because some sprite anims modify it
    shadowPalette = LoadSpritePalette(&sSpritePal_MonShadow);
    gSprites[SHADOW_SPRITE].oam.paletteNum = shadowPalette;
    gSprites[SHADOW_SPRITE].oam.objMode = ST_OAM_OBJ_BLEND;
}

#undef POKEMON_SPRITE
#undef SHADOW_SPRITE

// Preview gender-dependent front sprites only after staging the edit.
// Reuse the resource lifecycle from existing party switching.
static void RefreshStatEditorMonSprite(void)
{
    StopPokemonAnimationDelayTask();
    DestroyStatEditorMonSprite(sStatEditorDataPtr->monIconSpriteId);
    FreeResourcesAndDestroySprite(&gSprites[sStatEditorDataPtr->monIconShadowId],
                                  sStatEditorDataPtr->monIconShadowId);
    sStatEditorDataPtr->monAnimPlayed = TRUE;
    SampleUi_DrawMonIcon(sStatEditorDataPtr->speciesID);
}

static u8 CreateSelector(void)
{
    if (sStatEditorDataPtr->selectorSpriteId == 0xFF)
        sStatEditorDataPtr->selectorSpriteId = CreateSprite(&sSpriteTemplate_Selector, 188, 30, 0);

    gSprites[sStatEditorDataPtr->selectorSpriteId].invisible = FALSE;
    StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 0);

    // Reuse the original four-frame selector PNG, which is known-good.
    // Animation 2 shows only the left arrow (frame at tile offset 16);
    // animation 1 shows only the right arrow (frame at offset 32).
    // This avoids importing extra graphics or modifying EV/IV cursors.
    if (sStatEditorDataPtr->detailRightSelectorId == 0xFF)
    {
        u8 spriteId = CreateSprite(&sSpriteTemplate_Selector, 63, 130, 0);
        if (spriteId < MAX_SPRITES)
        {
            sStatEditorDataPtr->detailRightSelectorId = spriteId;
            gSprites[spriteId].callback = DetailRightSelectorCallback;
            gSprites[spriteId].invisible = TRUE;
            StartSpriteAnim(&gSprites[spriteId], 1);
        }
    }

    DebugPrintf("Sprite ID: %d", sStatEditorDataPtr->selectorSpriteId);
    return sStatEditorDataPtr->selectorSpriteId;
}

static void DestroySelector(void)
{
    if (sStatEditorDataPtr->detailRightSelectorId != 0xFF)
        DestroySprite(&gSprites[sStatEditorDataPtr->detailRightSelectorId]);
    sStatEditorDataPtr->detailRightSelectorId = 0xFF;

    if (sStatEditorDataPtr->selectorSpriteId != 0xFF)
        DestroySprite(&gSprites[sStatEditorDataPtr->selectorSpriteId]);
    sStatEditorDataPtr->selectorSpriteId = 0xFF;
}

#define DISTANCE_BETWEEN_STATS_Y 16
#define SECOND_COLUMN ((8 * 4) + 10)
#define THIRD_COLUMN ((8 * 8) + 10)
#define STARTING_X 50
#define STARTING_Y 26

struct MonPrintData {
    u16 x;
    u16 y;
};

static const struct MonPrintData StatPrintData[] =
{
    [MON_DATA_MAX_HP] = {STARTING_X, STARTING_Y},
    [MON_DATA_HP_EV] = {STARTING_X + SECOND_COLUMN, STARTING_Y},
    [MON_DATA_HP_IV] = {STARTING_X + THIRD_COLUMN, STARTING_Y},

    [MON_DATA_ATK] = {STARTING_X, STARTING_Y + DISTANCE_BETWEEN_STATS_Y},
    [MON_DATA_ATK_EV] = {STARTING_X + SECOND_COLUMN, STARTING_Y + DISTANCE_BETWEEN_STATS_Y},
    [MON_DATA_ATK_IV] = {STARTING_X + THIRD_COLUMN, STARTING_Y + DISTANCE_BETWEEN_STATS_Y},

    [MON_DATA_DEF] = {STARTING_X, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 2)},
    [MON_DATA_DEF_EV] = {STARTING_X + SECOND_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 2)},
    [MON_DATA_DEF_IV] = {STARTING_X + THIRD_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 2)},

    [MON_DATA_SPATK] = {STARTING_X, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 3)},
    [MON_DATA_SPATK_EV] = {STARTING_X + SECOND_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 3)},
    [MON_DATA_SPATK_IV] = {STARTING_X + THIRD_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 3)},

    [MON_DATA_SPDEF] = {STARTING_X, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 4)},
    [MON_DATA_SPDEF_EV] = {STARTING_X + SECOND_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 4)},
    [MON_DATA_SPDEF_IV] = {STARTING_X + THIRD_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 4)},

    [MON_DATA_SPEED] = {STARTING_X, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 5)},
    [MON_DATA_SPEED_EV] = {STARTING_X + SECOND_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 5)},
    [MON_DATA_SPEED_IV] = {STARTING_X + THIRD_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 5)},
};

static const u16 statsToPrintActual[] = {
        MON_DATA_MAX_HP, MON_DATA_ATK, MON_DATA_DEF, MON_DATA_SPEED, MON_DATA_SPATK, MON_DATA_SPDEF,
};

static const u16 statsToPrintEVs[] = {
        MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV, MON_DATA_SPEED_EV, MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV,
};

static const u16 statsToPrintIVs[] = {
        MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV, MON_DATA_SPEED_IV, MON_DATA_SPATK_IV, MON_DATA_SPDEF_IV,
};

static const u8 sGenderColors[2][3] =
{
    {TEXT_COLOR_TRANSPARENT, 4, 5},
    {TEXT_COLOR_TRANSPARENT, 6, 7}
};

static const u8 sText_MenuTitle[]     = _("Stat Editor");
static const u8 sText_MenuHP[]        = _("HP");
static const u8 sText_MenuAttack[]    = _("Attack");
static const u8 sText_MenuSpAttack[]  = _("Sp. Atk");
static const u8 sText_MenuDefense[]   = _("Defense");
static const u8 sText_MenuSpDefense[] = _("Sp. Def");
static const u8 sText_MenuSpeed[]     = _("Speed");
static const u8 sText_MenuTotal[]     = _("Total");
static const u8 sText_MenuStat[]      = _("Stat");
static const u8 sText_MenuActual[]    = _("Actual");
static const u8 sText_MenuEV[]        = _("EV");
static const u8 sText_MenuIV[]        = _("IV");
static const u8 sText_MonLevel[]      = _("Lv.{CLEAR 1}{STR_VAR_1}");

static const u8 sText_MenuStartButtonTextMain[] = _("Confirm");
static const u8 sText_MenuAButtonTextMain[]     = _("Edit Stats");
static const u8 sText_MenuAButtonTextDetail[]   = _("Edit Info");
static const u8 sText_MenuBButtonTextMain[]     = _("Back");
static const u8 sText_MenuDPadButtonTextMain[]  = _("Change Stat");
static const u8 sText_MenuDPadButtonTextDetail[] = _("Change");

#define BUTTON_Y 4
static void PrintTitleToWindowMainState()
{
    FillWindowPixelBuffer(WINDOW_1, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    
    AddTextPrinterParameterized4(WINDOW_1, FONT_NORMAL, 1, 0, 0, 0, sMenuWindowFontColors[FONT_WHITE], TEXT_SKIP_DRAW, sText_MenuTitle);

    BlitBitmapToWindow(WINDOW_1, sA_ButtonGfx, 75, (BUTTON_Y), 8, 8);
    AddTextPrinterParameterized4(WINDOW_1, FONT_NARROW, 87, 0, 0, 0, sMenuWindowFontColors[FONT_WHITE], TEXT_SKIP_DRAW,
                                 sStatEditorDataPtr->selectedPanel == PANEL_DETAILS ? sText_MenuAButtonTextDetail : sText_MenuAButtonTextMain);

    BlitBitmapToWindow(WINDOW_1, sStart_ButtonGfx, 140, (BUTTON_Y), 24, 8);
    AddTextPrinterParameterized4(WINDOW_1, FONT_NARROW, 168, 0, 0, 0, sMenuWindowFontColors[FONT_WHITE], TEXT_SKIP_DRAW, sText_MenuStartButtonTextMain);

    PutWindowTilemap(WINDOW_1);
    CopyWindowToVram(WINDOW_1, 3);
}

static void PrintTitleToWindowEditState()
{
    FillWindowPixelBuffer(WINDOW_1, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    
    AddTextPrinterParameterized4(WINDOW_1, FONT_NORMAL, 1, 0, 0, 0, sMenuWindowFontColors[FONT_WHITE], TEXT_SKIP_DRAW, sText_MenuTitle);

    BlitBitmapToWindow(WINDOW_1, sDPad_ButtonGfx, 75, (BUTTON_Y), 24, 8);
    AddTextPrinterParameterized4(WINDOW_1, FONT_NARROW, 102, 0, 0, 0, sMenuWindowFontColors[FONT_WHITE], TEXT_SKIP_DRAW,
                                 sStatEditorDataPtr->selectedPanel == PANEL_DETAILS ? sText_MenuDPadButtonTextDetail : sText_MenuDPadButtonTextMain);

    BlitBitmapToWindow(WINDOW_1, sB_ButtonGfx, 160, (BUTTON_Y), 8, 8);
    AddTextPrinterParameterized4(WINDOW_1, FONT_NARROW, 172, 0, 0, 0, sMenuWindowFontColors[FONT_WHITE], TEXT_SKIP_DRAW, sText_MenuBButtonTextMain);

    PutWindowTilemap(WINDOW_1);
    CopyWindowToVram(WINDOW_1, 3);
}

#define CALC_HP(baseStat, iv, ev) \
    (n = (((2 * (baseStat) + (iv) + (ev) / 4) * level) / 100) + level + 10)

#define CALC_STAT(baseStat, iv, ev, statIndex) \
    (n = (((2 * (baseStat) + (iv) + (ev) / 4) * level) / 100) + 5, \
     n = ModifyStatByNature(nature, n, statIndex))

static inline u32 CalculateStatFromNewEVIVs(u32 i, u16 species, u16 level, u8 nature)
{
    s32 n;

    switch (i)
    {
    case 0:
        if (species == SPECIES_SHEDINJA)
            return 1;
        CALC_HP(gSpeciesInfo[species].baseHP, sStatEditorDataPtr->newIVs[i], sStatEditorDataPtr->newEVs[i]);
        return n;
    case 1:
        CALC_STAT(gSpeciesInfo[species].baseAttack, sStatEditorDataPtr->newIVs[i], sStatEditorDataPtr->newEVs[i], STAT_ATK);
        return n;
    case 2:
        CALC_STAT(gSpeciesInfo[species].baseDefense, sStatEditorDataPtr->newIVs[i], sStatEditorDataPtr->newEVs[i], STAT_DEF);
        return n;
    case 3:
        CALC_STAT(gSpeciesInfo[species].baseSpeed, sStatEditorDataPtr->newIVs[i], sStatEditorDataPtr->newEVs[i], STAT_SPEED);
        return n;
    case 4:
        CALC_STAT(gSpeciesInfo[species].baseSpAttack, sStatEditorDataPtr->newIVs[i], sStatEditorDataPtr->newEVs[i], STAT_SPATK);
        return n;
    case 5:
        CALC_STAT(gSpeciesInfo[species].baseSpDefense, sStatEditorDataPtr->newIVs[i], sStatEditorDataPtr->newEVs[i], STAT_SPDEF);
        return n;
    default:
        return 0;
    }
}

static void PrintMonStats()
{
    u32 i;
    u16 currentStat;
    u8 text[2];
    u16 level = GetMonData(ReturnPartyMon(), MON_DATA_LEVEL);
    // Preview staged gender without mutating party data.
    u8 gender = GetGenderFromSpeciesAndPersonality(sStatEditorDataPtr->speciesID,
                                                   sStatEditorDataPtr->newPersonality);
    u16 ability = GetAbilityBySpecies(sStatEditorDataPtr->speciesID, sStatEditorDataPtr->newAbilitySlot);
    // This is the Mint/effective nature actually used by CalculateMonStats,
    // unlike GetNature(), which reads the original personality-based nature.
    u8 nature = sStatEditorDataPtr->newNature;

    FillWindowPixelBuffer(WINDOW_2, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    FillWindowPixelBuffer(WINDOW_3, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    sStatEditorDataPtr->normalTotal = 0;
    sStatEditorDataPtr->evTotal = 0;
    sStatEditorDataPtr->ivTotal = 0;

    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, 14, 7, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_MenuStat);
    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, STARTING_X - 2, 7, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_MenuActual);
    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, STARTING_X + SECOND_COLUMN + 4, 7, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_MenuEV);
    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, STARTING_X + THIRD_COLUMN + 5, 7, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_MenuIV);

    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, 20, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 0), 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, sText_MenuHP);
    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, 8,  STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 1), 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, sText_MenuAttack);
    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, 8,  STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 2), 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, sText_MenuDefense);
    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, 6,  STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 3), 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, sText_MenuSpAttack);
    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, 8,  STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 4), 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, sText_MenuSpDefense);
    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, 12, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 5), 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, sText_MenuSpeed);
    AddTextPrinterParameterized4(WINDOW_2, FONT_NARROW, 12, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 6), 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, sText_MenuTotal);

    // Print Mon Stats
    for(i = 0; i < 6; i++)
    {
        currentStat = CalculateStatFromNewEVIVs(i, sStatEditorDataPtr->speciesID, level, nature);
        sStatEditorDataPtr->normalTotal += currentStat;
        DebugPrintf("Stat: %d", currentStat);
        ConvertIntToDecimalStringN(gStringVar2, currentStat, STR_CONV_MODE_RIGHT_ALIGN, 3);
        AddTextPrinterParameterized4(WINDOW_2, 1, StatPrintData[statsToPrintActual[i]].x, StatPrintData[statsToPrintActual[i]].y, 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, gStringVar2);
    }

    for(i = 0; i < 6; i++)
    {
        currentStat = sStatEditorDataPtr->newEVs[i];
        sStatEditorDataPtr->evTotal += currentStat;
        DebugPrintf("Stat: %d", currentStat);
        ConvertIntToDecimalStringN(gStringVar2, currentStat, STR_CONV_MODE_RIGHT_ALIGN, 3);
        AddTextPrinterParameterized4(WINDOW_2, 1, StatPrintData[statsToPrintEVs[i]].x, StatPrintData[statsToPrintEVs[i]].y, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar2);
    }

    for(i = 0; i < 6; i++)
    {
        currentStat = sStatEditorDataPtr->newIVs[i];
        sStatEditorDataPtr->ivTotal += currentStat;
        DebugPrintf("Stat: %d", currentStat);
        ConvertIntToDecimalStringN(gStringVar2, currentStat, STR_CONV_MODE_RIGHT_ALIGN, 3);
        AddTextPrinterParameterized4(WINDOW_2, 1, StatPrintData[statsToPrintIVs[i]].x, StatPrintData[statsToPrintIVs[i]].y, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar2);
    }

    // Calc Totals
    ConvertIntToDecimalStringN(gStringVar2, sStatEditorDataPtr->normalTotal, STR_CONV_MODE_RIGHT_ALIGN, 4);
    AddTextPrinterParameterized4(WINDOW_2, 1, STARTING_X - 6, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 6), 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, gStringVar2);

    ConvertIntToDecimalStringN(gStringVar2, sStatEditorDataPtr->evTotal, STR_CONV_MODE_RIGHT_ALIGN, 3);
    AddTextPrinterParameterized4(WINDOW_2, 1, STARTING_X + SECOND_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 6), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar2);

    ConvertIntToDecimalStringN(gStringVar2, sStatEditorDataPtr->ivTotal, STR_CONV_MODE_RIGHT_ALIGN, 3);
    AddTextPrinterParameterized4(WINDOW_2, 1, STARTING_X + THIRD_COLUMN, STARTING_Y + (DISTANCE_BETWEEN_STATS_Y * 6), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar2);

    // Print ability / nature / name / level / gender
    StringCopy(gStringVar2, GetSpeciesName(sStatEditorDataPtr->speciesID));

    AddTextPrinterParameterized4(WINDOW_3, FONT_NARROW, 4, 2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar2);

    ConvertIntToDecimalStringN(gStringVar1, level, STR_CONV_MODE_RIGHT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar2, sText_MonLevel);
    AddTextPrinterParameterized4(WINDOW_3, FONT_SMALL_NARROW, 4, 18, 0, 0, sMenuWindowFontColors[FONT_WHITE], TEXT_SKIP_DRAW, gStringVar2);

    StringCopy(text, gText_MaleSymbol);
    if (gender != MON_GENDERLESS)
    {
        if (gender == MON_FEMALE)
            StringCopy(text, gText_FemaleSymbol);
        else
            StringCopy(text, gText_MaleSymbol);
        AddTextPrinterParameterized4(WINDOW_3, FONT_NORMAL, 41 + 8, 19, 0, 0, sGenderColors[(gender == MON_FEMALE)], TEXT_SKIP_DRAW, text);
    }

    StringCopy(gStringVar2, gNaturesInfo[nature].name);
    AddTextPrinterParameterized4(WINDOW_3, FONT_SMALL_NARROW, 4, 50, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar2);

    StringCopy(gStringVar2, gAbilitiesInfo[ability].name);
    AddTextPrinterParameterized4(WINDOW_3, FONT_SMALL_NARROW, 4, 34, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar2);

    PutWindowTilemap(WINDOW_3);
    CopyWindowToVram(WINDOW_3, 3);

    PutWindowTilemap(WINDOW_2);
    CopyWindowToVram(WINDOW_2, 3);
}

struct SpriteCordsStruct {
    u8 x;
    u8 y;
};

static void SelectorCallback(struct Sprite *sprite)
{
    struct SpriteCordsStruct spriteCords[6][2] = {
        {{188, 30 + 20}, {220, 30 + 20}},
        {{188, 46 + 20}, {220, 46 + 20}},
        {{188, 62 + 20}, {220, 62 + 20}},
        {{188, 78 + 20}, {220, 78 + 20}},
        {{188, 94 + 20}, {220, 94 + 20}},
        {{188, 110 + 20}, {220, 110 + 20}}, // Thanks Jaizu
    };

    if(sStatEditorDataPtr->inputMode == INPUT_EDIT_STAT)
    {
        if(sprite->data[0] == 32)
        {
            sprite->invisible = TRUE;
        }
        if(sprite->data[0] >= 48)
        {
            sprite->invisible = FALSE;
            sprite->data[0] = 0;
        }
        sprite->data[0]++;
    }
    else
    {
        sprite->invisible = FALSE;
        sprite->data[0] = 0;
    }

    if (sStatEditorDataPtr->selectedPanel == PANEL_DETAILS)
    {
        // Gender: move the arrow 2px left (visible at x=47..51),
        // clear of the level text and close to the gender symbol.
        // Ability/Nature retain their existing x=4..8 position.
        sprite->x = (sStatEditorDataPtr->selectedDetail == DETAIL_GENDER) ? 62 : 19;
        sprite->y = (sStatEditorDataPtr->selectedDetail == DETAIL_GENDER) ? 113
                  : (sStatEditorDataPtr->selectedDetail == DETAIL_ABILITY) ? 130 : 146;
    }
    else
    {
        sStatEditorDataPtr->selectedStat = sStatEditorDataPtr->selector_x + (sStatEditorDataPtr->selector_y * 2);
        sprite->x = spriteCords[sStatEditorDataPtr->selector_y][sStatEditorDataPtr->selector_x].x;
        sprite->y = spriteCords[sStatEditorDataPtr->selector_y][sStatEditorDataPtr->selector_x].y;
    }
}

// Keep the second bracket arrow in sync with the original cursor's position
// and edit-mode blinking, and hide it whenever the stat grid is selected.
static void DetailRightSelectorCallback(struct Sprite *sprite)
{
    // Gender: move the arrow 2px left (visible at x=66..70),
    // matching the left arrow's shift around the gender symbol.
    // Ability/Nature retain their existing x=74..78 position.
    sprite->x = (sStatEditorDataPtr->selectedDetail == DETAIL_GENDER) ? 55 : 63;
    sprite->y = (sStatEditorDataPtr->selectedDetail == DETAIL_GENDER) ? 113
              : (sStatEditorDataPtr->selectedDetail == DETAIL_ABILITY) ? 130 : 146;
    sprite->invisible = sStatEditorDataPtr->selectedPanel != PANEL_DETAILS
                     || gSprites[sStatEditorDataPtr->selectorSpriteId].invisible;
}

static const u16 selectedStatToStatEnum[] = {
        MON_DATA_HP_EV, MON_DATA_HP_IV, MON_DATA_ATK_EV, MON_DATA_ATK_IV, MON_DATA_DEF_EV, MON_DATA_DEF_IV,
        MON_DATA_SPATK_EV, MON_DATA_SPATK_IV, MON_DATA_SPDEF_EV, MON_DATA_SPDEF_IV, MON_DATA_SPEED_EV, MON_DATA_SPEED_IV,
};

static void Task_DelayedSpriteLoad(u8 taskId) // wait 4 frames after changing the mon you're editing so there are no palette problems
{   
    if (gTasks[taskId].data[11] >= 4)
    {
        SampleUi_DrawMonIcon(sStatEditorDataPtr->speciesID);
        PrintMonStats();
        gTasks[taskId].func = Task_StatEditorMain;
        return;
    }
    else
    {
        gTasks[taskId].data[11]++;
    }
}

static void ChangePartyPokemon(u8 taskId, s8 direction)
{
    u8 i;
    u8 nextPartyId = sStatEditorDataPtr->partyid;

    // Keep edits made to the current Pokémon before moving to another one.
    ChangeAndUpdateStats();

    for (i = 0; i < gPlayerPartyCount; i++)
    {
        if (direction > 0)
            nextPartyId = (nextPartyId + 1) % gPlayerPartyCount;
        else
            nextPartyId = (nextPartyId == 0) ? gPlayerPartyCount - 1 : nextPartyId - 1;

        if (GetMonData(&gPlayerParty[nextPartyId], MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(&gPlayerParty[nextPartyId], MON_DATA_IS_EGG))
            break;
    }

    // If there is no other editable Pokémon, stay on the current one.
    if (nextPartyId == sStatEditorDataPtr->partyid)
        return;

    StopPokemonAnimationDelayTask();

    DestroyStatEditorMonSprite(sStatEditorDataPtr->monIconSpriteId);
    FreeResourcesAndDestroySprite(&gSprites[sStatEditorDataPtr->monIconShadowId], sStatEditorDataPtr->monIconShadowId);

    sStatEditorDataPtr->partyid = nextPartyId;
    sStatEditorDataPtr->speciesID = GetMonData(ReturnPartyMon(), MON_DATA_SPECIES);
    sStatEditorDataPtr->monAnimPlayed = FALSE;

    for (i = 0; i < 6; i++)
        sStatEditorDataPtr->newEVs[i] = GetMonData(ReturnPartyMon(), MON_DATA_HP_EV + i);

    for (i = 0; i < 6; i++)
        sStatEditorDataPtr->newIVs[i] = GetMonData(ReturnPartyMon(), MON_DATA_HP_IV + i);

    sStatEditorDataPtr->newNature = GetMonData(ReturnPartyMon(), MON_DATA_HIDDEN_NATURE);
    sStatEditorDataPtr->newAbilitySlot = GetMonData(ReturnPartyMon(), MON_DATA_ABILITY_NUM);
    sStatEditorDataPtr->newPersonality = GetMonData(ReturnPartyMon(), MON_DATA_PERSONALITY);
    sStatEditorDataPtr->inputMode = INPUT_SELECT_STAT;
    StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId],
                    sStatEditorDataPtr->selectedPanel == PANEL_DETAILS ? 2 : 0);
    PrintTitleToWindowMainState();

    gTasks[taskId].func = Task_DelayedSpriteLoad;
    gTasks[taskId].data[11] = 0;
}

#define GETSETEV(stat) \
    if (getOrSet == GET_STAT) \
        sStatEditorDataPtr->editingStat = sStatEditorDataPtr->newEVs[stat]; \
    else \
        sStatEditorDataPtr->newEVs[stat] = sStatEditorDataPtr->editingStat;

#define GETSETIV(stat) \
    if (getOrSet == GET_STAT) \
        sStatEditorDataPtr->editingStat = sStatEditorDataPtr->newIVs[stat]; \
    else \
        sStatEditorDataPtr->newIVs[stat] = sStatEditorDataPtr->editingStat;

static void ConvertEnumToFromArray(u16 selectedStat, enum GetOrSetStat getOrSet)
{
    switch(selectedStatToStatEnum[selectedStat])
    {
    case MON_DATA_HP_EV:
        GETSETEV(UI_STAT_HP);
        break;
    case MON_DATA_HP_IV:
        GETSETIV(UI_STAT_HP)
        break;
    case MON_DATA_ATK_EV:
        GETSETEV(UI_STAT_ATK)
        break;
    case MON_DATA_ATK_IV:
        GETSETIV(UI_STAT_ATK)
        break;
    case MON_DATA_DEF_EV:
        GETSETEV(UI_STAT_DEF)
        break;
    case MON_DATA_DEF_IV:
        GETSETIV(UI_STAT_DEF)
        break;
    case MON_DATA_SPEED_EV:
        GETSETEV(UI_STAT_SPE)
        break;
    case MON_DATA_SPEED_IV:
        GETSETIV(UI_STAT_SPE)
        break;
    case MON_DATA_SPATK_EV:
        GETSETEV(UI_STAT_SPATK)
        break;
    case MON_DATA_SPATK_IV:
        GETSETIV(UI_STAT_SPATK)
        break;
    case MON_DATA_SPDEF_EV:
        GETSETEV(UI_STAT_SPDEF)
        break;
    case MON_DATA_SPDEF_IV:
        GETSETIV(UI_STAT_SPDEF)
        break;
    }
}

#undef GETSETEV
#undef GETSETIV

static void Task_StatEditorMain(u8 taskId) // input control when first loaded into menu
{
    if (JOY_NEW(L_BUTTON))
    {
        PlaySE(SE_SELECT);
        ChangePartyPokemon(taskId, -1);
        return;
    }
    if (JOY_NEW(R_BUTTON))
    {
        PlaySE(SE_SELECT);
        ChangePartyPokemon(taskId, 1);
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        if (sStatEditorDataPtr->selectedPanel == PANEL_DETAILS)
        {
            u8 slots[NUM_ABILITY_SLOTS];

            // Don't offer gender editing on fixed/genderless species or
            // ability editing when the species has just one distinct ability.
            if ((sStatEditorDataPtr->selectedDetail == DETAIL_GENDER
              && !CanEditGender(sStatEditorDataPtr->speciesID))
             || (sStatEditorDataPtr->selectedDetail == DETAIL_ABILITY
              && GetValidAbilitySlots(sStatEditorDataPtr->speciesID, slots) <= 1))
            {
                PlaySE(SE_FAILURE);
                return;
            }

            PlaySE(SE_SELECT);
            StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 2);
            sStatEditorDataPtr->inputMode = INPUT_EDIT_STAT;
            PrintTitleToWindowEditState();
            gTasks[taskId].func = Task_MenuEditingDetail;
            return;
        }

        ConvertEnumToFromArray(sStatEditorDataPtr->selectedStat, GET_STAT);
        StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 3);
        PlaySE(SE_SELECT);
        PrintTitleToWindowEditState();
        sStatEditorDataPtr->inputMode = INPUT_EDIT_STAT;
        gTasks[taskId].func = Task_MenuEditingStat;
        if(sStatEditorDataPtr->editingStat == 0)
            StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 1);
        if((sStatEditorDataPtr->editingStat == 255 || (sStatEditorDataPtr->evTotal == 510)) && (sStatEditorDataPtr->selector_x == 0))
            StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 2);
        if((sStatEditorDataPtr->editingStat == 31) && (sStatEditorDataPtr->selector_x == 1))
            StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 2);
        return;
    }
    if (JOY_NEW(START_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        ChangeAndUpdateStats();
        gTasks[taskId].func = Task_StatEditorTurnOff;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_StatEditorTurnOff;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (sStatEditorDataPtr->selectedPanel == PANEL_STATS)
        {
            if (sStatEditorDataPtr->selector_x == 0)
            {
                sStatEditorDataPtr->selectedPanel = PANEL_DETAILS;
                // Enter by whichever info line is nearer the selected stat.
                sStatEditorDataPtr->selectedDetail = sStatEditorDataPtr->selector_y >= 4 ? DETAIL_NATURE
                                                     : sStatEditorDataPtr->selector_y >= 2 ? DETAIL_ABILITY
                                                     : DETAIL_GENDER;
                StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 2);
                PrintTitleToWindowMainState();
            }
            else
                sStatEditorDataPtr->selector_x = 0;
        }
        return;
    }
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (sStatEditorDataPtr->selectedPanel == PANEL_DETAILS)
        {
            sStatEditorDataPtr->selectedPanel = PANEL_STATS;
            sStatEditorDataPtr->selector_x = 0;
            StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 0);
            PrintTitleToWindowMainState();
        }
        else
            sStatEditorDataPtr->selector_x = 1;
        return;
    }
    if (JOY_NEW(DPAD_UP))
    {
        if (sStatEditorDataPtr->selectedPanel == PANEL_DETAILS)
            sStatEditorDataPtr->selectedDetail = (sStatEditorDataPtr->selectedDetail + DETAIL_COUNT - 1) % DETAIL_COUNT;
        else
            sStatEditorDataPtr->selector_y = (sStatEditorDataPtr->selector_y + 5) % 6;
        return;
    }
    if (JOY_NEW(DPAD_DOWN))
    {
        if (sStatEditorDataPtr->selectedPanel == PANEL_DETAILS)
            sStatEditorDataPtr->selectedDetail = (sStatEditorDataPtr->selectedDetail + 1) % DETAIL_COUNT;
        else
            sStatEditorDataPtr->selector_y = (sStatEditorDataPtr->selector_y + 1) % 6;
    }

}

static void ChangeAndUpdateStats(void)
{
    u32 i;

    for (i = 0; i < 6; i++)
        SetMonData(ReturnPartyMon(), MON_DATA_HP_EV + i, &(sStatEditorDataPtr->newEVs[i]));
    
    for (i = 0; i < 6; i++)
        SetMonData(ReturnPartyMon(), MON_DATA_HP_IV + i, &(sStatEditorDataPtr->newIVs[i]));

    if (sStatEditorDataPtr->newPersonality != GetMonData(ReturnPartyMon(), MON_DATA_PERSONALITY))
    {
        // SoulGold's existing helper preserves shiny state, effective
        // nature and Tera type when the personality changes.
        UpdateMonPersonality(&ReturnPartyMon()->box, sStatEditorDataPtr->newPersonality);
    }

    SetMonData(ReturnPartyMon(), MON_DATA_HIDDEN_NATURE, &sStatEditorDataPtr->newNature);
    SetMonData(ReturnPartyMon(), MON_DATA_ABILITY_NUM, &sStatEditorDataPtr->newAbilitySlot);
    CalculateMonStats(ReturnPartyMon());
}

// Ability and nature edits use the same stage/confirm flow as EV/IV edits.
// Only offered ability slots are used; missing and duplicate abilities are skipped.
static u8 GetValidAbilitySlots(u16 species, u8 *slots)
{
    u8 slot, i, count = 0;

    for (slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
    {
        enum Ability ability = GetSpeciesAbility(species, slot);
        bool8 duplicate = FALSE;

        if (ability == ABILITY_NONE)
            continue;

        for (i = 0; i < count; i++)
        {
            if (GetSpeciesAbility(species, slots[i]) == ability)
            {
                duplicate = TRUE;
                break;
            }
        }

        if (!duplicate)
            slots[count++] = slot;
    }
    return count;
}

// Gender depends on PID's bottom byte. Choose the closest replacement
// preserving (a) upper 16 bits used by Wurmple and other evolutions,
// (b) original natural nature (PID % 25) and (c), if possible, the
// lowest two bits of every byte used by Unown's letter calculation.
// Search is deterministic and bounded; no RNG or extra save data.
static bool8 CanEditGender(u16 species)
{
    u8 ratio = gSpeciesInfo[species].genderRatio;

    // Party menu integrations can expose eggs; never edit their PID.
    if (GetMonData(ReturnPartyMon(), MON_DATA_IS_EGG))
        return FALSE;

    return ratio != MON_MALE && ratio != MON_FEMALE && ratio != MON_GENDERLESS;
}

static bool8 FindGenderPersonality(u16 species, u32 original, u8 gender, u32 *result)
{
    u32 lowByte, midByte, candidate, difference;
    u32 originalNature = GetNatureFromPersonality(original);
    u32 upperHalf = original & 0xFFFF0000;
    u32 bestDifference = 0xFFFFFFFF;
    u8 ratio = gSpeciesInfo[species].genderRatio;
    u8 pass;
    bool8 found = FALSE;

    if (!CanEditGender(species))
        return FALSE;

    for (pass = 0; pass < 2; pass++)
    {
        // Prefer retaining PID bits 0-1. Pass 1 relaxes only those
        // bits for unusual gender ratios with no matching candidate.
        u32 start = (pass == 0) ? (original & 3) : 0;
        u32 step = (pass == 0) ? 4 : 1;

        for (lowByte = start; lowByte < 256; lowByte += step)
        {
            if ((lowByte < ratio) != (gender == MON_FEMALE))
                continue;

            // Preserve bits 8-9 and all of the upper 16 bits.
            for (midByte = (original >> 8) & 3; midByte < 256; midByte += 4)
            {
                candidate = upperHalf | (midByte << 8) | lowByte;
                if (GetNatureFromPersonality(candidate) != originalNature)
                    continue;

                difference = (candidate > original) ? candidate - original : original - candidate;
                if (difference < bestDifference)
                {
                    bestDifference = difference;
                    *result = candidate;
                    found = TRUE;
                }
            }
        }
        if (found)
            break;
    }

    return found;
}

static void ChangeDetailValue(bool8 increase, bool8 jumpToEnd)
{
    if (sStatEditorDataPtr->selectedDetail == DETAIL_GENDER)
    {
        u32 personality;
        u32 original = GetMonData(ReturnPartyMon(), MON_DATA_PERSONALITY);
        u8 currentGender = GetGenderFromSpeciesAndPersonality(
            sStatEditorDataPtr->speciesID, sStatEditorDataPtr->newPersonality);
        u8 originalGender = GetGenderFromSpeciesAndPersonality(
            sStatEditorDataPtr->speciesID, original);
        u8 nextGender = currentGender == MON_MALE ? MON_FEMALE : MON_MALE;

        if (nextGender == originalGender)
        {
            // Toggling back restores the exact original PID, including
            // Spinda markings and any other PID-dependent characteristics.
            sStatEditorDataPtr->newPersonality = original;
        }
        else
        {
            if (!FindGenderPersonality(sStatEditorDataPtr->speciesID,
                                       original, nextGender, &personality))
            {
                PlaySE(SE_FAILURE);
                return;
            }
            sStatEditorDataPtr->newPersonality = personality;
        }
    }
    else if (sStatEditorDataPtr->selectedDetail == DETAIL_NATURE)
    {
        u8 value = sStatEditorDataPtr->newNature;

        if (jumpToEnd)
            value = increase ? NUM_NATURES - 1 : 0;
        else
            value = increase ? (value + 1) % NUM_NATURES : (value + NUM_NATURES - 1) % NUM_NATURES;

        sStatEditorDataPtr->newNature = value;
    }
    else
    {
        u8 slots[NUM_ABILITY_SLOTS];
        u8 count = GetValidAbilitySlots(sStatEditorDataPtr->speciesID, slots);
        u8 i, position = 0;
        bool8 found = FALSE;

        if (count <= 1)
            return;

        for (i = 0; i < count; i++)
        {
            if (slots[i] == sStatEditorDataPtr->newAbilitySlot)
            {
                position = i;
                found = TRUE;
                break;
            }
        }

        if (jumpToEnd)
            position = increase ? count - 1 : 0;
        else if (!found)
            position = increase ? 0 : count - 1;
        else
            position = increase ? (position + 1) % count : (position + count - 1) % count;

        sStatEditorDataPtr->newAbilitySlot = slots[position];
    }

    // Preview changes immediately; no Pokémon data is saved until START
    // (or the existing commit-before-changing-party behavior).
    PrintMonStats();
}

static void Task_MenuEditingDetail(u8 taskId)
{
    if (JOY_NEW(L_BUTTON))
    {
        PlaySE(SE_SELECT);
        ChangePartyPokemon(taskId, -1);
        return;
    }
    if (JOY_NEW(R_BUTTON))
    {
        PlaySE(SE_SELECT);
        ChangePartyPokemon(taskId, 1);
        return;
    }
    if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
    {
        // A stages, just like EVs/IVs. B retains the existing behavior.
        // Refresh a gender-dependent front sprite only once on staging.
        if (sStatEditorDataPtr->selectedDetail == DETAIL_GENDER)
            RefreshStatEditorMonSprite();
        gTasks[taskId].func = Task_StatEditorMain;
        StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 2);
        PlaySE(SE_SELECT);
        sStatEditorDataPtr->inputMode = INPUT_SELECT_STAT;
        PrintTitleToWindowMainState();
        return;
    }

    if (JOY_REPEAT(DPAD_LEFT))
        ChangeDetailValue(FALSE, FALSE);
    else if (JOY_REPEAT(DPAD_RIGHT))
        ChangeDetailValue(TRUE, FALSE);
    else if (JOY_NEW(DPAD_UP))
        ChangeDetailValue(TRUE, TRUE);
    else if (JOY_NEW(DPAD_DOWN))
        ChangeDetailValue(FALSE, TRUE);
}

#define EDIT_INPUT_INCREASE_STATE           0
#define EDIT_INPUT_MAX_INCREASE_STATE       1
#define EDIT_INPUT_DECREASE_STATE           2
#define EDIT_INPUT_MAX_DECREASE_STATE       3

#define STAT_MINIMUM          0  
#define IV_MAX_SINGLE_STAT    31   
#define EV_MAX_SINGLE_STAT    255   
#define EV_MAX_TOTAL          510            
                
#define EDITING_EVS     0
#define EDITING_IVS     1

#define CHECK_IF_STAT_CANT_INCREASE (((sStatEditorDataPtr->editingStat == ((sStatEditorDataPtr->selector_x == EDITING_EVS) ? (EV_MAX_SINGLE_STAT) : (IV_MAX_SINGLE_STAT))) \
                                     || ((sStatEditorDataPtr->selector_x == EDITING_EVS) && (sStatEditorDataPtr->evTotal == EV_MAX_TOTAL))))

static void HandleEditingStatInput(u32 input)
{
    u16 iterator = 0;
    if((input <= EDIT_INPUT_MAX_INCREASE_STATE) && CHECK_IF_STAT_CANT_INCREASE)
    {
        StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 2);
        return;
    }

    if((input >= EDIT_INPUT_DECREASE_STATE) && (sStatEditorDataPtr->editingStat == STAT_MINIMUM))
    {
        StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 1);
        return;
    }

    #define INCREASE_DECREASE_AMOUNT 1

    switch(input)
    {
        case EDIT_INPUT_DECREASE_STATE:
            for (iterator = 0; iterator < INCREASE_DECREASE_AMOUNT; iterator++)
            {
                if(!(sStatEditorDataPtr->editingStat == STAT_MINIMUM))
                    sStatEditorDataPtr->editingStat--;
                else
                    break;
            }
            break;
       case EDIT_INPUT_MAX_DECREASE_STATE:
            sStatEditorDataPtr->editingStat = STAT_MINIMUM;
            break;
        case EDIT_INPUT_INCREASE_STATE:
            for (iterator = 0; iterator < INCREASE_DECREASE_AMOUNT; iterator++)
            {
                if(!CHECK_IF_STAT_CANT_INCREASE)
                    sStatEditorDataPtr->editingStat++;
                else
                    break;
            }
            break;
        case EDIT_INPUT_MAX_INCREASE_STATE:
            if((sStatEditorDataPtr->selector_x == EDITING_EVS))
            {
                if (EV_MAX_TOTAL - sStatEditorDataPtr->evTotal < EV_MAX_SINGLE_STAT)
                    sStatEditorDataPtr->editingStat += EV_MAX_TOTAL - sStatEditorDataPtr->evTotal;
                else
                    sStatEditorDataPtr->editingStat = EV_MAX_SINGLE_STAT;
                if(sStatEditorDataPtr->editingStat > EV_MAX_SINGLE_STAT)
                    sStatEditorDataPtr->editingStat = EV_MAX_SINGLE_STAT;
            }
            else
            {
                sStatEditorDataPtr->editingStat = IV_MAX_SINGLE_STAT;
            }
    }

    ConvertEnumToFromArray(sStatEditorDataPtr->selectedStat, SET_STAT);
    PrintMonStats();

    if(CHECK_IF_STAT_CANT_INCREASE)
        StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 2);
    else if(sStatEditorDataPtr->editingStat == STAT_MINIMUM)
        StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 1); 
    else
        StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 3);       
}

static void Task_MenuEditingStat(u8 taskId) // This function should be refactored to not be a hot mess
{
    if (JOY_NEW(L_BUTTON))
    {
        PlaySE(SE_SELECT);
        ChangePartyPokemon(taskId, -1);
        return;
    }
    if (JOY_NEW(R_BUTTON))
    {
        PlaySE(SE_SELECT);
        ChangePartyPokemon(taskId, 1);
        return;
    }
    if (JOY_NEW(B_BUTTON) || JOY_NEW(A_BUTTON)) // Change this so that pressing B returns the stat to original value
    {
        gTasks[taskId].func = Task_StatEditorMain;
        StartSpriteAnim(&gSprites[sStatEditorDataPtr->selectorSpriteId], 0);
        PlaySE(SE_SELECT);
        sStatEditorDataPtr->inputMode = INPUT_SELECT_STAT;
        PrintTitleToWindowMainState();
        return;
    }
    if (JOY_NEW(DPAD_LEFT))
        HandleEditingStatInput(EDIT_INPUT_DECREASE_STATE);
    else if (JOY_NEW(DPAD_RIGHT))
        HandleEditingStatInput(EDIT_INPUT_INCREASE_STATE);
    else if (JOY_NEW(DPAD_UP))
        HandleEditingStatInput(EDIT_INPUT_MAX_INCREASE_STATE);
    else if (JOY_NEW(DPAD_DOWN))
        HandleEditingStatInput(EDIT_INPUT_MAX_DECREASE_STATE);

}

static void PlayMonCry(void)
{
    bool8 isEgg = sStatEditorDataPtr->speciesID == SPECIES_EGG ? TRUE : FALSE;

    if (!isEgg)
        PlayCry_ByMode(sStatEditorDataPtr->speciesID, 0, CRY_MODE_NORMAL);
}

static void SpriteCB_Pokemon(struct Sprite *sprite)
{
    bool8 isEgg = sStatEditorDataPtr->speciesID == SPECIES_EGG ? TRUE : FALSE;

    if (!gPaletteFade.active && sprite->sDelayAnim != 1)
    {
        sprite->sDontFlip = TRUE;

        if (!sStatEditorDataPtr->monAnimPlayed) // only play cry on the first time mon is animated
            PlayMonCry();

        PokemonSummaryDoMonAnimation(sprite, sprite->sSpecies, isEgg);
        sStatEditorDataPtr->monAnimPlayed = TRUE;
    }
}

#undef sSpecies
#undef sDontFlip
#undef sDelayAnim
#undef sIsShadow
