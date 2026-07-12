# 12 · UI / UX

> Status: nacrt (living document).

## Princip

Interfejs mora biti **jasan djetetu od 7 godina**, a opet dovoljno moćan za dubok gameplay.
Malo teksta, puno ikona, jasne boje. Radi na **PS5 kontroleru** (ne samo miš/tastatura).

## Ekrani (nacrt)

| Ekran | Sadržaj |
|---|---|
| HUD | potrebe (glad/žeđ/energija), aktivni alat, prompt za interakciju |
| Inventar | predmeti, oprema, drag & drop |
| Crafting | recepti, materijali, dugme napravi |
| Mapa | svijet, markeri, dom |
| Trgovina | kupi/prodaj, cijene |
| Vještine | pregled napretka (nenametljivo) |
| Karakter | izgled, oprema |

## UX pravila

- **Kontroler-first** dizajn (PS5), miš/tastatura kao dodatak.
- Radijalni meniji za brzi izbor alata (dobro rade na kontroleru).
- Boja i ikona prije teksta.
- Bez zatrpavanja — pokaži samo ono što treba sad.

## Tehnika

- UE **UMG** (Widget Blueprints) za UI, C++ za logiku podataka.
- Skalabilno za razne rezolucije (PS5, PC).

## Otvorena pitanja

- [ ] Stil: realističan HUD ili topao, ilustrovan (bliže publici)?
- [ ] Koliko HUD-a sakriti radi imerzije?
