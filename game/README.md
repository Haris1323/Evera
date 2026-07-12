# game/

Ovdje živi **Unreal Engine 5.8 projekat** (Evera).

## Kako se kreira (jednom, nakon instalacije Visual Studio-a)

1. Epic Games Launcher → Unreal Engine → **Launch 5.8**
2. **Games → Third Person**
3. Podešavanja: **C++** (ne Blueprint), **Maximum** kvalitet, **Starter Content: isključeno**
4. **Project Name:** `Evera`
5. **Project Location:** `C:\Users\xproj\Desktop\EVERA\game`
6. **Create** → dobićeš `game/Evera/Evera.uproject`

Nakon toga ću ja povezati git, LFS i početi dodavati Evera C++ sisteme.

## Struktura (nakon kreiranja)

```
game/Evera/
├── Evera.uproject
├── Config/
├── Content/Evera/   ← naši asseti (vidi docs/conventions/unreal-conventions.md)
└── Source/Evera/    ← C++ kod
```

> Napomena: `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` se NE commit-uju
> (već su u `.gitignore`).
