# 03 · Player

> Status: nacrt (living document) — ovo je prvi sistem koji kodiramo u 0.1

## Potrebe (needs)

Osnovne potrebe koje tjeraju gameplay. Ne smiju biti frustrirajuće — planiranje, ne kazna.

| Potreba | Opada kroz | Vraća se | Posljedica na nuli |
|---|---|---|---|
| Glad (Hunger) | vrijeme, rad | jelo | gubi zdravlje |
| Žeđ (Thirst) | vrijeme, vrućina | voda | gubi zdravlje |
| Energija (Energy) | rad, kretanje | spavanje, odmor | sporije kretanje |
| Zdravlje (Health) | glad/žeđ/povrede | regen kad su potrebe OK | onesposobljenje |
| Higijena (Hygiene) | vrijeme, prljav rad | pranje | utiče na raspoloženje/NPC |

**Tehnički:** implementira se kao `USurvivalStatsComponent` (ActorComponent) na igraču.
Vidi [conventions/unreal-conventions.md](conventions/unreal-conventions.md) za imenovanje.

## Vještine (skills) — BEZ klasa

Ovo je srce Evere. Nema izbora klase. Vještina raste kroz **upotrebu**.

- Sječeš drvo → raste **Woodcutting**
- Pecaš → raste **Fishing**
- Gradiš → raste **Building**
- Kuhaš → raste **Cooking**
- Liječiš → raste **Medicine**
- Letiš → raste **Piloting**

Veća vještina = brže, kvalitetnije, manje otpada, novi recepti. Igra **ne** slavi level-up
velikim natpisom — napredak se osjeti, ne najavljuje.

### Model napretka (nacrt)
- Svaka vještina ima nevidljivi XP i "nivo" (npr. 1–100).
- Kriva napretka: brzo na početku, sporije kasnije.
- Nema respawn/reset — trajno rasteš.

## Kontrole (0.1, iz Third Person template-a)

WASD kretanje, miš kamera, Space skok, E interakcija (dodaćemo).

## Otvorena pitanja

- [ ] Da li potrebe teku i offline (multiplayer)? Vjerovatno usporeno ili pauzirano.
- [ ] Koliko vještina u startu? (0.1 samo Woodcutting + Mining da testiramo sistem)
- [ ] Character creator — koliko rano? (vjerovatno 0.2+)
