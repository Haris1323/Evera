# C++ Style Guide (Evera)

> Pratimo **Epic Games / Unreal** stil. Kad si u dilemi — kako Unreal izvorni kod radi.

## Imenovanje (Unreal prefiksi su OBAVEZNI)

| Tip | Prefiks | Primjer |
|---|---|---|
| Klasa iz `UObject` | `U` | `USurvivalStatsComponent` |
| Klasa iz `AActor` | `A` | `AEveraCharacter` |
| `struct` | `F` | `FCraftingRecipe` |
| `enum class` | `E` | `EResourceType` |
| Interface | `I` | `IInteractable` |
| Template | `T` | `TArray` |
| Boolean varijabla | `b` | `bIsAlive` |

## Formatiranje

- **Braces:** Allman stil (otvorena vitičasta u novom redu) — kao Unreal.
- **Indentacija:** tabovi (Unreal default), ne razmaci.
- **Jedna klasa = jedan `.h` + jedan `.cpp`**, isto ime.
- `#pragma once` na vrhu svakog header-a.

## Praksa

- Koristi Unreal tipove: `FString`, `TArray`, `TMap`, `int32`, `float` (ne goli `std::`).
- `UPROPERTY()` / `UFUNCTION()` gdje treba (reflection, GC, editor, replikacija).
- Pametni pokazivači: `TObjectPtr<>`, `TWeakObjectPtr<>` — ne goli `new/delete`.
- Komentari na engleskom u kodu (standard), objašnjenja/dogovori mogu na bosanskom u docs.
- `check()` / `ensure()` za pretpostavke; ne tihi fail.

## Multiplayer od početka

- Razmišljaj o autoritetu: `HasAuthority()`, `UPROPERTY(Replicated)`, RPC (`Server`/`Client`/`Multicast`).
- Ne mijenjaj gameplay stanje na klijentu bez servera kao autoriteta.

## Primjer

```cpp
UCLASS()
class EVERA_API USurvivalStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
    float Hunger = 100.f;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float Amount);

private:
    bool bIsStarving = false;
};
```
