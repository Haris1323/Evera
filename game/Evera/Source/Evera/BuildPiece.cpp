// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "BuildPiece.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

namespace
{
	// Furniture props players can place to decorate rooms (from the village pack).
	struct FFurnitureDef { const TCHAR* Name; const TCHAR* Path; int32 Cost; };
	const FFurnitureDef GFurniture[] = {
		{ TEXT("Bench"),  TEXT("/Game/Fantastic_Village_Pack/meshes/props/furniture/SM_PROP_bench_01.SM_PROP_bench_01"),         3 },
		{ TEXT("Barrel"), TEXT("/Game/Fantastic_Village_Pack/meshes/props/container/SM_PROP_barrel_01.SM_PROP_barrel_01"),       2 },
		{ TEXT("Crate"),  TEXT("/Game/Fantastic_Village_Pack/meshes/props/container/SM_PROP_crate_01.SM_PROP_crate_01"),         2 },
		{ TEXT("Chest"),  TEXT("/Game/Fantastic_Village_Pack/meshes/props/container/SM_PROP_treasurechest.SM_PROP_treasurechest"), 4 },
		{ TEXT("Well"),   TEXT("/Game/Fantastic_Village_Pack/meshes/props/deco/SM_PROP_well.SM_PROP_well"),                      5 },
		{ TEXT("Pot"),    TEXT("/Game/Fantastic_Village_Pack/meshes/props/food/SM_PROP_cookingpot_big.SM_PROP_cookingpot_big"),  3 },
	};

	// Piece dimensions (cm). One cell is GridSize wide; walls are WallHeight tall.
	constexpr float Cell = ABuildPiece::GridSize; // 300
	constexpr float Half = Cell * 0.5f;           // 150
	constexpr float Thick = 15.f;                 // wall/floor thickness (half = 7.5)
	constexpr float HalfThick = Thick * 0.5f;
	constexpr float WallHeight = 300.f;
	constexpr float SlabHalf = 6.f;               // floor/roof slab half-thickness
}

ABuildPiece::ABuildPiece()
{
	// Ticking is only turned on for real doorways (to swing the door open).
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// The engine cube (100^3, centred) and a tintable material are both safe to
	// load in the constructor.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		CubeMesh = Cube.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (Mat.Succeeded())
	{
		BaseMaterial = Mat.Object;
	}
	// Real building look from the village pack (falls back to a tint if absent).
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Wood(TEXT("/Game/Fantastic_Village_Pack/materials/MI_wood_planks_01.MI_wood_planks_01"));
	if (Wood.Succeeded())
	{
		WoodMaterial = Wood.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Roof(TEXT("/Game/Fantastic_Village_Pack/materials/MI_rooftiles_01.MI_rooftiles_01"));
	if (Roof.Succeeded())
	{
		RoofMaterial = Roof.Object;
	}
}

void ABuildPiece::ApplyRealMaterial()
{
	if (FurnitureMesh)
	{
		return; // furniture keeps its own prop materials
	}
	UMaterialInterface* Mat = (PieceType == EBuildPieceType::Roof) ? RoofMaterial : WoodMaterial;
	if (!Mat)
	{
		// No pack material available: fall back to a warm wood tint.
		SetTint(FLinearColor(0.42f, 0.28f, 0.16f));
		return;
	}
	for (UStaticMeshComponent* P : Parts)
	{
		if (P)
		{
			P->SetMaterial(0, Mat);
		}
	}
}

void ABuildPiece::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Rebuild();
}

void ABuildPiece::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!DoorHinge)
	{
		return;
	}

	// Swing the door open while the player is close, shut it again when they leave.
	float Target = 0.f;
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (FVector::Dist(Player->GetActorLocation(), GetActorLocation()) < 240.f)
		{
			Target = 82.f;
		}
	}
	DoorAngle = FMath::FInterpTo(DoorAngle, Target, DeltaSeconds, 6.f);
	DoorHinge->SetRelativeRotation(FRotator(0.f, DoorAngle, 0.f));
}

void ABuildPiece::SetPieceType(EBuildPieceType InType)
{
	PieceType = InType;
	Rebuild();
}

void ABuildPiece::SetFurnitureMesh(UStaticMesh* InMesh)
{
	FurnitureMesh = InMesh;
	Rebuild();
}

int32 ABuildPiece::NumFurniture() { return UE_ARRAY_COUNT(GFurniture); }

FString ABuildPiece::FurnitureName(int32 Index)
{
	return (Index >= 0 && Index < NumFurniture()) ? FString(GFurniture[Index].Name) : FString();
}

FString ABuildPiece::FurniturePath(int32 Index)
{
	return (Index >= 0 && Index < NumFurniture()) ? FString(GFurniture[Index].Path) : FString();
}

int32 ABuildPiece::FurnitureCost(int32 Index)
{
	return (Index >= 0 && Index < NumFurniture()) ? GFurniture[Index].Cost : 1;
}

UStaticMeshComponent* ABuildPiece::AddBox(const FVector& Center, const FVector& HalfExtent)
{
	UStaticMeshComponent* Box = NewObject<UStaticMeshComponent>(this);
	Box->SetupAttachment(SceneRoot);
	Box->SetStaticMesh(CubeMesh);
	// The cube is 100 units (half-extent 50) -> scale to reach the wanted size.
	Box->SetRelativeLocation(Center);
	Box->SetRelativeScale3D(HalfExtent / 50.f);
	Box->SetCollisionEnabled(bIsGhost ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	Box->SetCollisionResponseToAllChannels(ECR_Block);
	Box->RegisterComponent();
	Parts.Add(Box);
	return Box;
}

void ABuildPiece::Rebuild()
{
	if (!CubeMesh)
	{
		return;
	}

	for (UStaticMeshComponent* P : Parts)
	{
		if (P)
		{
			P->DestroyComponent();
		}
	}
	Parts.Reset();

	if (DoorLeaf) { DoorLeaf->DestroyComponent(); DoorLeaf = nullptr; }
	if (DoorHinge) { DoorHinge->DestroyComponent(); DoorHinge = nullptr; }
	SetActorTickEnabled(false);

	// Furniture: show one real prop mesh (keeps its own materials).
	if (FurnitureMesh)
	{
		UStaticMeshComponent* Prop = NewObject<UStaticMeshComponent>(this);
		Prop->SetupAttachment(SceneRoot);
		Prop->SetStaticMesh(FurnitureMesh);
		Prop->SetCollisionEnabled(bIsGhost ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
		Prop->SetCollisionResponseToAllChannels(ECR_Block);
		Prop->RegisterComponent();
		Parts.Add(Prop);
		return;
	}

	switch (PieceType)
	{
	case EBuildPieceType::Floor:
		AddBox(FVector(0, 0, SlabHalf), FVector(Half, Half, SlabHalf));
		break;

	case EBuildPieceType::Wall:
		AddBox(FVector(0, 0, WallHeight * 0.5f), FVector(Half, HalfThick, WallHeight * 0.5f));
		break;

	case EBuildPieceType::Doorway:
	{
		// Wall framing a doorway. The door leaf hangs on a hinge and has no
		// collision, so it swings open (see Tick) and the player can walk in.
		AddBox(FVector(-105, 0, WallHeight * 0.5f), FVector(45, HalfThick, WallHeight * 0.5f)); // left post
		AddBox(FVector(105, 0, WallHeight * 0.5f), FVector(45, HalfThick, WallHeight * 0.5f));  // right post
		AddBox(FVector(0, 0, 255), FVector(60, HalfThick, 45));                                  // lintel

		DoorHinge = NewObject<USceneComponent>(this);
		DoorHinge->SetupAttachment(SceneRoot);
		DoorHinge->SetRelativeLocation(FVector(-58.f, 0.f, 0.f)); // hinge on the left jamb
		DoorHinge->RegisterComponent();

		DoorLeaf = NewObject<UStaticMeshComponent>(this);
		DoorLeaf->SetupAttachment(DoorHinge);
		DoorLeaf->SetStaticMesh(CubeMesh);
		DoorLeaf->SetRelativeLocation(FVector(55.f, HalfThick * 0.4f, 105.f));
		DoorLeaf->SetRelativeScale3D(FVector(55.f, HalfThick * 0.5f, 103.f) / 50.f);
		DoorLeaf->SetCollisionEnabled(ECollisionEnabled::NoCollision); // passable
		if (WoodMaterial)
		{
			DoorLeaf->SetMaterial(0, WoodMaterial);
		}
		DoorLeaf->RegisterComponent();

		// Only real (placed) doors need to swing; the ghost preview stays shut.
		SetActorTickEnabled(!bIsGhost);
		break;
	}

	case EBuildPieceType::Window:
		// Wall with a framed window: sill, header, jambs and a cross mullion.
		AddBox(FVector(-110, 0, WallHeight * 0.5f), FVector(40, HalfThick, WallHeight * 0.5f)); // left
		AddBox(FVector(110, 0, WallHeight * 0.5f), FVector(40, HalfThick, WallHeight * 0.5f));  // right
		AddBox(FVector(0, 0, 60), FVector(70, HalfThick, 60));                                   // sill
		AddBox(FVector(0, 0, 260), FVector(70, HalfThick, 40));                                  // header
		AddBox(FVector(0, 0, 170), FVector(6, HalfThick, 50));                                   // mullion (vertical)
		AddBox(FVector(0, 0, 170), FVector(70, HalfThick, 6));                                   // mullion (horizontal)
		break;

	case EBuildPieceType::Roof:
		// A flat slab sitting at wall height, so it caps a cell of walls.
		AddBox(FVector(0, 0, WallHeight + SlabHalf), FVector(Half, Half, SlabHalf));
		break;

	case EBuildPieceType::Pillar:
		AddBox(FVector(0, 0, WallHeight * 0.5f), FVector(22, 22, WallHeight * 0.5f));
		break;

	case EBuildPieceType::Fence:
	{
		// A low rustic fence: two posts with two horizontal rails between them,
		// spanning one cell so fences line up into a run for a farm pen.
		const float FenceH = 120.f;
		AddBox(FVector(-Half + 12.f, 0, FenceH * 0.5f), FVector(12, 12, FenceH * 0.5f)); // left post
		AddBox(FVector(Half - 12.f, 0, FenceH * 0.5f), FVector(12, 12, FenceH * 0.5f));  // right post
		AddBox(FVector(0, 0, FenceH - 20.f), FVector(Half, 6, 8));                        // top rail
		AddBox(FVector(0, 0, FenceH * 0.45f), FVector(Half, 6, 8));                       // lower rail
		break;
	}

	case EBuildPieceType::Campfire:
		// Never box-built: a Campfire is placed as its own ACampfire actor (it can
		// be lit). This case only matters for the ghost preview — show a small ring.
		AddBox(FVector(0, 0, 8.f), FVector(46, 46, 8.f));
		break;

	case EBuildPieceType::Bed:
		// A simple cosy bed: legs, a mattress and a pillow at one end.
		AddBox(FVector(-70, 0, 12.f), FVector(10, 34, 12));   // foot legs
		AddBox(FVector(70, 0, 12.f), FVector(10, 34, 12));    // head legs
		AddBox(FVector(0, 0, 30.f), FVector(85, 38, 10));     // mattress
		AddBox(FVector(58, 0, 46.f), FVector(24, 30, 8));     // pillow
		break;
	}

	// Default appearance: real building material. Ghost mode overrides in SetGhost.
	if (!bIsGhost)
	{
		ApplyRealMaterial();
	}
}

void ABuildPiece::SetGhost(bool bGhost, bool bValid)
{
	bIsGhost = bGhost;
	for (UStaticMeshComponent* P : Parts)
	{
		if (P)
		{
			P->SetCollisionEnabled(bGhost ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
		}
	}

	if (bGhost)
	{
		SetActorTickEnabled(false); // a preview door shouldn't swing
		SetTint(bValid ? FLinearColor(0.15f, 0.85f, 0.25f) : FLinearColor(0.90f, 0.15f, 0.15f));
	}
	else
	{
		ApplyRealMaterial();
	}
}

void ABuildPiece::SetTint(const FLinearColor& Color)
{
	if (!BaseMaterial)
	{
		return;
	}
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (!MID)
	{
		return;
	}
	MID->SetVectorParameterValue(TEXT("Color"), Color);
	for (UStaticMeshComponent* P : Parts)
	{
		if (P)
		{
			P->SetMaterial(0, MID);
		}
	}
}

int32 ABuildPiece::GetWoodCost(EBuildPieceType Type)
{
	switch (Type)
	{
	case EBuildPieceType::Floor:   return 2;
	case EBuildPieceType::Wall:    return 2;
	case EBuildPieceType::Doorway: return 3;
	case EBuildPieceType::Window:  return 3;
	case EBuildPieceType::Roof:    return 2;
	case EBuildPieceType::Pillar:  return 1;
	case EBuildPieceType::Fence:   return 1;
	case EBuildPieceType::Campfire:return 3;
	case EBuildPieceType::Bed:     return 4;
	default:                       return 2;
	}
}

FString ABuildPiece::GetDisplayName(EBuildPieceType Type)
{
	switch (Type)
	{
	case EBuildPieceType::Floor:   return TEXT("Floor");
	case EBuildPieceType::Wall:    return TEXT("Wall");
	case EBuildPieceType::Doorway: return TEXT("Doorway");
	case EBuildPieceType::Window:  return TEXT("Window");
	case EBuildPieceType::Roof:    return TEXT("Roof");
	case EBuildPieceType::Pillar:  return TEXT("Pillar");
	case EBuildPieceType::Fence:   return TEXT("Fence");
	case EBuildPieceType::Campfire:return TEXT("Campfire");
	case EBuildPieceType::Bed:     return TEXT("Bed");
	default:                       return TEXT("Piece");
	}
}

EBuildPieceType ABuildPiece::NextType(EBuildPieceType Type)
{
	const uint8 Count = static_cast<uint8>(EBuildPieceType::Bed) + 1;
	return static_cast<EBuildPieceType>((static_cast<uint8>(Type) + 1) % Count);
}
