# assets/

**Sirovi** asseti prije nego uđu u Unreal (izvorni fajlovi iz Blender/GIMP/Krita, zvuk).

> Razlika od `game/Evera/Content/`: ovdje su IZVORNI/radni fajlovi (`.blend`, `.psd`, `.wav`).
> U `Content/` idu importovani/gotovi Unreal asseti. Ovako čuvamo originale.

## Organizacija

```
assets/
├── models/       # .blend, .fbx (3D modeli)
├── textures/     # .psd, .png, .tga
├── audio/        # .wav, .mp3, .ogg
└── reference/    # reference slike
```

Imenovanje: `kategorija_ime_varijanta.ext` (npr. `tree_oak_01.fbx`, `wood_bark_basecolor.png`).

> Svi veliki binarni fajlovi idu kroz **Git LFS** (podešeno u `.gitattributes`).
