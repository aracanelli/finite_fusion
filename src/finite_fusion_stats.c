#include "finite_fusion_stats.h"

unsigned int FiniteFusionCalculateBaseStat(unsigned int head,
                                         unsigned int body,
                                         enum FiniteFusionStat stat)
{
    unsigned int dominant;
    unsigned int other;
    unsigned int result;

    if ((unsigned int)stat >= FF_STAT_COUNT
        || head < FF_MIN_BASE_STAT || head > FF_MAX_BASE_STAT
        || body < FF_MIN_BASE_STAT || body > FF_MAX_BASE_STAT)
        return 0;

    if (stat == FF_STAT_SPEED || stat == FF_STAT_SP_ATTACK || stat == FF_STAT_SP_DEFENSE)
    {
        dominant = head;
        other = body;
    }
    else
    {
        dominant = body;
        other = head;
    }

    // 60/40 blend plus 10% synergy. Round down ONCE, after the bonus.
    // Maximum intermediate: (3*255 + 2*255)*110 = 140250 (32-bit safe).
    result = ((3u * dominant + 2u * other) * FF_SYNERGY_PERCENT) / 500u;
    if (result > FF_MAX_BASE_STAT)
        result = FF_MAX_BASE_STAT;
    return result;
}

unsigned int FiniteFusionCalculateBaseStats(const struct FiniteFusionBaseStats *head,
                                          const struct FiniteFusionBaseStats *body,
                                          struct FiniteFusionBaseStats *output)
{
    struct FiniteFusionBaseStats result;
    unsigned int i;

    if (!head || !body || !output)
        return 0;

    for (i = 0; i < FF_STAT_COUNT; i++)
    {
        result.values[i] = FiniteFusionCalculateBaseStat(head->values[i], body->values[i], i);
        if (result.values[i] == 0)
            return 0;
    }
    *output = result;
    return 1;
}
