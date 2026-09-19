# Lifecycle prototype and save audit

## Status

This milestone implements **an isolated, host-tested C transaction core**. It is
not an in-game debug menu or save adapter. There are no writes to Emerald globals,
no changes to BoxPokemon, no item UI and no stat recalculation hooks. Do not wire
these functions directly to the live party until the gates below are complete.

## What the audit found

Inspected baseline: `597936ed0acf4144896048d5b3d8f125b141f05f`.

- `include/pokemon_storage_system.h`: four full Pokemon records already exist in
  `PokemonStorage.fusions`; the structure is in the saved storage block.
- `src/data/pokemon/form_change_tables.h`: slot 0 belongs to Kyurem; slots 1 and 2
  to Necrozma; slot 3 to Calyrex. None is permanently free for us to reuse.
- `src/party_menu.c`: existing official-fusion handlers copy into and clear those
  slots. They do not recognize Finite Fusion ownership.
- `src/save.c`: PokemonStorage is divided over nine sectors; a compile-time size
  assertion protects its capacity. Save slots alternate and sectors rotate.
- `include/pokemon.h`: boxed metadata has candidate spare bit fields, but adding
  to one of four equal-sized substructures can grow all four. Do not append a
  field casually. Existing defaults describe 12-byte substructures.
- `src/pokemon.c`: encrypted setters validate and refresh checksums; metadata must
  go through matching get/set paths. Reads are not a substitute for a checksum
  audit. The adapter must avoid getters that mutate caller records on failure.
- `BoxMonToMon` recalculates stats when reconstructing a party Pokemon. Overwriting
  party stats once would therefore not persist reliably.

Decision: do not borrow the four official-fusion slots or reinterpret spare bits
in this PR. Prototype with explicitly owned buffers; finalize native layout only
after measuring the compiled structures and auditing copy/serialization paths.

## Transaction core

`finite_fusion_lifecycle.h` accepts caller-owned arrays of six complete party
records and four complete hidden records. Record bytes are opaque. A read-only
adapter supplies national Dex ID, HP, egg/bad-egg and held-item facts. All-zero
records are empty. The core copies complete records, preserving unknown fields.

The ephemeral context stores party count, per-member hidden-slot ownership and
orientation. Each hidden record must have exactly one party owner. No owner can
reference an empty slot, and no hidden record may be orphaned. Party compaction
moves metadata with the corresponding record. A future PC implementation must
replace/extend this party-only ownership model rather than dropping that metadata.

- Fuse validates the entire state and capacity, then copies the partner and
  compacts the party. If the removed slot precedes the anchor, its index decreases.
- Reverse changes only orientation. It never swaps full records, rerolls identity,
  updates XP or applies another stat bonus.
- Split validates ownership and party space, restores the complete partner,
  clears its hidden slot, and leaves the current anchor record unchanged.
- All precondition failures leave records and context untouched. There are no
  callbacks or fallible operations during the memory-copy commit section.
- This is synchronous in-memory failure atomicity, NOT power-loss atomicity.
  Never save midway through a transaction or call this concurrently.

Eligibility is Gen I-III national Dex IDs, excluding Shedinja, with no egg, bad
egg, fainting, held item or nested fusion. Unsupported forms must also be rejected
by the future adapter; national Dex ID alone cannot identify a regional form.

## Verification and limitations

Run `bash tools/finite_fusion/test.sh` in WSL/Linux with a C compiler.

Tests run the actual core against synthetic records containing opaque identity,
moves and XP bytes. Coverage includes all 70 selections across party sizes 2-6,
byte-for-byte partner restoration, unaffected-party preservation, full party,
full storage, nested fusion, invalid eligibility, failed decoding, corrupt links,
orphaned slots, duplicate owners, slot reuse, same-species individuals and 100
reversals. Logical checkpoint/reload copies records and metadata to a new address
and reconstructs pointers. It is **not** a portable save format, a native Emerald
save/reset/load test or a test of interrupted flash writes.

The core deliberately leaves HP/stats unchanged. Before battles, an adapter must
derive stats and preserve HP ratios without healing exploits or rounding-induced
resurrection. Existing tests prove only that these transactions don't mutate HP.

## Remaining gates before in-game debug commands

1. Obtain a full ARM build and measure BoxPokemon/Pokemon/save-block/RAM sizes.
2. Choose dedicated storage or explicitly replace/disable all official-fusion
   handlers; no sharing without a common allocator and ownership contract.
3. Specify save format/version and migration. Test old saves or require new saves.
4. Implement native record adapter and checksummed metadata persistence, including
   decode errors, copies, reset/load and interrupted-save recovery.
5. Block PC deposit/release, trade, daycare, evolution, form changes and debug party
   replacement while fused. Prevent escape through secondary menus/scripts.
6. Integrate per-mon stat lookup, HP handling and typing, then expose debug-only
   commands on disposable test saves. Items and sprite composition come later.

Four concurrent fusions remains a prototype policy, not a proven hardware maximum.
