#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "pokevial.h"
#include "constants/vars.h"

// Based on the Pokévial from evilchinesefood/PKMN-World.
// Persistent vars avoid incompatible changes to SoulGold's SaveBlock layout.
static void PokeVial_Init(void)
{
    u16 capacity = VarGet(VAR_POKEVIAL_MAX_CHARGES);
    u16 charges = VarGet(VAR_POKEVIAL_CHARGES);

    if (capacity == 0)
    {
        VarSet(VAR_POKEVIAL_MAX_CHARGES, POKEVIAL_STARTING_CHARGES);
        VarSet(VAR_POKEVIAL_CHARGES, POKEVIAL_STARTING_CHARGES);
        return;
    }

    if (capacity > POKEVIAL_MAX_CHARGES)
    {
        capacity = POKEVIAL_MAX_CHARGES;
        VarSet(VAR_POKEVIAL_MAX_CHARGES, capacity);
    }
    if (charges > capacity)
        VarSet(VAR_POKEVIAL_CHARGES, capacity);
}

u16 PokeVial_GetCharges(void)
{
    PokeVial_Init();
    return VarGet(VAR_POKEVIAL_CHARGES);
}

u16 PokeVial_GetMaxCharges(void)
{
    PokeVial_Init();
    return VarGet(VAR_POKEVIAL_MAX_CHARGES);
}

bool32 PokeVial_Use(void)
{
    u32 i;
    u16 charges = PokeVial_GetCharges();

    if (gPlayerPartyCount == 0 || charges == 0)
        return FALSE;

    // HealPokemon restores HP, PP and status, including fainting.
    // Unlike HealPlayerParty, it doesn't heal stored Pokémon in Gen 8 mode.
    for (i = 0; i < gPlayerPartyCount; i++)
        HealPokemon(&gPlayerParty[i]);

    VarSet(VAR_POKEVIAL_CHARGES, charges - 1);
    return TRUE;
}
