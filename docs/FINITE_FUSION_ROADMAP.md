# Finite Fusion project roadmap

This is a living plan. Creative decisions stay open until the core development loop is proven.

## Phase 0 — Toolchain

- [ ] Install WSL2 and dependencies.
- [ ] Clone this fork and configure the `upstream` remote.
- [ ] Run `bash scripts/check-setup.sh`.
- [ ] Build `pokeemerald.gba`.
- [ ] Boot the build in mGBA.
- [ ] Install VS Code and Porymap.
- [ ] Make, compile, and test one visible text change.

**Exit condition:** a clean clone can be compiled and a small source change appears in-game.

## Phase 1 — Game brief

Decide these before building large amounts of content:

- working title and one-sentence premise;
- original region versus modified Hoenn;
- intended game length;
- generation roster and regional Pokédex size;
- starter choices;
- tone, difficulty, and target audience;
- major battle mechanics and generational gimmicks;
- level cap, experience, encounter, and evolution philosophy;
- approximate number of towns, routes, gyms, and major story beats.

**Exit condition:** a one-page design brief and a deliberately small vertical-slice scope.

## Phase 2 — Vertical slice

Build one polished segment containing:

- player introduction and starter selection;
- one town, one route, and one dungeon or landmark;
- wild encounters;
- several NPC scripts;
- at least two regular trainers and one important battle;
- one quest or story event;
- healing, saving, map transitions, and progression flags;
- a beginning and end suitable for playtesting.

**Exit condition:** a new player can complete a coherent 20–30 minute slice without developer intervention.

## Phase 3 — Production foundation

Before expanding the world:

- establish naming and content conventions;
- document how maps, scripts, graphics, trainers, and encounters are added;
- decide save-data compatibility expectations;
- create a repeatable smoke-test checklist;
- maintain credits for upstream code and imported assets;
- use feature branches and pull requests for meaningful changes.

## Phase 4 — Full production

Produce the game in small, playable increments. Each increment should include maps, scripts, battles, encounters, graphics, and testing for one connected section of progression.

## Near-term backlog

1. Complete Phase 0.
2. Replace this generic roadmap with the chosen game concept.
3. Locate and change one opening text string.
4. Create the first project-specific branch.
5. Configure Porymap against the WSL checkout.
6. Draft the regional Pokédex and first vertical slice.

## Guardrails

- Never commit commercial ROM files or generated `.gba` builds.
- Keep attribution required by `CREDITS.md`.
- Avoid large engine modifications until the vertical slice proves they are necessary.
- Pull upstream updates on dedicated maintenance branches and test them before merging.
- Commit source assets and editable originals where licensing permits.
