EVERA AXE — UE5 IMPORT NOTES

Files
- SM_EVERA_Axe.glb: recommended asset
- SM_EVERA_Axe.obj: OBJ fallback
- UCX_SM_EVERA_Axe_00.obj: simplified collision mesh

Scale
- Modeled in centimeters
- Approximate overall length: 72 cm

Unreal Engine 5
1. Import SM_EVERA_Axe.glb as Static Mesh.
2. Preserve hierarchy if separate parts are useful.
3. Replace placeholder colors with PBR materials:
   M_Axe_Steel, M_Axe_Wood, M_Axe_Leather.
4. Use 2K textures for gameplay and 4K for hero renders.
5. Suggested maps: Base Color, Normal, ORM.
6. Nanite can be enabled for hero/display usage; profile before using it
   for large numbers of replicated gameplay objects.
7. Import the UCX mesh or build simple collision in Unreal.

Design direction
- Warm stylized realism
- Clear silhouette at gameplay distance
- Practical woodworking tool, family-friendly presentation
