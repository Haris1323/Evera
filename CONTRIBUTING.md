# Contributing to Evera

> Ovo je **privatni projekat** NoLimit Developments FZ-LLC. Doprinosi su interni (tim).
> Ovaj dokument opisuje kako tim radi da bi sve ostalo uredno.

## Radni tok (workflow)

1. Uzmi zadatak iz [`docs/14_Roadmap.md`](docs/14_Roadmap.md) ili GitHub Issues.
2. Napravi granu: `feature/kratak-opis` ili `fix/kratak-opis` (nikad direktno na `main`).
3. Radi u malim koracima. Commit-uj često.
4. Otvori Pull Request u `dev`/`main` uz opis šta i zašto.
5. Testiraj da se **build-a i radi** prije mergea.

## Grane

- `main` — stabilno, uvijek se build-a i pokreće.
- `dev` — integracija aktivnog razvoja.
- `feature/*`, `fix/*` — pojedinačni zadaci.

## Commit poruke

Format: `tip: kratak opis` (vidi [folder-naming.md](docs/conventions/folder-naming.md)).
Primjeri: `feat: dodaj inventar`, `fix: glad ne ide ispod nule`, `docs: azuriraj GDD`.

## Standardi koda

- C++: [`docs/conventions/cpp-style.md`](docs/conventions/cpp-style.md)
- Unreal asseti/Content: [`docs/conventions/unreal-conventions.md`](docs/conventions/unreal-conventions.md)
- Imenovanje u repou: [`docs/conventions/folder-naming.md`](docs/conventions/folder-naming.md)

## Pravila za assete (VAŽNO)

- Veliki binarni fajlovi idu kroz **Git LFS** (već podešeno u `.gitattributes`).
- Prije prvog rada: `git lfs install`.
- Ne commit-uj `Binaries/`, `Intermediate/`, `Saved/` (već u `.gitignore`).

## Odluke

Svaku VAŽNU tehničku/dizajn odluku zapiši u [`docs/DECISIONS.md`](docs/DECISIONS.md).
