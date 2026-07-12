# Unreal Engine Conventions (Evera)

> Kako organizujemo Content/ i imenujemo assete. Pratimo poznati **Allar UE Style Guide**.

## Prefiksi asseta

| Tip | Prefiks | Primjer |
|---|---|---|
| Blueprint | `BP_` | `BP_Campfire` |
| Blueprint (bazna klasa) | `BP_` | `BP_EveraCharacter` |
| Material | `M_` | `M_Wood` |
| Material Instance | `MI_` | `MI_Wood_Dark` |
| Texture | `T_` | `T_Wood_BaseColor` |
| Static Mesh | `SM_` | `SM_Log` |
| Skeletal Mesh | `SK_` | `SK_Deer` |
| Widget (UMG) | `WBP_` | `WBP_Inventory` |
| Data Table | `DT_` | `DT_CraftingRecipes` |
| Data Asset | `DA_` | `DA_ToolAxe` |
| Sound Cue | `SC_` / `MS_` | `MS_Footsteps` |
| Level / Map | `L_` | `L_ForestPrototype` |
| Enum | `E_` | `E_ResourceType` |
| Struct | `F_` / `S_` | `S_ItemData` |

## Organizacija Content foldera

```
Content/
├── Evera/
│   ├── Core/          # game mode, player, osnovne klase
│   ├── Characters/    # likovi, animacije
│   ├── Systems/       # survival, crafting, building, inventory
│   ├── Items/         # predmeti, alati, resursi
│   ├── World/         # mape, okolina, foliage
│   ├── UI/            # widgeti
│   ├── Audio/         # zvuk, muzika
│   └── Data/          # data tables, data assets
└── (template/starter folderi po potrebi)
```

**Pravilo:** sve NAŠE ide pod `Content/Evera/`. Kupljeni/marketplace asseti ostaju u
svojim folderima da se lako ažuriraju/uklone.

## Ostalo

- Nema razmaka ni specijalnih znakova u imenima asseta/foldera.
- Nema "New Folder", "Untitled", "MyBlueprint" — imenuj odmah.
- C++ bazne klase, Blueprint child klase za podešavanja (data + brzu iteraciju).
