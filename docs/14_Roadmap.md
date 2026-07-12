# 14 · Roadmap

> Pravilo br. 1: gradimo po MALIM verzijama koje se mogu IGRATI.
> Bolje mala igra koja radi danas, nego "savršena" igra za 5 godina koju niko nije vidio.

---

## ✅ Faza 0 — Setup (u toku)

- [x] Instaliran Unreal Engine 5.8
- [x] Instaliran Git + Git LFS
- [x] Napravljena repo struktura + dokumentacija
- [ ] Instaliran Visual Studio (Game development with C++)
- [ ] Napravljen UE C++ projekat "Evera" (Third Person template) u `game/`
- [ ] Git repo povezan sa GitHub-om (+ LFS)
- [ ] Prvi build prolazi (zeleno)

## 🎯 Evera 0.1 — Preživljavanje

Cilj: sam u šumi, osnovno preživljavanje. Ništa više.

- [ ] Kretanje (WASD + skok) — iz template-a
- [ ] Statovi: glad, žeđ, energija, zdravlje (`USurvivalStatsComponent`)
- [ ] Sakupljanje drveta i kamena
- [ ] Inventar (jednostavan)
- [ ] Vatra
- [ ] Prva koliba (postavljanje trupaca)
- [ ] Dan/noć + kiša
- [ ] Save / load igre

## 🎯 Evera 0.2 — Život

- [ ] Pecanje
- [ ] Životinje (divlje)
- [ ] Higijena i spavanje
- [ ] Kuhanje (vatra kao stanica)
- [ ] Prvi NPC trgovac
- [ ] Modularno građenje (zidovi/krov/vrata)

## 🎯 Evera 0.3 — Zajednica

- [ ] Multiplayer (2+ igrača, listen server)
- [ ] Trgovina među igračima
- [ ] Prvi poslovi
- [ ] Privatni serveri

## 💤 Kasnije (ne sada — da ne izgubimo fokus)

- Vozila, avioni, vozovi
- Napredna ekonomija i biznisi
- NPC životni ciklus i pamćenje
- Dedicated serveri (backend — `server/`)
- PS5 build & PlayStation Partners submisija

---

### Kako radimo (proces)
1. Uzmemo JEDNU nečekiranu stavku iz trenutne faze.
2. Ja napišem C++/config, ti uradiš editor dio uz moje vođenje.
3. Testiramo da radi → commit → sljedeća stavka.

### Verzioniranje
Pratimo promjene u [`CHANGELOG.md`](../CHANGELOG.md). Format verzija: `0.MAJOR.minor`.
