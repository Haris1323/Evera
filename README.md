<div align="center">

# EVERA

**Build Your Life. Shape Your World.**

Family-friendly online life-simulation & sandbox igra za PlayStation 5.

`Open World` · `Sandbox` · `Life Sim` · `Co-op Multiplayer`

</div>

---

Igrač počinje bez ičega i gradi cijeli život kroz kreativnost, saradnju i napredovanje —
bez borbe kao fokusa. Postaješ ono što radiš. Svaka stvar ima vrijednost jer ju je neko napravio.

- **Studio:** NoLimit Developments FZ-LLC (Dubai)
- **Engine:** Unreal Engine 5.8 (C++ + Blueprints)
- **Platforma:** PlayStation 5 (kasnije PC / Xbox)

## 📚 Dokumentacija

| | |
|---|---|
| [00 · Project Vision](docs/00_Project_Vision.md) | Zašto Evera postoji |
| [01 · Game Design Bible](docs/01_Game_Design_Bible.md) | Glavni indeks svih sistema |
| [14 · Roadmap](docs/14_Roadmap.md) | Plan razvoja 0.1 → 0.2 → 0.3 |
| [DECISIONS](docs/DECISIONS.md) | Zapis svih važnih odluka |

Svi dokumenti su u [`docs/`](docs/). Standardi kodiranja u [`docs/conventions/`](docs/conventions/).

## 🗂️ Struktura repozitorija

```
EVERA/
├── docs/          # sva dokumentacija (dizajn, standardi, tehnika)
├── game/          # Unreal Engine 5 projekat (C++ + Content)
├── server/        # dedicated serveri / backend (kasnije)
├── tools/         # pomoćni skripti i alati
├── website/       # landing stranica studija
├── concept-art/   # koncept ilustracije
└── assets/        # sirovi asseti (modeli, teksture, zvuk)
```

## 🚀 Setup na novom kompjuteru

1. Instaliraj: **Unreal Engine 5.8**, **Visual Studio 2022/2026** (workload: *Game development with C++*), **Git**, **Git LFS**
2. `git lfs install`
3. `git clone <repo>` i uđi u folder
4. Desni klik na `game/Evera/Evera.uproject` → **Generate Visual Studio project files**
5. Dupli klik na `Evera.uproject` → kad pita da build-a, klikni **Yes**

## 📄 Licenca

Proprietary — sva prava zadržana. Vidi [LICENSE](LICENSE).

<div align="center"><sub>© 2026 NoLimit Developments FZ-LLC</sub></div>
