#ifndef GUARD_FINITE_FUSION_NATIVE_H
#define GUARD_FINITE_FUSION_NATIVE_H

#include "config/finite_fusion.h"
#include "gametypes.h"
#include "pokemon.h"
#include "finite_fusion_lifecycle.h"
#include "finite_fusion_stats.h"

#if FF_NATIVE_DEBUG
// Include after global.h and pokemon.h. Appended to PokemonStorage only in the
// opt-in build. Never serialize the pointer-bearing FiniteFusionContext.
struct FiniteFusionNativeSave
{
    u32 magic;
    u16 version;
    u8 count;
    u8 reserved;
    u8 links[FF_PARTY_CAPACITY];
    u8 reversed[FF_PARTY_CAPACITY];
    struct Pokemon party[FF_PARTY_CAPACITY];
    struct Pokemon partners[FF_PARTNER_CAPACITY];
};

enum FiniteFusionNativeAction
{
    FF_NATIVE_CAPTURE,
    FF_NATIVE_FUSE,
    FF_NATIVE_REVERSE,
    FF_NATIVE_SPLIT,
    FF_NATIVE_VALIDATE
};

enum FiniteFusionResult FiniteFusionNativeAction(enum FiniteFusionNativeAction action);
enum FiniteFusionResult FiniteFusionNativePreview(struct FiniteFusionBaseStats *stats);
unsigned int FiniteFusionNativeCount(void);
#endif
#endif
