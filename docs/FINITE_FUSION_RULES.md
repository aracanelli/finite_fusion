# Finite Fusion: implementation contract (draft v0.1)

## Scope and status

Keep Emerald's story, maps and encounters for the first playable version. Target
Gen I-III component species, not the entire expansion roster. Whether to restrict
the initial roster further to the Hoenn regional Dex remains open.

Implemented in this milestone: an integer-only stat calculation module and host
tests. It is NOT connected to live Pokemon stat recalculation yet. No items,
sprites, fusion transactions or new save fields are active. Ordinary Pokemon and
existing saves are unchanged. No GBA build or emulator validation is claimed.

## Base stats and strength

Use the original head/body proposal, plus a 10% synergy bonus:

| Stats | Head weight | Body weight |
| --- | --- | --- |
| HP, Attack, Defense | 40% | 60% |
| Speed, Special Attack, Special Defense | 60% | 40% |

For each stat: `min(255, floor((3 * dominant + 2 * other) * 110 / 500))`.
Round only once, after applying synergy. Use integer arithmetic, no floats.
Inputs must be between 1 and 255. Zero indicates invalid input, never a valid
calculated stat. Compute from original component base stats on every recalculation;
never apply the bonus to an already boosted value. Reversing twice must restore
the same derived base stats and never accumulate a bonus.

This raises the weighted blend by about 10%, subject to rounding and the cap. It
does not guarantee superiority to both components: a weak partner can lower a
strong parent's stats. Identical species get the synergy bonus, not doubled stats.
The bonus applies BEFORE the engine's normal level/IV/EV/nature calculations,
not as a direct 10% damage or final-HP multiplier. Do not edit gSpeciesInfo globally.

## Proposed first-playable rules

- Splicer: fuse two distinct, eligible, non-egg, healthy party members; choose head/body.
- Splitter: restore two components only with a free party slot.
- Reverser: swap visual/stat/type roles, not the anchor's identity or XP curve.
- Use reusable Key Items during prototyping; final acquisition/pricing is deferred.
- Initially retain anchor moves, nature, IVs, EVs, ability, held item and XP.
- Preserve the partner's individual data in hidden storage; it does not earn XP.
- XP/EV gains stay with the anchor after splitting; partner stats are not rerolled.
- Reject fusion if either component holds an item during the first prototype.
- Block nested fusions, fused evolution, PC deposit, release, trading and daycare
  until those paths have explicit integration tests. Never silently lose a partner.
- Reserve capacity before changing either party member. Cancellation/failure must
  leave both Pokemon and item counts unchanged. Full hidden storage must fail safely.
- Preserve fainted state and HP percentage on reverse; prevent reverse cycles from
  generating healing. HP, PP and status transitions need transaction tests.
- Use head primary/body secondary typing, with documented duplicate/Normal-Flying
  handling before implementation. No extra type or double ability by default.
- Exclude Shedinja from the first playable eligible roster pending an explicit
  one-HP/Wonder Guard policy; exclude unsupported special forms as needed.

These are first-playable targets, not guarantees implemented by the stat module.

## Save and rendering feasibility gates

The expansion has four existing official-fusion storage slots. Reusing them would
require ownership tracking and disabling or segregating existing fusion features.
Four concurrent fusions is a prototype choice, not a hardware limit. Do not raise
it until save-sector/RAM budgets are measured.

Unused packed bits are candidates, not a verified save format. Audit all reads,
writes, checksums, initialization, old-save values and copy paths before reuse.
Use validated IDs or an explicit roster mapping, not assumed contiguous Dex IDs.
An orientation flag must not change the anchor's experience growth curve.

Two-palette head/body composition is a rendering experiment, not yet proven.
Audit OAM, palette and animation use in single/double battles, summary and party UI.
Simple horizontal cuts will not produce clean heads for all species; masks and
per-species anchors may be needed. Hand-drawn overrides require permission/credits.

## Ordered milestones

1. **Stat foundation (implemented):** shared C calculation and exhaustive host tests.
2. **Save prototype:** audit storage, versioning and metadata; debug-only fusion,
   reverse and split; prove save/reset/load round trips and failure atomicity.
3. **Engine integration:** per-mon base-stat accessor, normal-mon regression tests,
   level-up recalculation, ability/type handling, no free healing or bonus stacking.
4. **Visual proof:** one front/back fusion, then summary and party icon, then arbitrary
   eligible pairs; measure real resource use before selecting the final renderer.
5. **Three items:** selection, confirmation, cancel, full-party/full-storage checks,
   restrictions and a small tutorial event without replacing Emerald's story.
6. **Playable gate:** battle/heal/faint/blackout/level-up/save/reload/reverse/split
   tests, followed by balance testing through the first gym.

## Running this milestone's tests

From the repository root on Linux or WSL with a C compiler:

```sh
bash tools/finite_fusion/test.sh
```

The tests compile the actual production C implementation, exhaustively check
255 x 255 x 6 stat inputs, test both orientations, caps, same-species behavior,
monotonicity, invalid input, unchanged output on failure and input/output aliasing.
They do not substitute for ROM, emulator, persistence or battle tests.
