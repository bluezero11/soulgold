#ifndef GUARD_POKEVIAL_H
#define GUARD_POKEVIAL_H

// Pokémon World-style refillable whole-party healing using persistent vars.
#define POKEVIAL_STARTING_CHARGES 5
#define POKEVIAL_MAX_CHARGES 15

u16 PokeVial_GetCharges(void);
u16 PokeVial_GetMaxCharges(void);
bool32 PokeVial_Use(void);

#endif // GUARD_POKEVIAL_H
