# Folder & File Naming (repo)

> Kako imenujemo foldere/fajlove u REPOU (ne u Unreal Content — to je u
> [unreal-conventions.md](unreal-conventions.md)).

## Repo folderi (top-level)

- `docs/` `game/` `server/` `tools/` `website/` `concept-art/` `assets/`
- Sve **lowercase**, bez razmaka. Više riječi → `kebab-case` (npr. `concept-art`).

## Dokumenti

- Dizajn-biblija: `NN_Naslov.md` (npr. `05_Economy.md`) — broj drži redoslijed.
- Ostali docs: `kebab-case.md` (npr. `server-architecture.md`).
- Naslovi unutar fajla na engleskom (tehnički termini), tekst može bosanski.

## Asseti (sirovi, u `assets/`)

```
assets/
├── models/       # .fbx, .blend
├── textures/     # .png, .tga, .psd
├── audio/        # .wav, .mp3, .ogg
└── reference/    # reference slike, mood board
```
Imenovanje: `kategorija_ime_varijanta.ext` (npr. `tree_oak_01.fbx`).

## Grane (git branches)

- `main` — stabilno, uvijek se build-a.
- `dev` — aktivni razvoj.
- `feature/kratak-opis` — nova funkcija (npr. `feature/survival-stats`).
- `fix/kratak-opis` — ispravka.

## Commit poruke

`tip: kratak opis` — npr. `feat: dodaj SurvivalStatsComponent`, `docs: azuriraj roadmap`,
`fix: popravi glad ispod nule`. Tipovi: `feat`, `fix`, `docs`, `refactor`, `chore`, `art`.
