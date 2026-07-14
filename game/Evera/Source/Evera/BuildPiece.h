// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildPiece.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

/** The kinds of modular pieces the player can build a house from. */
UENUM(BlueprintType)
enum class EBuildPieceType : uint8
{
	Floor    UMETA(DisplayName = "Floor"),
	Wall     UMETA(DisplayName = "Wall"),
	Doorway  UMETA(DisplayName = "Doorway"),
	Window   UMETA(DisplayName = "Window"),
	Roof     UMETA(DisplayName = "Roof"),
	Pillar   UMETA(DisplayName = "Pillar"),
	Fence    UMETA(DisplayName = "Fence"),
	Campfire UMETA(DisplayName = "Campfire")
};

/**
 *  A single placeable building piece. Its shape is assembled at runtime from
 *  simple boxes so that doorways and windows have real openings, and so the
 *  whole modular kit works without any bespoke art. Pieces snap to a fixed grid
 *  (see GridSize) which lets the player raise walls side by side, drop a window
 *  or a door wherever they like, and cap it all with a roof — a free-form,
 *  build-your-dream-house system. Server-spawned and replicated.
 */
UCLASS()
class EVERA_API ABuildPiece : public AActor
{
	GENERATED_BODY()

public:
	ABuildPiece();

	virtual void Tick(float DeltaSeconds) override;

	/** Set which piece this is and (re)assemble its shape. */
	UFUNCTION(BlueprintCallable, Category="Build")
	void SetPieceType(EBuildPieceType InType);

	EBuildPieceType GetPieceType() const { return PieceType; }

	/** Display a real furniture prop mesh instead of the box-built structure.
	 *  Pass nullptr to go back to the structural piece for the current PieceType. */
	void SetFurnitureMesh(UStaticMesh* InMesh);

	/** Switch between a translucent placement preview and a solid, built piece. */
	void SetGhost(bool bGhost, bool bValid);

	// ---- Furniture catalogue (prop meshes players can place to decorate) ----
	static int32 NumFurniture();
	static FString FurnitureName(int32 Index);
	static FString FurniturePath(int32 Index);
	static int32 FurnitureCost(int32 Index);

	/** One grid cell = one piece footprint. Walls/floors tile on this spacing. */
	static constexpr float GridSize = 300.f;

	/** Wood needed to build a given piece. */
	static int32 GetWoodCost(EBuildPieceType Type);

	/** Short label for the HUD. */
	static FString GetDisplayName(EBuildPieceType Type);

	/** Cycle to the next piece type (wraps around). */
	static EBuildPieceType NextType(EBuildPieceType Type);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Rebuild the box parts for the current PieceType. */
	void Rebuild();

	/** Add one box part (centre + half-extents in cm, local space). */
	UStaticMeshComponent* AddBox(const FVector& Center, const FVector& HalfExtent);

	/** Tint every part (creates a dynamic material instance). */
	void SetTint(const FLinearColor& Color);

	UPROPERTY()
	USceneComponent* SceneRoot;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Build")
	EBuildPieceType PieceType = EBuildPieceType::Wall;

	UPROPERTY()
	UStaticMesh* CubeMesh;

	/** Tintable material used for the green/red placement preview. */
	UPROPERTY()
	UMaterialInterface* BaseMaterial;

	/** Real wood-plank look for built walls/floors/etc. */
	UPROPERTY()
	UMaterialInterface* WoodMaterial;

	/** Roof-tile look for built roofs. */
	UPROPERTY()
	UMaterialInterface* RoofMaterial;

	/** When set, the piece shows this prop mesh (furniture) instead of boxes. */
	UPROPERTY()
	UStaticMesh* FurnitureMesh = nullptr;

	/** Doorway only: hinge + swinging leaf that opens when the player is near. */
	UPROPERTY()
	USceneComponent* DoorHinge = nullptr;

	UPROPERTY()
	UStaticMeshComponent* DoorLeaf = nullptr;

	float DoorAngle = 0.f;

	/** Apply the solid built look (wood, or roof tiles for a roof). */
	void ApplyRealMaterial();

	bool bIsGhost = false;
};
