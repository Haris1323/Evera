# 11 · Multiplayer

> Status: vizija (living document). Prvi multiplayer u 0.3. Tehnika: [technical/server-architecture.md](technical/server-architecture.md)

## Cilj

Evera je zamišljena kao **zajednički, trajni svijet** — porodica i prijatelji zajedno.
Ali kreće postepeno: prvo solidan singleplayer, pa co-op, pa pravi serveri.

## Modeli igranja

1. **Privatni svijet / server** — porodica i prijatelji, pozivnice. **Primarni fokus.**
2. **Listen server / co-op** — jedan igrač hostuje, drugi se pridruže (0.3 start).
3. **Dedicated serveri** — trajni svijetovi (kasnije, backend).

## Sigurnost (family-first)

- **Roditeljske kontrole:** ko može ući, chat filter, vrijeme igranja.
- **Privatni serveri** znače da djeca ne igraju s nepoznatima osim ako roditelj dozvoli.
- Bez pay-to-win, bez agresivne monetizacije.

## Tehnički pristup

- UE koristi **replikaciju** (server-authoritative) — server je izvor istine.
- Karakter, potrebe, inventar, građevine se repliciraju.
- **Bitno:** kod pišemo multiplayer-svjestan od početka (autoritet, replikacija),
  čak i dok testiramo solo. Migracija kasnije je bolna ako se to ignoriše.
- Odvojeni servisi (Login, World, Chat, Economy, NPC...) — vidi
  [technical/server-architecture.md](technical/server-architecture.md).

## Faze

1. **0.3:** dva igrača zajedno (listen server), osnovna trgovina.
2. **Kasnije:** dedicated serveri, privatni svjetovi s pozivnicama.
3. **Daleko:** skaliranje, više servisa, PS5 online.

## Otvorena pitanja

- [ ] Koliko igrača po svijetu ciljamo?
- [ ] Kako se persistira svijet između sesija?
- [ ] P2P ili uvijek dedicated za prave svjetove?
