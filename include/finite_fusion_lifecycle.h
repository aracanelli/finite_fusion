#ifndef GUARD_FINITE_FUSION_LIFECYCLE_H
#define GUARD_FINITE_FUSION_LIFECYCLE_H

#include <stddef.h>

#define FF_PARTY_CAPACITY 6u
#define FF_PARTNER_CAPACITY 4u
#define FF_RECORD_MAX_BYTES 256u

enum FiniteFusionResult
{
    FF_OK,
    FF_BAD_ARGUMENT,
    FF_CORRUPT_STATE,
    FF_INELIGIBLE,
    FF_ALREADY_FUSED,
    FF_NOT_FUSED,
    FF_STORAGE_FULL,
    FF_PARTY_FULL
};

// Adapter-provided facts. The opaque record remains authoritative for all data.
struct FiniteFusionFacts
{
    unsigned int species; // National Dex ID; 0 means empty.
    unsigned int hp;
    unsigned int heldItem;
    unsigned int isEgg;
    unsigned int isBadEgg;
};

// Must be read-only, deterministic, fill ALL facts, accept zero-filled records
// as empty, and return 0 for corrupt/unreadable records.
typedef unsigned int (*FiniteFusionReadRecord)(const void *, struct FiniteFusionFacts *);

// Caller owns two NON-OVERLAPPING contiguous buffers of complete Pokemon records.
// This view is ephemeral: do not write its pointers to a save file.
// Metadata must move with each party member. links are 0 (normal) or slot + 1.
// Buffers and metadata are an isolated prototype, NOT gPokemonStoragePtr->fusions.
struct FiniteFusionContext
{
    void *party;
    void *partners;
    size_t recordSize;
    FiniteFusionReadRecord read;
    unsigned char count;
    unsigned char links[FF_PARTY_CAPACITY];
    unsigned char reversed[FF_PARTY_CAPACITY];
};

enum FiniteFusionResult FiniteFusionValidate(const struct FiniteFusionContext *context);
// Anchor initially supplies the body and keeps its full record unchanged.
// Partner is moved to hidden storage; other party records are compacted.
// On ANY failure all buffers and context fields remain unchanged.
enum FiniteFusionResult FiniteFusionFuse(struct FiniteFusionContext *context,
                                       unsigned int anchor, unsigned int partner);
enum FiniteFusionResult FiniteFusionReverse(struct FiniteFusionContext *context,
                                          unsigned int anchor);
enum FiniteFusionResult FiniteFusionSplit(struct FiniteFusionContext *context,
                                        unsigned int anchor);
enum FiniteFusionResult FiniteFusionGetComponents(const struct FiniteFusionContext *context,
                                                unsigned int anchor,
                                                unsigned int *head, unsigned int *body);

// GetComponents output pointers must be distinct and must not alias context or
// either record buffer. The caller must serialize metadata along with records;
// persisting the record buffers alone loses ownership/orientation information.

// Deliberately does not recalculate HP/stats, touch globals, use items or write saves.
// A future engine adapter must validate checksums and supply eligibility facts,
// implement HP/stat handling and unsupported-operation guards before exposure.
#endif
