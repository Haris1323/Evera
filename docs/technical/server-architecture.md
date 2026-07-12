# Technical · Server Architecture

> Status: vizija (living document). **Ništa od ovoga nije za sada** — kreće tek nakon
> što singleplayer/co-op radi (0.3+). Ovdje da imamo plan kad dođe vrijeme.

## Princip

UE je **server-authoritative**: dedicated server je izvor istine, klijenti prikazuju.
Backend servisi (Login, Economy, itd.) su ODVOJENI od UE server procesa i komuniciraju s njim.

## Servisi (ciljna arhitektura)

```
                 ┌──────────────┐
   PS5 klijent → │ Login Server │ → autentikacija, sesije
                 └──────┬───────┘
                        │
   ┌────────────────────┼─────────────────────┐
   │            ┌───────────────┐              │
   │            │ World Server  │  UE dedicated (gameplay, replikacija)
   │            └───────┬───────┘              │
   │   ┌────────────────┼───────────────┐      │
┌──┴──┐ ┌────────┐ ┌────┴─────┐ ┌───────┴──┐ ┌─┴──────┐
│Chat │ │Economy │ │   NPC    │ │ Weather  │ │Analytics│
└─────┘ └────────┘ └──────────┘ └──────────┘ └────────┘
```

| Servis | Uloga |
|---|---|
| Login | nalozi, autentikacija, sesije |
| World | UE dedicated — gameplay, replikacija, persistencija svijeta |
| Chat | poruke, filtriranje (family-safe) |
| Economy | cijene, transakcije, imovina |
| NPC | životni ciklus i ponašanje van vidokruga igrača |
| Weather | sinhronizovano vrijeme za sve |
| Analytics | metrika, balans, praćenje grešaka |

## Tehnologije (kandidati)

- **Backend servisi:** Go ili C# (.NET) — brzo, dobro za servere.
- **Baza:** PostgreSQL (trajni podaci) + Redis (brzi keš, sesije).
- **Komunikacija:** gRPC ili REST između servisa; UE ↔ backend preko sigurnog API-ja.
- **Hosting:** kontejneri (Docker) → orkestracija kad naraste.

## Faze

1. **0.3:** UE listen/dedicated server, bez zasebnog backenda (sve u UE).
2. **Kasnije:** izdvoji Login + Economy + persistencija u zaseban servis.
3. **Daleko:** puna mikroservisna arhitektura + skaliranje.

## Otvorena pitanja

- [ ] PlayStation Network integracija (nalozi, prijatelji) — zahtjevi Sony-ja.
- [ ] Kako persistirati velike svjetove efikasno?
- [ ] Region hosting / latencija.
