#include <assert.h>
#include <stdio.h>
#include "finite_fusion_stats.h"

int main(void)
{
    unsigned int head, body, stat;
    unsigned long cases = 0;
    // Synthetic inputs keep the test independent from generation-specific balance.
    struct FiniteFusionBaseStats a = {{55, 50, 45, 120, 135, 95}};
    struct FiniteFusionBaseStats b = {{160, 110, 65, 30, 65, 110}};
    struct FiniteFusionBaseStats out;
    const unsigned short expected[] = {129, 94, 62, 92, 117, 111};
    const unsigned short reversed[] = {106, 81, 58, 72, 102, 114};

    assert(FiniteFusionCalculateBaseStats(&a, &b, &out));
    for (stat = 0; stat < FF_STAT_COUNT; stat++)
        assert(out.values[stat] == expected[stat]);
    assert(FiniteFusionCalculateBaseStats(&b, &a, &out));
    for (stat = 0; stat < FF_STAT_COUNT; stat++)
        assert(out.values[stat] == reversed[stat]);

    for (head = 1; head <= 255; head++)
        for (body = 1; body <= 255; body++)
            for (stat = 0; stat < FF_STAT_COUNT; stat++)
            {
                unsigned int dominantHead = stat >= FF_STAT_SPEED;
                unsigned int blend = dominantHead ? 6 * head + 4 * body : 4 * head + 6 * body;
                unsigned int reference = blend * 11 / 100;
                unsigned int actual = FiniteFusionCalculateBaseStat(head, body, stat);
                if (reference > 255)
                    reference = 255;
                assert(actual == reference);
                assert(actual >= blend / 10);
                assert(actual >= 1 && actual <= 255);
                if (head < 255)
                    assert(FiniteFusionCalculateBaseStat(head + 1, body, stat) >= actual);
                if (body < 255)
                    assert(FiniteFusionCalculateBaseStat(head, body + 1, stat) >= actual);
                cases++;
            }

    assert(FiniteFusionCalculateBaseStat(100, 100, FF_STAT_HP) == 110);
    assert(FiniteFusionCalculateBaseStat(1, 1, FF_STAT_HP) == 1);
    assert(FiniteFusionCalculateBaseStat(255, 255, FF_STAT_HP) == 255);
    assert(FiniteFusionCalculateBaseStat(0, 100, FF_STAT_HP) == 0);
    assert(FiniteFusionCalculateBaseStat(100, 256, FF_STAT_HP) == 0);
    assert(FiniteFusionCalculateBaseStat(100, 100, FF_STAT_COUNT) == 0);
    assert(FiniteFusionCalculateBaseStat(100, 100, (enum FiniteFusionStat)-1) == 0);
    assert(!FiniteFusionCalculateBaseStats(NULL, &b, &out));
    assert(!FiniteFusionCalculateBaseStats(&a, NULL, &out));
    assert(!FiniteFusionCalculateBaseStats(&a, &b, NULL));

    assert(FiniteFusionCalculateBaseStats(&a, &b, &out));
    a.values[FF_STAT_SP_DEFENSE] = 0;
    assert(!FiniteFusionCalculateBaseStats(&a, &b, &out));
    for (stat = 0; stat < FF_STAT_COUNT; stat++)
        assert(out.values[stat] == expected[stat]);
    a.values[FF_STAT_SP_DEFENSE] = 95;
    assert(FiniteFusionCalculateBaseStats(&a, &b, &a));
    for (stat = 0; stat < FF_STAT_COUNT; stat++)
        assert(a.values[stat] == expected[stat]);
    printf("PASS: %lu stat combinations plus fixtures and invalid-input checks\n", cases);
    return 0;
}
