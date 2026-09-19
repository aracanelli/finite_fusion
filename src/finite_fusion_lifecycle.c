#include <stdint.h>
#include <string.h>
#include "finite_fusion_lifecycle.h"

static unsigned char *Record(void *records, size_t size, unsigned int index)
{
    return (unsigned char *)records + size * index;
}

static unsigned int Overlaps(const void *a, size_t aSize, const void *b, size_t bSize)
{
    uintptr_t x = (uintptr_t)a;
    uintptr_t y = (uintptr_t)b;
    return x <= y ? y - x < aSize : x - y < bSize;
}

enum FiniteFusionResult FiniteFusionValidate(const struct FiniteFusionContext *c)
{
    unsigned int i;
    unsigned char used[FF_PARTNER_CAPACITY] = {0};
    struct FiniteFusionFacts facts;

    if (!c || !c->party || !c->partners || !c->read
        || c->recordSize == 0 || c->recordSize > FF_RECORD_MAX_BYTES)
        return FF_BAD_ARGUMENT;
    // Reject overlap without addition/overflow, including the view's metadata.
    if (Overlaps(c->party, c->recordSize * FF_PARTY_CAPACITY,
                 c->partners, c->recordSize * FF_PARTNER_CAPACITY)
        || Overlaps(c, sizeof(*c), c->party, c->recordSize * FF_PARTY_CAPACITY)
        || Overlaps(c, sizeof(*c), c->partners, c->recordSize * FF_PARTNER_CAPACITY))
        return FF_BAD_ARGUMENT;
    if (c->count > FF_PARTY_CAPACITY)
        return FF_CORRUPT_STATE;

    for (i = 0; i < FF_PARTY_CAPACITY; i++)
    {
        if (!c->read(Record(c->party, c->recordSize, i), &facts))
            return FF_CORRUPT_STATE;
        if ((i < c->count) != (facts.species != 0))
            return FF_CORRUPT_STATE;
        if (c->links[i] > FF_PARTNER_CAPACITY || c->reversed[i] > 1
            || (!c->links[i] && c->reversed[i])
            || (i >= c->count && c->links[i]))
            return FF_CORRUPT_STATE;
        if (c->links[i])
        {
            if (used[c->links[i] - 1]++)
                return FF_CORRUPT_STATE;
        }
    }
    for (i = 0; i < FF_PARTNER_CAPACITY; i++)
    {
        if (!c->read(Record(c->partners, c->recordSize, i), &facts)
            || ((facts.species != 0) != (used[i] != 0)))
            return FF_CORRUPT_STATE; // orphan, missing partner or duplicate owner
    }
    return FF_OK;
}

static unsigned int Eligible(const struct FiniteFusionFacts *f)
{
    // IDs supplied by the adapter are NATIONAL DEX IDs, not arbitrary enum values.
    return f->species >= 1 && f->species <= 386 && f->species != 292
        && f->hp != 0 && !f->isEgg && !f->isBadEgg && !f->heldItem;
}

enum FiniteFusionResult FiniteFusionFuse(struct FiniteFusionContext *c,
                                       unsigned int anchor, unsigned int partner)
{
    enum FiniteFusionResult result = FiniteFusionValidate(c);
    struct FiniteFusionFacts a, b, stored;
    unsigned int slot, i;
    if (result != FF_OK)
        return result;
    if (anchor >= c->count || partner >= c->count || anchor == partner)
        return FF_BAD_ARGUMENT;
    if (c->links[anchor] || c->links[partner])
        return FF_ALREADY_FUSED;
    if (!c->read(Record(c->party, c->recordSize, anchor), &a)
        || !c->read(Record(c->party, c->recordSize, partner), &b))
        return FF_CORRUPT_STATE;
    if (!Eligible(&a) || !Eligible(&b))
        return FF_INELIGIBLE;
    for (slot = 0; slot < FF_PARTNER_CAPACITY; slot++)
    {
        if (!c->read(Record(c->partners, c->recordSize, slot), &stored))
            return FF_CORRUPT_STATE;
        if (!stored.species)
            break;
    }
    if (slot == FF_PARTNER_CAPACITY)
        return FF_STORAGE_FULL;

    // No failing operations after this point. Commit synchronously between frames.
    memcpy(Record(c->partners, c->recordSize, slot),
           Record(c->party, c->recordSize, partner), c->recordSize);
    c->links[anchor] = slot + 1;
    c->reversed[anchor] = 0;
    for (i = partner; i + 1 < c->count; i++)
    {
        memcpy(Record(c->party, c->recordSize, i),
               Record(c->party, c->recordSize, i + 1), c->recordSize);
        c->links[i] = c->links[i + 1];
        c->reversed[i] = c->reversed[i + 1];
    }
    c->count--;
    memset(Record(c->party, c->recordSize, c->count), 0, c->recordSize);
    c->links[c->count] = c->reversed[c->count] = 0;
    return FF_OK;
}

enum FiniteFusionResult FiniteFusionReverse(struct FiniteFusionContext *c, unsigned int anchor)
{
    enum FiniteFusionResult result = FiniteFusionValidate(c);
    if (result != FF_OK)
        return result;
    if (anchor >= c->count)
        return FF_BAD_ARGUMENT;
    if (!c->links[anchor])
        return FF_NOT_FUSED;
    c->reversed[anchor] ^= 1;
    return FF_OK;
}

enum FiniteFusionResult FiniteFusionSplit(struct FiniteFusionContext *c, unsigned int anchor)
{
    enum FiniteFusionResult result = FiniteFusionValidate(c);
    unsigned int slot;
    if (result != FF_OK)
        return result;
    if (anchor >= c->count)
        return FF_BAD_ARGUMENT;
    if (!c->links[anchor])
        return FF_NOT_FUSED;
    if (c->count == FF_PARTY_CAPACITY)
        return FF_PARTY_FULL;
    slot = c->links[anchor] - 1;
    memcpy(Record(c->party, c->recordSize, c->count),
           Record(c->partners, c->recordSize, slot), c->recordSize);
    memset(Record(c->partners, c->recordSize, slot), 0, c->recordSize);
    c->links[anchor] = c->reversed[anchor] = 0;
    c->links[c->count] = c->reversed[c->count] = 0;
    c->count++;
    return FF_OK;
}

enum FiniteFusionResult FiniteFusionGetComponents(const struct FiniteFusionContext *c,
                                                unsigned int anchor,
                                                unsigned int *head, unsigned int *body)
{
    enum FiniteFusionResult result = FiniteFusionValidate(c);
    struct FiniteFusionFacts a, b;
    if (result != FF_OK)
        return result;
    if (!head || !body || head == body || anchor >= c->count)
        return FF_BAD_ARGUMENT;
    if (!c->links[anchor])
        return FF_NOT_FUSED;
    if (!c->read(Record(c->party, c->recordSize, anchor), &a)
        || !c->read(Record(c->partners, c->recordSize, c->links[anchor] - 1), &b))
        return FF_CORRUPT_STATE;
    *head = c->reversed[anchor] ? a.species : b.species;
    *body = c->reversed[anchor] ? b.species : a.species;
    return FF_OK;
}
