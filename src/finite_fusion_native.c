#include "global.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "finite_fusion_native.h"
#include "malloc.h"

#if FF_NATIVE_DEBUG
#define FF_NATIVE_MAGIC 0x46464431u
#define FF_NATIVE_VERSION 1

STATIC_ASSERT(PARTY_SIZE == FF_PARTY_CAPACITY, FiniteFusionPartyCapacity);
STATIC_ASSERT(sizeof(struct Pokemon) <= FF_RECORD_MAX_BYTES, FiniteFusionRecordCapacity);
STATIC_ASSERT(STAT_HP == FF_STAT_HP && STAT_ATK == FF_STAT_ATTACK
    && STAT_DEF == FF_STAT_DEFENSE && STAT_SPEED == FF_STAT_SPEED
    && STAT_SPATK == FF_STAT_SP_ATTACK && STAT_SPDEF == FF_STAT_SP_DEFENSE,
    FiniteFusionStatOrder);

static unsigned int ReadNativeRecord(const void *record, struct FiniteFusionFacts *facts)
{
    struct Pokemon copy;
    enum Species species;
    memcpy(&copy, record, sizeof(copy));
    memset(facts, 0, sizeof(*facts));
    if (!IsBoxMonChecksumValid(&copy.box) || copy.box.isBadEgg)
        return 0;
    species = GetMonData(&copy, MON_DATA_SPECIES);
    if (species == SPECIES_NONE)
        return !copy.box.hasSpecies;
    if (!copy.box.hasSpecies || species >= NUM_SPECIES || copy.level == 0
        || copy.level > MAX_LEVEL || copy.hp > copy.maxHP || copy.maxHP == 0)
        return 0;
    facts->species = SpeciesToNationalPokedexNum(species);
    // Alternate forms are not supported, even when mapping to a Gen I-III dex ID.
    if (species > SPECIES_DEOXYS_NORMAL)
        facts->species = 387;
    facts->hp = copy.hp;
    facts->heldItem = GetMonData(&copy, MON_DATA_HELD_ITEM);
    facts->isEgg = GetMonData(&copy, MON_DATA_IS_EGG);
    facts->isBadEgg = copy.box.isBadEgg;
    return facts->species != 0;
}

static struct FiniteFusionContext MakeContext(struct FiniteFusionNativeSave *save)
{
    struct FiniteFusionContext context = {0};
    context.party = save->party;
    context.partners = save->partners;
    context.recordSize = sizeof(struct Pokemon);
    context.read = ReadNativeRecord;
    context.count = save->count;
    memcpy(context.links, save->links, sizeof(context.links));
    memcpy(context.reversed, save->reversed, sizeof(context.reversed));
    return context;
}

static bool32 ValidHeader(const struct FiniteFusionNativeSave *save)
{
    return save->magic == FF_NATIVE_MAGIC && save->version == FF_NATIVE_VERSION
        && save->reserved == 0;
}

static enum FiniteFusionResult CaptureParty(struct FiniteFusionNativeSave *save)
{
    struct FiniteFusionNativeSave *candidate;
    struct FiniteFusionContext context;
    enum FiniteFusionResult result;

    if (ValidHeader(save))
    {
        context = MakeContext(save);
        result = FiniteFusionValidate(&context);
        if (result != FF_OK)
            return result;
        for (u32 i = 0; i < FF_PARTY_CAPACITY; i++)
            if (save->links[i])
                return FF_ALREADY_FUSED; // split existing copies before recapture
    }
    else if (save->magic != 0 || save->version != 0)
        return FF_CORRUPT_STATE; // no implicit migration/reset of unknown versions

    candidate = AllocZeroed(sizeof(*candidate));
    if (!candidate)
        return FF_BAD_ARGUMENT;
    candidate->magic = FF_NATIVE_MAGIC;
    candidate->version = FF_NATIVE_VERSION;
    candidate->count = gPartiesCount[B_TRAINER_PLAYER];
    memcpy(candidate->party, gParties[B_TRAINER_PLAYER], sizeof(candidate->party));
    context = MakeContext(candidate);
    result = FiniteFusionValidate(&context);
    if (result == FF_OK)
        *save = *candidate;
    Free(candidate);
    return result;
}

enum FiniteFusionResult FiniteFusionNativeAction(enum FiniteFusionNativeAction action)
{
    struct FiniteFusionNativeSave *save = &gPokemonStoragePtr->finiteFusion;
    struct FiniteFusionContext context;
    enum FiniteFusionResult result;
    if (action == FF_NATIVE_CAPTURE)
        return CaptureParty(save);
    if (!ValidHeader(save))
        return FF_CORRUPT_STATE;
    context = MakeContext(save);
    switch (action)
    {
    case FF_NATIVE_FUSE: result = FiniteFusionFuse(&context, 0, 1); break;
    case FF_NATIVE_REVERSE: result = FiniteFusionReverse(&context, 0); break;
    case FF_NATIVE_SPLIT: result = FiniteFusionSplit(&context, 0); break;
    case FF_NATIVE_VALIDATE: return FiniteFusionValidate(&context);
    default: return FF_BAD_ARGUMENT;
    }
    if (result == FF_OK)
    {
        save->count = context.count;
        memcpy(save->links, context.links, sizeof(save->links));
        memcpy(save->reversed, context.reversed, sizeof(save->reversed));
    }
    return result;
}

unsigned int FiniteFusionNativeCount(void)
{
    const struct FiniteFusionNativeSave *save = &gPokemonStoragePtr->finiteFusion;
    return ValidHeader(save) ? save->count : 0;
}

enum FiniteFusionResult FiniteFusionNativePreview(struct FiniteFusionBaseStats *stats)
{
    struct FiniteFusionNativeSave *save = &gPokemonStoragePtr->finiteFusion;
    struct FiniteFusionContext context;
    struct FiniteFusionBaseStats a, b;
    struct Pokemon anchor, partner;
    enum Species anchorSpecies, partnerSpecies;
    enum FiniteFusionResult result;
    if (!stats)
        return FF_BAD_ARGUMENT;
    if (!ValidHeader(save))
        return FF_CORRUPT_STATE;
    context = MakeContext(save);
    result = FiniteFusionValidate(&context);
    if (result != FF_OK)
        return result;
    if (!context.count || !context.links[0])
        return FF_NOT_FUSED;
    anchor = save->party[0];
    partner = save->partners[context.links[0] - 1];
    anchorSpecies = GetMonData(&anchor, MON_DATA_SPECIES);
    partnerSpecies = GetMonData(&partner, MON_DATA_SPECIES);
    for (u32 i = 0; i < FF_STAT_COUNT; i++)
    {
        a.values[i] = GetSpeciesBaseStat(anchorSpecies, i);
        b.values[i] = GetSpeciesBaseStat(partnerSpecies, i);
    }
    return FiniteFusionCalculateBaseStats(context.reversed[0] ? &a : &b,
        context.reversed[0] ? &b : &a, stats) ? FF_OK : FF_INELIGIBLE;
}
#endif
