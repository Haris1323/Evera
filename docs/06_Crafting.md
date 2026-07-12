# 06 · Crafting

> Status: nacrt (living document). Dio 0.1 (osnovno sakupljanje) → 0.2 (crafting stanice).

## Petlja

```
Sakupi sirovinu → (alat/stanica) → Napravi predmet → Koristi / prodaj / ugradi
```

## Sakupljanje (gathering) — 0.1

- **Drvo:** sječa stabala (sjekira) → trupci → daske
- **Kamen:** rudarenje stijena (pijuk) → kamen → obrađeni kamen
- **Bobice/hrana:** branje rukom
- **Voda:** zahvatanje iz rijeke

Svaka radnja daje XP odgovarajućoj vještini (vidi [03_Player.md](03_Player.md)).

## Alati

Alati ubrzavaju i otključavaju sakupljanje. Troše se (durability) i popravljaju/prave se.

| Alat | Za | Napravljen od |
|---|---|---|
| Sjekira | drvo | drvo + kamen |
| Pijuk | kamen/rudu | drvo + kamen |
| Štap za pecanje | riba | drvo + vlakno |

## Recepti (crafting)

- Recept = ulazni materijali → izlazni predmet (+ potrebna stanica/vještina).
- Veća vještina → bolji kvalitet, manje otpada, novi recepti.
- **Tehnički (nacrt):** recepti kao **DataTable** (`FCraftingRecipe` struct) da se lako dodaju bez koda.

## Crafting stanice (0.2+)

Vatra (kuhanje), radni sto (alati), peć (topljenje rude), itd.

## Otvorena pitanja

- [ ] Recepti u DataTable ili DataAsset? (vjerovatno DataTable za masu, DataAsset za posebne)
- [ ] Durability alata — koliko strog da ne bude dosadno?
- [ ] Da li igrač "uči" recepte ili su svi dostupni od početka?
