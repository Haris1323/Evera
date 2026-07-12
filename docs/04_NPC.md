# 04 · NPC

> Status: vizija (living document) — ovo je "wow faktor" Evere, ali dolazi kasnije (0.2+).
> Duboka tehnička razrada: [technical/npc-ai-system.md](technical/npc-ai-system.md)

## Cilj

NPC-jevi nisu ukrasi. Oni **žive**. Svijet se kreće i kad igrač ne gleda.

## Atributi NPC-a

Svaki NPC ima:
- ime, godine, spol, izgled
- posao i platu
- porodicu (roditelji, partner, djeca)
- raspoloženje, umor, zdravlje, glad
- želje i ciljeve (npr. "hoću veću kuću", "hoću dijete")
- odnos prema igraču i drugim NPC-jevima

## Životni ciklus

```
rođenje → djetinjstvo → škola → posao → partner → djeca → starost → smrt
```

Svijet se obnavlja sam: nove generacije se rađaju, stare odlaze.

## Pamćenje (memory)

NPC pamti postupke igrača. Primjer:

> Daš NPC-u jabuku. Kaže "Hvala!".
> Tri sedmice kasnije: *"Noah! Još pamtim onu jabuku koju si mi dao."*

Ovo stvara emocionalnu vezu koju malo igara ima.

## Ponašanje (nacrt tehnike)

- **Utility AI** ili **behavior tree** za dnevne rutine (posao, jelo, san, druženje).
- Raspored dana: NPC ide na posao, pauzu, kući.
- Reakcije na svijet: kiša → traži zaklon; glad → jede.

## Faze implementacije

1. **0.2:** jedan statični NPC trgovac (kupi/prodaj). Bez životnog ciklusa.
2. **Kasnije:** rutine, raspoloženja, odnosi.
3. **Daleko:** puni životni ciklus + pamćenje + (možda) AI-generisani dijalozi.

## Otvorena pitanja

- [ ] Koliko NPC-jeva svijet može podnijeti (performanse)?
- [ ] Pamćenje: koliko duboko prije nego postane preskupo za čuvati?
- [ ] AI dijalozi lokalno ili preko servera?
