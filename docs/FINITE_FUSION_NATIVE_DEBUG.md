# Native saved debug sandbox

## Scope

This opt-in build adds six debug-menu actions operating on saved **copies** of the
real party. It bridges the tested lifecycle core to actual `struct Pokemon` data
without permitting an unfinished fusion to enter live gameplay. No items or
sprites are added; battle stats, types and live party remain unchanged.

## Build and test

```sh
make FF_NATIVE_DEBUG=1 -j2
```

Output: `pokeemerald-fusion.gba`. This uses distinct object directories and output
names, so switching between normal and prototype builds does not silently reuse
objects compiled for a different storage layout. Prototype release builds are
rejected at compile time.

Use a **new disposable save** with this ROM. Do not import an existing Emerald,
Finite Fusion default-build or expansion save. The sandbox changes the storage
layout. Version 1 identifies sandbox records, not a migration scheme for old saves.

1. Obtain two eligible healthy Gen I-III Pokemon with no held items (not Shedinja).
2. Open R + Start, then Party, then **FF copy party**. Expect OK and the copy count.
3. Choose **FF fuse copies 1+2**. The first copy is the body; the second is the head.
4. Choose **FF preview base stats**. Note all six boosted base stats.
5. Choose **FF reverse copy 1** and preview again. Verify orientation changes.
6. Save through the normal game menu. Fully close/reopen the ROM, load the save,
   then choose **FF validate copies** and preview. Do NOT recapture the party.
7. Choose **FF split copy 1**. The partner returns at the end of the copy party.
8. Verify the original live party has remained unchanged throughout.

Capture refuses to overwrite a sandbox with active fusions. Split first before
recapture. Before the initial capture, other actions report uninitialized/invalid.
The sandbox is initialized empty on new game. Unknown nonzero versions are rejected.

## Storage and integrity

- An appended `FiniteFusionNativeSave` owns six full party copies, four full partner
  records, a header, count, ownership links and orientation flags. Official Kyurem,
  Necrozma and Calyrex slots are unchanged.
- The payload is part of PokemonStorage and therefore included in its existing
  flash-sector checksums and save-slot machinery. The existing storage-size assert
  and linker RAM limits remain enabled. No limits are relaxed.
- Box checksum validation is read-only; getters operate on local copies because
  some existing getters can mutate the original record into a Bad Egg on failure.
- Lifecycle mutations only touch sandbox buffers. Pointer-bearing context structs
  are reconstructed on each operation and are never saved.
- Preview reads original component species data and calls the shared 60/40 +10%
  calculation. It does not modify stored HP, XP, IVs, EVs or derived stats.

## Verification status

Host stat and lifecycle suites pass. A dedicated GitHub job compiles the opt-in
Emerald ROM with all existing size assertions and reports storage symbol sizes.
Native save/reset/load and menu behavior require the emulator checklist above;
do not interpret host tests or a successful compile as completing that checklist.
Local ARM installation was blocked by container permissions.

## Next step

After native compilation and emulator verification, design production per-mon
metadata and ownership across party/PC operations, integrate stat/HP/type handling,
and add restrictions before exposing fusion to live gameplay. This saved copy
sandbox is a diagnostic facility, not the final save format.
