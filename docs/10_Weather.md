# 10 · Weather

> Status: nacrt (living document). Dan/noć + kiša u 0.1, godišnja doba kasnije.

## Elementi

- **Dan / noć ciklus** — utiče na vidljivost, NPC rutine, temperaturu.
- **Vrijeme:** sunce, oblaci, kiša, magla, (kasnije snijeg, oluje).
- **Godišnja doba** (kasnije): proljeće/ljeto/jesen/zima — utiču na farmu, izgled svijeta.

## Uticaj na gameplay

| Pojava | Efekat |
|---|---|
| Noć | hladnije, slabija vidljivost, neki NPC spavaju |
| Kiša | puni vodu, gasi vatru na otvorenom, mokro = higijena/hladnoća |
| Zima (kasnije) | usjevi ne rastu, treba grijanje |

Ne smije biti frustrirajuće — vrijeme dodaje atmosferu i planiranje, ne kaznu.

## Tehnika (nacrt)

- UE **Sky Atmosphere** + **Volumetric Clouds** + dinamičko sunce (Directional Light).
- Sistem koji vodi vrijeme dana i bira vremenske prilike (weather state machine).
- Sinhronizacija u multiplayeru (svi vide isto vrijeme) → Weather server kasnije.

## Otvorena pitanja

- [ ] Dužina dana (realno vs. skraćeno)?
- [ ] Koliko godišnja doba utiču — kozmetički ili duboko mehanički?
