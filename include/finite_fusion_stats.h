#ifndef GUARD_FINITE_FUSION_STATS_H
#define GUARD_FINITE_FUSION_STATS_H

// A standalone, integer-only rules module. No save data or global species writes.
// Keep independent from engine headers so the same implementation can be host-tested.
enum FiniteFusionStat
{
    FF_STAT_HP,
    FF_STAT_ATTACK,
    FF_STAT_DEFENSE,
    FF_STAT_SPEED,
    FF_STAT_SP_ATTACK,
    FF_STAT_SP_DEFENSE,
    FF_STAT_COUNT
};

#define FF_SYNERGY_PERCENT 110u
#define FF_MIN_BASE_STAT 1u
#define FF_MAX_BASE_STAT 255u

struct FiniteFusionBaseStats
{
    unsigned short values[FF_STAT_COUNT];
};

// Returns 0 on invalid input; valid results are always in [1, 255].
unsigned int FiniteFusionCalculateBaseStat(unsigned int head,
                                         unsigned int body,
                                         enum FiniteFusionStat stat);

// Returns 1 on success. On invalid input, output remains unchanged.
// Output may alias either input. Pass swapped inputs to reverse orientation.
unsigned int FiniteFusionCalculateBaseStats(const struct FiniteFusionBaseStats *head,
                                          const struct FiniteFusionBaseStats *body,
                                          struct FiniteFusionBaseStats *output);

#endif
