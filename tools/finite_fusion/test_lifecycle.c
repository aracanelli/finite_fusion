#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "finite_fusion_lifecycle.h"
#include "finite_fusion_stats.h"

// Synthetic opaque records, NOT the game's Pokemon structure.
struct TestMon
{
    unsigned int species, hp, heldItem, egg, badEgg, experience;
    unsigned char identityAndMoves[76];
};
struct TestSave
{
    struct TestMon party[6], partners[4];
    unsigned char count, links[6], reversed[6];
};
static unsigned int Read(const void *record, struct FiniteFusionFacts *facts)
{
    struct TestMon m;
    memcpy(&m, record, sizeof(m));
    if (m.species == 9999) return 0; // checksum failure simulation
    facts->species = m.species;
    facts->hp = m.hp;
    facts->heldItem = m.heldItem;
    facts->isEgg = m.egg;
    facts->isBadEgg = m.badEgg;
    return 1;
}
static struct FiniteFusionContext View(struct TestSave *s)
{
    struct FiniteFusionContext c;
    memset(&c, 0, sizeof(c));
    c.party = s->party; c.partners = s->partners;
    c.recordSize = sizeof(struct TestMon); c.read = Read; c.count = s->count;
    memcpy(c.links, s->links, sizeof(c.links));
    memcpy(c.reversed, s->reversed, sizeof(c.reversed));
    return c;
}
static void Checkpoint(struct TestSave *s, const struct FiniteFusionContext *c)
{
    s->count = c->count;
    memcpy(s->links, c->links, sizeof(s->links));
    memcpy(s->reversed, c->reversed, sizeof(s->reversed));
}
static void Init(struct TestSave *s, unsigned int count)
{
    unsigned int i;
    memset(s, 0, sizeof(*s)); s->count = count;
    for (i = 0; i < count; i++)
    {
        s->party[i].species = i + 1; s->party[i].hp = 50;
        s->party[i].experience = 1000 + i;
        memset(s->party[i].identityAndMoves, i + 10, 76);
    }
}
#define UNCHANGED(call, expected) do { \
    struct TestSave before = s; \
    struct FiniteFusionContext beforeContext = c; \
    assert((call) == (expected)); \
    assert(memcmp(&s, &before, sizeof(s)) == 0); \
    assert(memcmp(&c, &beforeContext, sizeof(c)) == 0); \
} while (0)

int main(void)
{
    struct TestSave s, original, disk, restored;
    struct FiniteFusionContext c;
    unsigned int count, anchor, partner, actualAnchor, i, head, body, pairCases = 0;
    for (count = 2; count <= 6; count++)
        for (anchor = 0; anchor < count; anchor++)
            for (partner = 0; partner < count; partner++)
            {
                if (anchor == partner) continue;
                Init(&s, count); original = s; c = View(&s);
                assert(FiniteFusionFuse(&c, anchor, partner) == FF_OK);
                actualAnchor = anchor - (partner < anchor);
                assert(c.count == count - 1 && FiniteFusionValidate(&c) == FF_OK);
                assert(memcmp(&s.partners[0], &original.party[partner], sizeof(struct TestMon)) == 0);
                assert(memcmp(&s.party[actualAnchor], &original.party[anchor], sizeof(struct TestMon)) == 0);
                assert(FiniteFusionGetComponents(&c, actualAnchor, &head, &body) == FF_OK);
                assert(head == partner + 1 && body == anchor + 1);
                assert(FiniteFusionReverse(&c, actualAnchor) == FF_OK);
                assert(FiniteFusionGetComponents(&c, actualAnchor, &head, &body) == FF_OK);
                assert(head == anchor + 1 && body == partner + 1);
                assert(FiniteFusionReverse(&c, actualAnchor) == FF_OK);
                assert(c.reversed[actualAnchor] == 0);
                assert(FiniteFusionReverse(&c, actualAnchor) == FF_OK);
                // Logical reload at a different address, NOT an Emerald save test.
                Checkpoint(&s, &c); disk = s; memset(&s, 0, sizeof(s));
                restored = disk; c = View(&restored);
                assert(FiniteFusionValidate(&c) == FF_OK);
                assert(FiniteFusionGetComponents(&c, actualAnchor, &head, &body) == FF_OK);
                assert(head == anchor + 1 && body == partner + 1);
                restored.party[actualAnchor].experience += 123;
                restored.party[actualAnchor].hp = 0;
                assert(FiniteFusionReverse(&c, actualAnchor) == FF_OK);
                assert(restored.party[actualAnchor].hp == 0);
                assert(FiniteFusionSplit(&c, actualAnchor) == FF_OK);
                assert(c.count == count && FiniteFusionValidate(&c) == FF_OK);
                assert(memcmp(&restored.party[count - 1], &original.party[partner], sizeof(struct TestMon)) == 0);
                assert(restored.party[actualAnchor].experience == original.party[anchor].experience + 123);
                for (i = 0; i < count - 1; i++)
                    if (i != actualAnchor)
                        assert(memcmp(&restored.party[i], &original.party[i + (i >= partner)], sizeof(struct TestMon)) == 0);
                assert(FiniteFusionSplit(&c, actualAnchor) == FF_NOT_FUSED);
                pairCases++;
            }
    Init(&s, 6); c = View(&s);
    UNCHANGED(FiniteFusionFuse(&c, 0, 0), FF_BAD_ARGUMENT);
    UNCHANGED(FiniteFusionFuse(&c, 6, 0), FF_BAD_ARGUMENT);
    UNCHANGED(FiniteFusionReverse(&c, 0), FF_NOT_FUSED);
    for (i = 0; i < 7; i++)
    {
        Init(&s, 6); c = View(&s);
        switch (i)
        {
        case 0: s.party[1].hp = 0; break;
        case 1: s.party[1].heldItem = 1; break;
        case 2: s.party[1].egg = 1; break;
        case 3: s.party[1].badEgg = 1; break;
        case 4: s.party[1].species = 292; break;
        case 5: s.party[1].species = 387; break;
        case 6: s.party[1].species = 9999; break;
        }
        UNCHANGED(FiniteFusionFuse(&c, 0, 1), i == 6 ? FF_CORRUPT_STATE : FF_INELIGIBLE);
        UNCHANGED(FiniteFusionFuse(&c, 1, 0), i == 6 ? FF_CORRUPT_STATE : FF_INELIGIBLE);
    }
    Init(&s, 6); c = View(&s);
    for (i = 0; i < 4; i++)
    {
        assert(FiniteFusionFuse(&c, i, 5) == FF_OK);
        s.party[5] = s.partners[i]; // fixture simulates a new catch
        s.party[5].experience += 100; c.count = 6;
    }
    UNCHANGED(FiniteFusionFuse(&c, 4, 5), FF_STORAGE_FULL);
    UNCHANGED(FiniteFusionFuse(&c, 0, 5), FF_ALREADY_FUSED);
    UNCHANGED(FiniteFusionSplit(&c, 0), FF_PARTY_FULL);
    c.links[1] = c.links[0];
    UNCHANGED(FiniteFusionSplit(&c, 0), FF_CORRUPT_STATE);
    c.links[1] = 2;
    memset(&s.party[5], 0, sizeof(struct TestMon)); c.count = 5;
    assert(FiniteFusionSplit(&c, 2) == FF_OK);
    assert(FiniteFusionFuse(&c, 2, 5) == FF_OK);
    assert(FiniteFusionValidate(&c) == FF_OK);
    Init(&s, 2); c = View(&s); s.partners[0] = s.party[1];
    UNCHANGED(FiniteFusionFuse(&c, 0, 1), FF_CORRUPT_STATE);
    memset(&s.partners[0], 0, sizeof(struct TestMon)); c.links[0] = 1;
    UNCHANGED(FiniteFusionReverse(&c, 0), FF_CORRUPT_STATE);
    c.links[0] = 5;
    UNCHANGED(FiniteFusionSplit(&c, 0), FF_CORRUPT_STATE);
    c.links[0] = 0; c.reversed[0] = 1;
    UNCHANGED(FiniteFusionReverse(&c, 0), FF_CORRUPT_STATE);
    c.reversed[0] = 0; c.count = 7;
    UNCHANGED(FiniteFusionFuse(&c, 0, 1), FF_CORRUPT_STATE);
    c.count = 2; c.partners = c.party;
    UNCHANGED(FiniteFusionFuse(&c, 0, 1), FF_BAD_ARGUMENT);
    assert(FiniteFusionFuse(NULL, 0, 1) == FF_BAD_ARGUMENT);
    Init(&s, 2); s.party[1].species = 1; c = View(&s);
    assert(FiniteFusionFuse(&c, 0, 1) == FF_OK);
    for (i = 0; i < 100; i++)
    {
        assert(FiniteFusionReverse(&c, 0) == FF_OK);
        assert(FiniteFusionGetComponents(&c, 0, &head, &body) == FF_OK);
        assert(head == 1 && body == 1);
        assert(FiniteFusionCalculateBaseStat(100, 100, FF_STAT_HP) == 110);
    }
    assert(FiniteFusionSplit(&c, 0) == FF_OK);
    printf("PASS: %u party-order/reload cases, ownership, failure atomicity, 100 reversals\n", pairCases);
    return 0;
}
