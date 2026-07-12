# DECISIONS — Dnevnik odluka (ADR)

> Ovdje bilježimo VAŽNE odluke i ZAŠTO smo ih donijeli. Kad se za 6 mjeseci pitamo
> "zašto smo ovo tako uradili?", odgovor je ovdje. Novi zapisi idu na vrh.

Format:
```
## [broj] Naslov — datum
**Odluka:** šta smo odlučili
**Zašto:** razlog
**Alternative:** šta smo razmatrali
```

---

## [0004] Proprietary licenca, ne open-source — 2026-07-13
**Odluka:** Repo koristi proprietary "All Rights Reserved" licencu (ne MIT/Apache).
**Zašto:** Evera je komercijalna igra. Open-source licenca bi dozvolila bilo kome da
legalno kopira i objavi našu igru.
**Alternative:** MIT (odbačeno — daje prava kopiranja); dual-license (nepotrebno sada).

## [0003] Monorepo struktura — 2026-07-13
**Odluka:** Sve u jednom repou: `game/` (UE), `server/`, `docs/`, `website/`, `assets/`.
**Zašto:** Jedno mjesto istine, lakše praćenje, profesionalan izgled. Git LFS rješava
problem velikih binarnih fajlova.
**Alternative:** Odvojeni repoi po komponenti (odbačeno — komplikuje za mali tim).

## [0002] C++ projekat (ne Blueprint-only) — 2026-07-13
**Odluka:** Kreiramo UE **C++** projekat iz Third Person template-a.
**Zašto:** Vizija (multiplayer, ekonomija, NPC AI, serveri) zahtijeva C++. Blueprintove
i dalje koristimo povrh C++. Bolji temelj, izbjegava bolnu migraciju kasnije.
**Alternative:** Blueprint-only (odbačeno — ograničava dubinu i backend integraciju).

## [0001] Unreal Engine 5.8 — 2026-07-13
**Odluka:** Engine je Unreal Engine 5.8.
**Zašto:** Najbolja grafika u industriji, odličan za open-world, vrhunska PS5 podrška,
kasnije Xbox/PC, snažan multiplayer framework, C++ + Blueprint.
**Alternative:** Unity (odbačeno — slabije za AAA grafiku i naš server-orijentisan pristup).
