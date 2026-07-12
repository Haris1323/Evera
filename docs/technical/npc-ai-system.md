# Technical · NPC AI System

> Status: vizija (living document). Dizajn-pregled je u [../04_NPC.md](../04_NPC.md).
> Ovdje je tehnički plan. Kreće u 0.2 (prosti trgovac), puni sistem mnogo kasnije.

## Slojevi

1. **Podaci (NPC state)** — ime, godine, posao, potrebe, raspoloženje, odnosi, pamćenje.
2. **Odlučivanje (decision)** — šta NPC radi sada.
3. **Akcija (action)** — kretanje, animacija, interakcija.
4. **Simulacija van vidokruga** — NPC-jevi "žive" i kad ih igrač ne gleda (jeftinije, apstraktno).

## Odlučivanje: Utility AI (preporuka)

Umjesto krutog behavior tree-a, **Utility AI** boduje opcije prema potrebama:
- Gladan? → jedi (skor raste s glađu)
- Umoran? → spavaj
- Radno vrijeme? → idi na posao
- Usamljen? → druži se

NPC bira akciju s najvišim skorom. Prirodnije, lakše za balans.
(Behavior Tree se može koristiti za same akcije/rutine.)

## Pamćenje (memory)

- Svaki NPC ima ograničen "dnevnik" značajnih događaja (npr. "igrač mi dao jabuku +5 naklonost").
- Događaji stare/blijede osim ako su jaki.
- Naklonost prema igraču utiče na cijene, dijaloge, pomoć.

## Životni ciklus

Konačni automat: `rođenje → dijete → škola → posao → partner → djeca → starost → smrt`.
Vremenski vođen (skalirano na dužinu igre). Nove generacije popunjavaju svijet.

## Performanse (ključni izazov)

- **LOD za AI:** blizu igrača = puna simulacija; daleko = apstraktna/statistička.
- Limit aktivnih "punih" NPC-jeva; ostali simulirani jeftino na serveru.
- Teško pitanje: koliko pamćenja čuvati bez eksplozije podataka.

## Dijalozi (daleka budućnost)

- Prvo: skriptovani/šablonski dijalozi s varijablama (ime, sjećanja).
- Možda kasnije: AI-generisani dijalozi (lokalno ili server), ali tek kad igra radi.

## Faze

1. **0.2:** jedan statični NPC trgovac. Bez rutina, bez ciklusa.
2. **Kasnije:** Utility AI, dnevne rutine, raspoloženja.
3. **Daleko:** odnosi, pamćenje, puni životni ciklus.
