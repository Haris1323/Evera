# 05 · Economy

> Status: vizija (living document). Tehnička razrada: [technical/server-architecture.md](technical/server-architecture.md)

## Glavni princip

**Svaka stvar ima vrijednost jer ju je neko napravio.** Nema magičnog "shop-a" gdje se
predmeti pojave niotkuda. Ako postoji hljeb, neko je posijao žito, neko samljeo brašno,
neko ispekao. To je duša Everine ekonomije.

## Tokovi vrijednosti

```
Sirovina → Obrada → Proizvod → Transport → Prodaja → Novac → Nova sirovina/alat
```

## Uloge u ekonomiji

- **Proizvođač** — pravi sirovine i predmete (drvosječa, farmer, rudar).
- **Zanatlija** — pretvara sirovine u vrijedne predmete.
- **Trgovac** — kupuje jeftino, prodaje skuplje, drži radnju.
- **Transporter** — vozi robu (kola, brod, voz, avion).
- **Poslodavac** — otvara biznis, zapošljava NPC-jeve ili igrače, plaća platu.

## Novac i cijene

- Cijene se formiraju ponudom i potražnjom (nacrt — počinjemo jednostavno).
- Nema pay-to-win: pravi novac se ne može kupiti da bi se dobila prednost.

## Tehnologija kao roba

Tehnologija se **otkriva** kroz igru i može se:
- koristiti samo za sebe,
- podijeliti besplatno,
- prodati ili licencirati drugim igračima.

Ne otključava se globalno za sve — to čini znanje vrijednim. Vidi [08_Technology.md](08_Technology.md).

## Faze implementacije

1. **0.2:** jedan NPC trgovac, fiksne cijene, kupi/prodaj.
2. **0.3:** trgovina među igračima.
3. **Kasnije:** poslovi, plate, transport, dinamičke cijene, Economy server.

## Otvorena pitanja

- [ ] Kako spriječiti inflaciju u trajnom svijetu?
- [ ] Global ekonomija po serveru ili po regiji?
- [ ] Šta se dešava s imovinom kad igrač dugo ne igra?
