# 02 · World

> Status: nacrt (living document)

## Filozofija svijeta

Svijet je **dizajniran s namjerom**, ne nasumično generisan. Velik, ali ručno oblikovan
tako da svako mjesto ima karakter. Priroda je živa: rijeke teku, drveće se njiše, životinje žive.

## Biomi (početni set)

| Biom | Opis | Resursi |
|---|---|---|
| Šuma | gusta, topla, početna zona | drvo, bobice, divljač |
| Rijeka / jezero | voda, ribolov | voda, riba, glina |
| Livada | otvoreno, za farme | trava, tlo za usjeve |
| Planina | kamen, rude | kamen, ruda, minerali |
| Obala | plaža, more | pijesak, školjke, riba |

## Skala

- Cilj za prototip: **jedno manje ostrvo / dolina** (dovoljno za 0.1–0.2).
- Kasnije: povezani regioni, pa kontinent.
- **Ne** krećemo od ogromnog svijeta — krećemo od malog, ispoliranog.

## Dan / noć i vrijeme

Vidi [10_Weather.md](10_Weather.md). Ukratko: dinamičan dan/noć ciklus i vremenske prilike
utiču na gameplay (hladnoća, kiša, vidljivost).

## Persistencija svijeta

Svijet pamti promjene igrača (izgrađene kuće, posječeno drveće koje ponovo raste, itd.).
Tehnički detalji: vidi [technical/server-architecture.md](technical/server-architecture.md).

## Otvorena pitanja

- [ ] Fiksna mapa ili proceduralno-potpomognuta ručna izrada?
- [ ] Koliko velik je "dovoljno velik" za MVP?
- [ ] Kako drveće/resursi respawnuju bez da svijet izgleda mrtvo ili prenapučeno?
