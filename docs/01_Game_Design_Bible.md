# 01 · Game Design Bible

> Glavni indeks. Ovaj dokument povezuje sve ostale i opisuje kako se sistemi uklapaju.
> Svaki sistem ima svoj detaljan dokument — ovdje je pregled.

## Sadržaj

| # | Dokument | Sistem |
|---|---|---|
| 00 | [Project Vision](00_Project_Vision.md) | Vizija i stubovi |
| 02 | [World](02_World.md) | Svijet, biomi, skala |
| 03 | [Player](03_Player.md) | Potrebe, vještine, napredak |
| 04 | [NPC](04_NPC.md) | AI likovi, pamćenje, životni ciklus |
| 05 | [Economy](05_Economy.md) | Vrijednost, trgovina, poslovi |
| 06 | [Crafting](06_Crafting.md) | Sakupljanje → izrada |
| 07 | [Building](07_Building.md) | Građenje domova i gradova |
| 08 | [Technology](08_Technology.md) | Otkrivanje i dijeljenje tehnologije |
| 09 | [Animals](09_Animals.md) | Divlje i domaće životinje |
| 10 | [Weather](10_Weather.md) | Vrijeme, godišnja doba, dan/noć |
| 11 | [Multiplayer](11_Multiplayer.md) | Serveri, privatni svjetovi, co-op |
| 12 | [UI / UX](12_UI_UX.md) | Interfejs i osjećaj |
| 13 | [Audio](13_Audio.md) | Zvuk i muzika |
| 14 | [Roadmap](14_Roadmap.md) | Plan po verzijama |

## Osnovna gameplay petlja

```
Skupljaš  →  Praviš alate  →  Gradiš sklonište  →  Zadovoljavaš potrebe
   ↑                                                        │
   └──────  Trguješ / radiš / napreduješ  ←─────────────────┘
```

Od kolibe do grada: `sklonište → dom → radionica → farma → biznis → selo → moderni grad`.

## Kako se sistemi uklapaju

- **Potrebe** (glad, žeđ, energija) tjeraju igrača da **sakuplja** i **pravi**.
- **Sakupljanje/crafting** hrani **ekonomiju** — sve što napraviš ima vrijednost.
- **Ekonomija** stvara **poslove**, a poslovi uključuju **NPC-jeve** i druge igrače.
- **Vještine** rastu kroz sve gore — bez klasa, samo kroz ponavljanje.
- **Multiplayer** pretvara sve ovo u zajednički, trajni svijet.

## Dizajn-principi (kompas za sve odluke)

1. **Napredak kroz stvaranje, ne uništavanje.**
2. **Trud stvara vrijednost** — ništa vrijedno nije besplatno, ali ništa nije frustrirajuće.
3. **Sigurno za djecu, duboko za odrasle.**
4. **Pokaži, ne reci** — svijet uči igrača kroz igru, ne kroz tutorijale.
5. **Malo ali kvalitetno** — radije jedan sistem koji radi savršeno nego deset polovičnih.
