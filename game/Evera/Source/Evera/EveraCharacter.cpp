// Copyright Epic Games, Inc. All Rights Reserved.

#include "EveraCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "SurvivalStatsComponent.h"
#include "InventoryComponent.h"
#include "SkillsComponent.h"
#include "CraftingComponent.h"
#include "EveraHUD.h"
#include "ResourceNode.h"
#include "ResourcePickup.h"
#include "RideableHorse.h"
#include "CompanionPet.h"
#include "Campfire.h"
#include "WanderingAnimal.h"
#include "EveraTimeOfDay.h"
#include "ForestSpawner.h"
#include "BuildPiece.h"
#include "EveraGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Evera.h"

AEveraCharacter::AEveraCharacter()
{
	// Tick so the build-mode placement preview can follow the camera.
	PrimaryActorTick.bCanEverTick = true;

	BuildPieceType = EBuildPieceType::Wall;

	// Set size for collision capsule (sized for the ~155 cm chibi hero).
	GetCapsuleComponent()->InitCapsuleSize(40.f, 78.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Tick drives the gentle gather bend.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create the survival stats component (health, hunger, thirst, energy)
	SurvivalStats = CreateDefaultSubobject<USurvivalStatsComponent>(TEXT("SurvivalStats"));

	// Create the inventory component (gathered resources)
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

	// Create the skills component (woodcutting, mining, ... grow through use)
	Skills = CreateDefaultSubobject<USkillsComponent>(TEXT("Skills"));

	// Create the crafting component (turns resources into tools)
	Crafting = CreateDefaultSubobject<UCraftingComponent>(TEXT("Crafting"));

	// Create the held-axe mesh, attached to the right hand (hidden until crafted).
	HeldAxe = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeldAxe"));
	HeldAxe->SetupAttachment(GetMesh(), AxeHandSocket);
	HeldAxe->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeldAxe->SetVisibility(false);

	// Create the backpack mesh, worn on the back.
	Backpack = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Backpack"));
	Backpack->SetupAttachment(GetMesh(), BackpackSocket);
	Backpack->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Little flies that appear buzzing around the player when hygiene gets low.
	for (int32 i = 0; i < 4; ++i)
	{
		UStaticMeshComponent* Fly = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Fly%d"), i));
		Fly->SetupAttachment(GetCapsuleComponent());
		Fly->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Fly->SetVisibility(false);
		Fly->SetCastShadow(false);
		Fly->SetRelativeScale3D(FVector(0.03f));
		Flies.Add(Fly);
	}

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AEveraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEveraCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AEveraCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEveraCharacter::Look);
	}
	else
	{
		UE_LOG(LogEvera, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// Interact / gather on the E key. Enhanced Input deletes BindKey on its own
	// component, so bind through the base UInputComponent pointer (E is not mapped
	// to any Enhanced Input action, so this legacy binding still fires).
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AEveraCharacter::Interact);

	// Craft a stone axe on C, a stone pickaxe on V.
	PlayerInputComponent->BindKey(EKeys::C, IE_Pressed, this, &AEveraCharacter::CraftStoneAxe);
	PlayerInputComponent->BindKey(EKeys::V, IE_Pressed, this, &AEveraCharacter::CraftStonePickaxe);

	// Open/close the backpack on the I key.
	PlayerInputComponent->BindKey(EKeys::I, IE_Pressed, this, &AEveraCharacter::ToggleInventory);

	// Mount / dismount the nearest horse on the F key.
	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &AEveraCharacter::MountToggle);

	// Eat farm food on the G key.
	PlayerInputComponent->BindKey(EKeys::G, IE_Pressed, this, &AEveraCharacter::EatFood);

	// Building: B toggles build mode, N cycles the piece, R rotates it,
	// left mouse places it, X removes the piece being looked at.
	PlayerInputComponent->BindKey(EKeys::B, IE_Pressed, this, &AEveraCharacter::ToggleBuildMode);
	PlayerInputComponent->BindKey(EKeys::N, IE_Pressed, this, &AEveraCharacter::CycleBuildPiece);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &AEveraCharacter::RotateBuildPiece);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AEveraCharacter::PlaceBuildPiece);
	PlayerInputComponent->BindKey(EKeys::X, IE_Pressed, this, &AEveraCharacter::RemoveBuildPiece);
	// Build up/down a storey at a time.
	PlayerInputComponent->BindKey(EKeys::PageUp, IE_Pressed, this, &AEveraCharacter::RaiseBuildLevel);
	PlayerInputComponent->BindKey(EKeys::PageDown, IE_Pressed, this, &AEveraCharacter::LowerBuildLevel);
	// Tab switches the palette between structure and furniture.
	PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AEveraCharacter::ToggleBuildCategory);

	// Pick a build piece straight from the palette with the number keys.
	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &AEveraCharacter::SelectBuild1);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AEveraCharacter::SelectBuild2);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AEveraCharacter::SelectBuild3);
	PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AEveraCharacter::SelectBuild4);
	PlayerInputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AEveraCharacter::SelectBuild5);
	PlayerInputComponent->BindKey(EKeys::Six, IE_Pressed, this, &AEveraCharacter::SelectBuild6);
	PlayerInputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &AEveraCharacter::SelectBuild7);
	PlayerInputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &AEveraCharacter::SelectBuild8);
}

void AEveraCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AEveraCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AEveraCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	EnsureGameInput();
}

void AEveraCharacter::EnsureGameInput()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
		PC->FlushPressedKeys();
	}
}

void AEveraCharacter::BeginPlay()
{
	Super::BeginPlay();

	// The main menu left input in UI mode; hand control back to gameplay. Do it now
	// and again next tick, since the controller may not be set on the first frame.
	EnsureGameInput();
	GetWorldTimerManager().SetTimerForNextTick(this, &AEveraCharacter::EnsureGameInput);

	// Swap the default mannequin for the chibi hero before placing equipment,
	// so the axe/backpack sockets resolve against the hero skeleton.
	SetupHeroAvatar();

	// Set up the held axe: load the mesh, place it in the hand, hide until crafted.
	if (UStaticMesh* AxeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Evera/Items/SM_EVERA_HeroAxe_Pro/StaticMeshes/SM_EVERA_HeroAxe_Pro.SM_EVERA_HeroAxe_Pro")))
	{
		HeldAxe->SetStaticMesh(AxeMesh);
	}
	HeldAxe->SetRelativeLocation(AxeGripLocation);
	HeldAxe->SetRelativeRotation(AxeGripRotation);
	HeldAxe->SetRelativeScale3D(FVector(AxeGripScale));

	// Set up the backpack on the back.
	if (UStaticMesh* BackpackMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Evera/Items/SM_EVERA_AdventureBackpack_Pro/StaticMeshes/SM_EVERA_AdventureBackpack_Pro.SM_EVERA_AdventureBackpack_Pro")))
	{
		Backpack->SetStaticMesh(BackpackMesh);
	}
	Backpack->SetRelativeLocation(BackpackLocation);
	Backpack->SetRelativeRotation(BackpackRotation);
	Backpack->SetRelativeScale3D(FVector(BackpackScale));
	if (Crafting)
	{
		Crafting->OnCraftingChanged.AddDynamic(this, &AEveraCharacter::UpdateHeldAxe);
	}
	UpdateHeldAxe();

	// Give the flies a tiny dark-sphere look.
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Evera/Materials/M_EveraTint.M_EveraTint"));
		if (!BaseMat)
		{
			BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		}
		UMaterialInstanceDynamic* FlyMID = BaseMat ? UMaterialInstanceDynamic::Create(BaseMat, this) : nullptr;
		if (FlyMID)
		{
			FlyMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.02f, 0.02f, 0.02f));
		}
		for (UStaticMeshComponent* Fly : Flies)
		{
			if (!Fly) continue;
			Fly->SetStaticMesh(Sphere);
			if (FlyMID) { Fly->SetMaterial(0, FlyMID); }
		}
	}

	// Load the gather animations (created on our own skeleton, no retarget needed).
	PickupAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Evera/Animations/AN_EVERA_Pickup_Ground.AN_EVERA_Pickup_Ground"));
	ChopAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Evera/Animations/AN_EVERA_Chop_Wood.AN_EVERA_Chop_Wood"));

	// Scatter loose wood & stone on the ground so the player can gather starter
	// materials by hand (no tool) and craft their first axe & pickaxe.
	if (HasAuthority())
	{
		const FVector PickupCenter = GetActorLocation();
		auto ScatterPickups = [&](EResourceType Type, int32 Count)
		{
			for (int32 i = 0; i < Count; ++i)
			{
				const float Angle = FMath::FRandRange(0.f, 2.f * PI);
				const float Rad = FMath::FRandRange(300.f, 2600.f);
				FVector Pos = PickupCenter + FVector(FMath::Cos(Angle) * Rad, FMath::Sin(Angle) * Rad, 0.f);
				FHitResult Ground;
				FCollisionQueryParams GP(FName(TEXT("PickupGround")), false, this);
				if (GetWorld()->LineTraceSingleByChannel(Ground, Pos + FVector(0, 0, 3000), Pos - FVector(0, 0, 6000), ECC_Visibility, GP))
				{
					Pos.Z = Ground.ImpactPoint.Z + 9.f;
				}
				FActorSpawnParameters SP;
				SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				const FTransform T(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Pos);
				if (AResourcePickup* Pk = GetWorld()->SpawnActor<AResourcePickup>(AResourcePickup::StaticClass(), T, SP))
				{
					Pk->Setup(Type, 2);
				}
			}
		};
		ScatterPickups(EResourceType::Wood, 28);
		ScatterPickups(EResourceType::Stone, 20);

		// Give the player their companion dog, Lea — she trots along beside them
		// and offers friendly, context-aware tips on what to do next.
		FVector LeaPos = GetActorLocation() + GetActorForwardVector() * 130.f + GetActorRightVector() * 90.f;
		FActorSpawnParameters PetSP;
		PetSP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PetSP.Owner = this;
		Companion = GetWorld()->SpawnActor<ACompanionPet>(ACompanionPet::StaticClass(), FTransform(FRotator::ZeroRotator, LeaPos), PetSP);
		if (Companion)
		{
			Companion->SetOwnerPlayer(this);
		}

		// Spawn a rideable horse a little way in front, seated on the terrain.
		FVector HorsePos = GetActorLocation() + GetActorForwardVector() * 550.f;
		FHitResult HorseGround;
		FCollisionQueryParams HGP(FName(TEXT("HorseSpawn")), false, this);
		if (GetWorld()->LineTraceSingleByChannel(HorseGround, HorsePos + FVector(0, 0, 3000), HorsePos - FVector(0, 0, 6000), ECC_Visibility, HGP))
		{
			HorsePos.Z = HorseGround.ImpactPoint.Z;
		}
		FActorSpawnParameters HorseSP;
		HorseSP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<ARideableHorse>(ARideableHorse::StaticClass(), FTransform(FRotator::ZeroRotator, HorsePos), HorseSP);

		// Populate the world with a mix of stylized, animated, tameable animals
		// (Animals_Free pack). Each is an AWanderingAnimal pointed at its species;
		// the player can tame any of them (E) onto their farm.
		// Bigger herds — the world is meant for multiplayer, so there's plenty of
		// wildlife for everyone to find and tame.
		// WalkIdx = the clip number in the walk anim's name (most are 001, but the
		// chicken's walk clip is numbered 003 in this pack).
		struct FSpeciesSpawn { const TCHAR* Name; float CapHalf; float CapRadius; int32 Count; int32 WalkIdx; };
		static const FSpeciesSpawn Species[] = {
			{ TEXT("Deer"),    80.f, 45.f, 8,  1 },
			{ TEXT("Chicken"), 26.f, 18.f, 10, 3 },
			{ TEXT("Kitty"),   24.f, 16.f, 8,  1 },
			{ TEXT("Pinguin"), 36.f, 22.f, 4,  1 },
			{ TEXT("Tiger"),   75.f, 55.f, 2,  1 },
		};
		int32 AnimalIdx = 0;
		for (const FSpeciesSpawn& Sp : Species)
		{
			for (int32 c = 0; c < Sp.Count; ++c)
			{
				++AnimalIdx;
				// Spread them out in a widening spiral so they're not all clustered.
				const float Ang = 2.39996f * AnimalIdx; // golden-angle scatter
				const float Rad = 650.f + 150.f * (AnimalIdx % 12);
				FVector Pos = GetActorLocation() + FVector(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
				FHitResult G;
				FCollisionQueryParams GP(FName(TEXT("AnimalSpawn")), false, this);
				if (GetWorld()->LineTraceSingleByChannel(G, Pos + FVector(0, 0, 3000), Pos - FVector(0, 0, 6000), ECC_Visibility, GP))
				{
					Pos.Z = G.ImpactPoint.Z + Sp.CapHalf;
				}
				const FTransform Xf(FRotator::ZeroRotator, Pos);
				AWanderingAnimal* A = GetWorld()->SpawnActorDeferred<AWanderingAnimal>(
					AWanderingAnimal::StaticClass(), Xf, nullptr, nullptr,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if (A)
				{
					const FString MeshP = FString::Printf(TEXT("/Game/Animals_Free/Animals/Meshes/SKM_%s_001.SKM_%s_001"), Sp.Name, Sp.Name);
					const FString WalkName = FString::Printf(TEXT("ANIM_%s_001_Anim_%s_%03d_walk"), Sp.Name, Sp.Name, Sp.WalkIdx);
					const FString WalkP = FString::Printf(TEXT("/Game/Animals_Free/Animals/Animations/%s.%s"), *WalkName, *WalkName);
					const FString IdleP = FString::Printf(TEXT("/Game/Animals_Free/Animals/Animations/ANIM_%s_001_Anim_%s_001_idle.ANIM_%s_001_Anim_%s_001_idle"), Sp.Name, Sp.Name, Sp.Name, Sp.Name);
					// Mesh faces sideways by default in this pack — rotate to face travel.
					A->ConfigureSpecies(MeshP, WalkP, IdleP, Sp.Name, -90.f, Sp.CapHalf, Sp.CapRadius);
					A->FinishSpawning(Xf);
				}
			}
		}

		// Start the day/night cycle (only one driver for the whole world).
		bool bHasTime = false;
		for (TActorIterator<AEveraTimeOfDay> It(GetWorld()); It; ++It) { bHasTime = true; TimeOfDay = *It; break; }
		if (!bHasTime)
		{
			TimeOfDay = GetWorld()->SpawnActor<AEveraTimeOfDay>(AEveraTimeOfDay::StaticClass(), FTransform::Identity);
		}
	}

	if (!HasAuthority() || !bSpawnTestResourceNodes)
	{
		return;
	}

	// NOTE: In the CozyNature DemoMap the level already ships its own trees,
	// grass and rocks, so we do NOT scatter our procedural forest on top of it
	// (that only doubled up the ground grass into a green wall in front of the
	// camera). We keep just the gatherable resource nodes below. Re-enable the
	// ForestSpawner only when playing in an empty/flat level of our own.
	// GetWorld()->SpawnActor<AForestSpawner>(AForestSpawner::StaticClass(), FTransform(GetActorLocation()));

	// Prototype convenience: drop a ring of gatherable nodes around the player so
	// there is something to harvest without hand-placing actors in the level.
	// Remove this once the world has real, level-placed resources.
	const int32 NumNodes = 6;
	const float Radius = 450.f;
	const FVector Center = GetActorLocation();

	for (int32 i = 0; i < NumNodes; ++i)
	{
		const EResourceType Type = (i % 2 == 0) ? EResourceType::Wood : EResourceType::Stone;

		const float Angle = (2.f * PI * i) / NumNodes;
		const FVector Dir(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		FVector SpawnLocation = Center + Dir * Radius;

		// Drop the node onto the ground beneath the ring position. The player can
		// spawn high above the terrain (PlayerStart Z is well above ground so the
		// pawn drops in), so trace across a generous vertical range to always find
		// the landscape below — a short trace left the nodes floating in mid-air.
		FHitResult Ground;
		const FVector TraceStart = SpawnLocation + FVector(0.f, 0.f, 2000.f);
		const FVector TraceEnd = SpawnLocation - FVector(0.f, 0.f, 6000.f);
		FCollisionQueryParams GroundParams(FName(TEXT("NodeGround")), false, this);
		if (GetWorld()->LineTraceSingleByChannel(Ground, TraceStart, TraceEnd, ECC_Visibility, GroundParams))
		{
			// Place the node's base on the ground (meshes are base-pivoted).
			SpawnLocation.Z = Ground.ImpactPoint.Z;
		}

		const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
		AResourceNode* Node = GetWorld()->SpawnActorDeferred<AResourceNode>(
			AResourceNode::StaticClass(), SpawnTransform, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Node)
		{
			Node->SetResourceType(Type);
			Node->FinishSpawning(SpawnTransform);
		}
	}
}

void AEveraCharacter::Interact()
{
	if (!FollowCamera)
	{
		return;
	}

	// Trace forward from the camera; if we're looking at a resource node, gather it.
	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End = Start + FollowCamera->GetForwardVector() * InteractDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(FName(TEXT("Interact")), /*bTraceComplex=*/false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (ACampfire* Fire = Cast<ACampfire>(Hit.GetActor()))
		{
			// Light or put out the campfire we're looking at.
			ServerToggleCampfire(Fire);
		}
		else if (AWanderingAnimal* Animal = Cast<AWanderingAnimal>(Hit.GetActor()))
		{
			// Befriend the animal so it joins the farm and follows us.
			if (!Animal->IsTamed())
			{
				ServerTameAnimal(Animal);
			}
		}
		else if (AResourcePickup* Pickup = Cast<AResourcePickup>(Hit.GetActor()))
		{
			PlayGatherAnim(); // bend down and pick it up
			ServerGatherPickup(Pickup);
		}
		else if (AResourceNode* Node = Cast<AResourceNode>(Hit.GetActor()))
		{
			PlayGatherAnim(); // swing the axe / pickaxe
			ServerGather(Node);
		}
		else if (UInstancedStaticMeshComponent* Foliage = Cast<UInstancedStaticMeshComponent>(Hit.GetComponent()))
		{
			// A painted tree in the forest: harvest the single instance we hit.
			if (Hit.Item >= 0)
			{
				PlayGatherAnim();
				ServerGatherFoliage(Foliage, Hit.Item);
			}
		}
	}
}

void AEveraCharacter::ServerGatherFoliage_Implementation(UInstancedStaticMeshComponent* FoliageComp, int32 InstanceIndex)
{
	if (!FoliageComp || InstanceIndex < 0 || InstanceIndex >= FoliageComp->GetInstanceCount())
	{
		return;
	}

	// Only tree foliage yields wood; walking-over grass/bushes must never harvest.
	UStaticMesh* SM = FoliageComp->GetStaticMesh();
	if (!SM)
	{
		return;
	}
	const FString MeshName = SM->GetName();
	if (!MeshName.Contains(TEXT("Tree")) && !MeshName.Contains(TEXT("Pine")))
	{
		return;
	}

	// You need an axe to chop a tree.
	UCraftingComponent* Craft = FindComponentByClass<UCraftingComponent>();
	if (!Craft || Craft->GetCraftedCount(ECraftableItem::StoneAxe) <= 0)
	{
		return;
	}

	// Reach check + a stable position key for this tree's remaining health.
	FTransform InstanceXform;
	if (!FoliageComp->GetInstanceTransform(InstanceIndex, InstanceXform, /*bWorldSpace=*/true))
	{
		return;
	}
	const FVector TreePos = InstanceXform.GetLocation();
	if (FVector::Dist(GetActorLocation(), TreePos) > MaxGatherDistance)
	{
		return;
	}

	// Per-hit yield scales with the woodcutting level: low level fells slowly
	// (many hits), high level fells fast (few hits).
	USkillsComponent* SkillsComp = FindComponentByClass<USkillsComponent>();
	const int32 Level = SkillsComp ? SkillsComp->GetLevel(ESkillType::Woodcutting) : 1;
	const int32 PerHit = FMath::Max(1, 1 + Level);

	if (UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
	{
		Inv->AddResource(EResourceType::Wood, PerHit);
	}
	if (SkillsComp)
	{
		SkillsComp->AddXP(ESkillType::Woodcutting, static_cast<float>(PerHit));
	}

	const FIntVector Key(FMath::RoundToInt(TreePos.X / 50.f), FMath::RoundToInt(TreePos.Y / 50.f), FMath::RoundToInt(TreePos.Z / 50.f));
	int32 Health = FoliageHealth.Contains(Key) ? FoliageHealth[Key] : TreeHealthMax;
	Health -= PerHit;
	if (Health <= 0)
	{
		FoliageHealth.Remove(Key);
		FoliageComp->RemoveInstance(InstanceIndex); // fell the tree only when depleted
	}
	else
	{
		FoliageHealth.Add(Key, Health);
	}
}

// ---- Riding the horse ------------------------------------------------------

void AEveraCharacter::MountToggle()
{
	if (bMounted)
	{
		Dismount();
		return;
	}

	// Climb onto the nearest horse within reach.
	ARideableHorse* Nearest = nullptr;
	float BestDist = MountRange;
	for (TActorIterator<ARideableHorse> It(GetWorld()); It; ++It)
	{
		const float D = FVector::Dist(GetActorLocation(), It->GetActorLocation());
		if (D < BestDist)
		{
			BestDist = D;
			Nearest = *It;
		}
	}
	if (Nearest)
	{
		Mount(Nearest);
	}
}

void AEveraCharacter::Mount(ARideableHorse* Horse)
{
	if (!Horse || !Horse->GetSaddlePoint())
	{
		return;
	}

	CurrentHorse = Horse;
	bMounted = true;

	// Hand physics/movement over to the horse and sit the player in the saddle.
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachToComponent(Horse->GetSaddlePoint(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeRotation(FRotator::ZeroRotator);

	// Pull the camera back and up so the whole horse + rider are in view.
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = 800.f;
		CameraBoom->SocketOffset = FVector(0.f, 0.f, 90.f);
	}
}

void AEveraCharacter::Dismount()
{
	bMounted = false;
	MountMoveInput = FVector2D::ZeroVector;

	// Restore the normal third-person camera framing.
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = 400.f;
		CameraBoom->SocketOffset = FVector::ZeroVector;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Restore normal movement + collision, then step off to the side onto the ground.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	FVector Side = GetActorLocation();
	if (CurrentHorse)
	{
		Side = CurrentHorse->GetActorLocation() + CurrentHorse->GetActorRightVector() * 160.f;
		FHitResult G;
		FCollisionQueryParams GP(FName(TEXT("Dismount")), false, this);
		GP.AddIgnoredActor(CurrentHorse);
		if (GetWorld()->LineTraceSingleByChannel(G, Side + FVector(0, 0, 300), Side - FVector(0, 0, 2000), ECC_Visibility, GP))
		{
			Side.Z = G.ImpactPoint.Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}
	}
	SetActorLocation(Side, false, nullptr, ETeleportType::TeleportPhysics);

	CurrentHorse = nullptr;
}

void AEveraCharacter::DriveMountedHorse(float DeltaSeconds)
{
	if (!CurrentHorse || MountMoveInput.IsNearlyZero())
	{
		return; // no input: the horse holds still
	}

	// Direction from the movement input, relative to where the player is looking.
	const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
	const FVector Fwd = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	FVector Dir = Fwd * MountMoveInput.Y + Right * MountMoveInput.X;
	Dir.Z = 0.f;
	if (Dir.IsNearlyZero())
	{
		return;
	}
	Dir.Normalize();

	// Turn the horse smoothly toward that direction, then walk it forward.
	const FRotator WantRot(0.f, Dir.Rotation().Yaw, 0.f);
	CurrentHorse->SetActorRotation(FMath::RInterpTo(CurrentHorse->GetActorRotation(), WantRot, DeltaSeconds, CurrentHorse->GetTurnSpeed()));

	const FVector Step = CurrentHorse->GetActorForwardVector() * CurrentHorse->GetRideSpeed() * DeltaSeconds;
	CurrentHorse->AddActorWorldOffset(Step, /*bSweep=*/true);
}

// ---- Farm (tame animals) + campfire ----------------------------------------

void AEveraCharacter::ServerToggleCampfire_Implementation(ACampfire* Fire)
{
	if (!Fire)
	{
		return;
	}
	if (FVector::Dist(GetActorLocation(), Fire->GetActorLocation()) > MaxGatherDistance)
	{
		return;
	}
	Fire->ToggleLit();
}

void AEveraCharacter::ServerTameAnimal_Implementation(AWanderingAnimal* Animal)
{
	if (!Animal || Animal->IsTamed())
	{
		return;
	}
	// Animals are large, so allow a slightly longer reach than for gathering.
	if (FVector::Dist(GetActorLocation(), Animal->GetActorLocation()) > MaxGatherDistance + 250.f)
	{
		return;
	}
	Animal->Tame(this);
	++FarmAnimalCount;
}

void AEveraCharacter::EatFood()
{
	ServerEatFood();
}

void AEveraCharacter::ServerEatFood_Implementation()
{
	UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>();
	USurvivalStatsComponent* Stats = FindComponentByClass<USurvivalStatsComponent>();
	if (!Inv || !Stats)
	{
		return;
	}

	const float Max = Stats->GetMaxValue();

	// Eggs fill you up; milk is lighter but also quenches thirst.
	if (Inv->RemoveResource(EResourceType::Egg, 1))
	{
		Stats->Hunger = FMath::Min(Max, Stats->Hunger + 25.f);
	}
	else if (Inv->RemoveResource(EResourceType::Milk, 1))
	{
		Stats->Hunger = FMath::Min(Max, Stats->Hunger + 12.f);
		Stats->Thirst = FMath::Min(Max, Stats->Thirst + 25.f);
	}
}

// ---- Building --------------------------------------------------------------

void AEveraCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateLocomotionAnim();
	UpdateFlies(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);

	if (bBuildMode)
	{
		UpdateBuildGhost();
	}

	if (bMounted && CurrentHorse)
	{
		DriveMountedHorse(DeltaSeconds);
		// Clear the captured input; DoMove refills it each frame the keys are held,
		// so releasing the keys leaves it zero and the horse comes to a stop.
		MountMoveInput = FVector2D::ZeroVector;
	}
}

void AEveraCharacter::UpdateFlies(float TimeSeconds)
{
	if (Flies.Num() == 0)
	{
		return;
	}

	bool bBuzzing = false;
	if (SurvivalStats)
	{
		const float Frac = SurvivalStats->Hygiene / FMath::Max(1.f, SurvivalStats->GetMaxValue());
		bBuzzing = (Frac < 0.35f);
	}

	for (int32 i = 0; i < Flies.Num(); ++i)
	{
		UStaticMeshComponent* Fly = Flies[i];
		if (!Fly)
		{
			continue;
		}
		Fly->SetVisibility(bBuzzing);
		if (bBuzzing)
		{
			const float Angle = TimeSeconds * 3.f + i * (2.f * PI / Flies.Num());
			const float R = 24.f;
			Fly->SetRelativeLocation(FVector(FMath::Cos(Angle) * R, FMath::Sin(Angle) * R,
				82.f + FMath::Sin(TimeSeconds * 5.f + i) * 10.f));
		}
	}
}

void AEveraCharacter::SetupHeroAvatar()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	// Use the avatar the player picked on the menu (falls back to the defaults).
	FString MeshPath = HeroMeshPath;
	FString IdlePath = HeroIdlePath;
	FString WalkPath = HeroWalkPath;
	FString ChopPath;
	if (const UEveraGameInstance* GI = GetGameInstance<UEveraGameInstance>())
	{
		const int32 Idx = GI->AvatarIndex;
		if (Idx >= 0 && Idx < UEveraGameInstance::NumAvatars())
		{
			MeshPath = UEveraGameInstance::AvatarMeshPath(Idx);
			IdlePath = UEveraGameInstance::AvatarIdlePath(Idx);
			WalkPath = UEveraGameInstance::AvatarWalkPath(Idx);
			ChopPath = UEveraGameInstance::AvatarChopPath(Idx);
		}
	}

	USkeletalMesh* Hero = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
	if (!Hero)
	{
		return; // keep the default mesh if the hero pack isn't installed
	}

	// Drop any incompatible mannequin anim blueprint before swapping skeletons.
	MeshComp->SetAnimInstanceClass(nullptr);
	MeshComp->SetSkeletalMeshAsset(Hero);
	// Seat the mesh so the feet (at the mesh's local Z=0) meet the capsule bottom,
	// computed from the ACTUAL capsule half-height — a Blueprint may have overridden
	// the size we set in the constructor, which left the avatar floating. The small
	// extra nudge plants the feet through the movement component's floor-hover.
	const float CapHalf = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 78.f;
	MeshComp->SetRelativeLocation(FVector(HeroMeshOffset.X, HeroMeshOffset.Y, -CapHalf - 2.f));
	MeshComp->SetRelativeRotation(HeroMeshRotation);
	MeshComp->SetRelativeScale3D(FVector(HeroMeshScale));

	HeroIdleAnim = LoadObject<UAnimSequence>(nullptr, *IdlePath);
	HeroWalkAnim = LoadObject<UAnimSequence>(nullptr, *WalkPath);
	if (!ChopPath.IsEmpty())
	{
		HeroChopAnim = LoadObject<UAnimSequence>(nullptr, *ChopPath);
	}
	if (HeroIdleAnim)
	{
		MeshComp->PlayAnimation(HeroIdleAnim, true);
		CurrentHeroAnim = HeroIdleAnim;
	}
}

void AEveraCharacter::UpdateLocomotionAnim()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || !HeroIdleAnim || !HeroWalkAnim)
	{
		return;
	}

	// Let a one-shot gather/chop swing finish before locomotion takes back over.
	if (GetWorld() && GetWorld()->GetTimeSeconds() < GatherAnimEndTime)
	{
		return;
	}

	const float Speed = GetVelocity().Size2D();
	UAnimSequence* Want = (Speed > 10.f) ? HeroWalkAnim : HeroIdleAnim;
	if (Want != CurrentHeroAnim)
	{
		MeshComp->PlayAnimation(Want, true);
		CurrentHeroAnim = Want;
	}
}

bool AEveraCharacter::ComputeBuildPlacement(FTransform& OutXform) const
{
	if (!FollowCamera)
	{
		return false;
	}

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End = Start + FollowCamera->GetForwardVector() * BuildReach;

	FHitResult Hit;
	FCollisionQueryParams Params(FName(TEXT("BuildTrace")), /*bTraceComplex=*/false, this);
	if (BuildGhost)
	{
		Params.AddIgnoredActor(BuildGhost);
	}

	const FVector Target = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)
		? Hit.ImpactPoint
		: End;

	// Snap to a grid *corner*, then offset per piece so everything shares one
	// cell grid: floors fill a cell, walls sit on a cell edge, pillars land on
	// the corner. This makes rotated walls meet at clean corners to form rooms.
	const float G = ABuildPiece::GridSize;

	if (bFurnitureMode)
	{
		// Furniture places freely on a fine grid at the ground; rotate with R.
		const float FineGrid = 75.f;
		const float FX = FMath::RoundToFloat(Target.X / FineGrid) * FineGrid;
		const float FY = FMath::RoundToFloat(Target.Y / FineGrid) * FineGrid;

		float FGroundZ = Target.Z;
		FHitResult FDown;
		FCollisionQueryParams FDownParams(FName(TEXT("FurnDown")), false, this);
		if (BuildGhost)
		{
			FDownParams.AddIgnoredActor(BuildGhost);
		}
		if (GetWorld()->LineTraceSingleByChannel(FDown, FVector(FX, FY, Target.Z + 6000.f), FVector(FX, FY, Target.Z - 12000.f), ECC_Visibility, FDownParams))
		{
			FGroundZ = FDown.ImpactPoint.Z;
		}
		OutXform = FTransform(FRotator(0.f, BuildYaw, 0.f), FVector(FX, FY, FGroundZ + BuildLevel * G));
		return true;
	}

	const float CornerX = FMath::RoundToFloat(Target.X / G) * G;
	const float CornerY = FMath::RoundToFloat(Target.Y / G) * G;

	const FRotator Rot(0.f, BuildYaw, 0.f);
	FVector Offset(0.f, 0.f, 0.f);
	switch (BuildPieceType)
	{
	case EBuildPieceType::Floor:
	case EBuildPieceType::Roof:
		Offset = FVector(G * 0.5f, G * 0.5f, 0.f); // centre of the cell
		break;
	case EBuildPieceType::Wall:
	case EBuildPieceType::Doorway:
	case EBuildPieceType::Window:
		Offset = Rot.RotateVector(FVector(G * 0.5f, 0.f, 0.f)); // along a cell edge
		break;
	default: // Pillar sits right on the corner
		break;
	}

	const float PlaceX = CornerX + Offset.X;
	const float PlaceY = CornerY + Offset.Y;

	// Find the ground under the piece, then raise by the chosen storey level so
	// the player can build up floor by floor, Minecraft-style.
	float GroundZ = Target.Z;
	FHitResult Down;
	FCollisionQueryParams DownParams(FName(TEXT("BuildDown")), /*bTraceComplex=*/false, this);
	if (BuildGhost)
	{
		DownParams.AddIgnoredActor(BuildGhost);
	}
	const FVector DownStart(PlaceX, PlaceY, Target.Z + 6000.f);
	const FVector DownEnd(PlaceX, PlaceY, Target.Z - 12000.f);
	if (GetWorld()->LineTraceSingleByChannel(Down, DownStart, DownEnd, ECC_Visibility, DownParams))
	{
		GroundZ = Down.ImpactPoint.Z;
	}

	const float PlaceZ = GroundZ + BuildLevel * G; // walls are one grid tall
	OutXform = FTransform(Rot, FVector(PlaceX, PlaceY, PlaceZ));
	return true;
}

void AEveraCharacter::RaiseBuildLevel()
{
	if (bBuildMode)
	{
		BuildLevel = FMath::Clamp(BuildLevel + 1, 0, 12);
	}
}

void AEveraCharacter::LowerBuildLevel()
{
	if (bBuildMode)
	{
		BuildLevel = FMath::Clamp(BuildLevel - 1, 0, 12);
	}
}

void AEveraCharacter::UpdateBuildGhost()
{
	if (!BuildGhost)
	{
		return;
	}

	FTransform Xform;
	if (!ComputeBuildPlacement(Xform))
	{
		return;
	}
	BuildGhost->SetActorTransform(Xform);

	// Green preview if the player can afford the piece, red otherwise.
	bool bAffordable = true;
	if (const UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
	{
		const int32 Cost = bFurnitureMode ? ABuildPiece::FurnitureCost(FurnitureIndex) : ABuildPiece::GetWoodCost(BuildPieceType);
		bAffordable = Inv->HasResource(EResourceType::Wood, Cost);
	}
	BuildGhost->SetGhost(true, bAffordable);
}

void AEveraCharacter::ToggleBuildMode()
{
	bBuildMode = !bBuildMode;

	if (bBuildMode && !BuildGhost)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		BuildGhost = GetWorld()->SpawnActor<ABuildPiece>(ABuildPiece::StaticClass(), FTransform::Identity, SpawnParams);
		RefreshBuildGhost();
	}
	else if (!bBuildMode && BuildGhost)
	{
		BuildGhost->Destroy();
		BuildGhost = nullptr;
	}
}

void AEveraCharacter::RefreshBuildGhost()
{
	if (!BuildGhost)
	{
		return;
	}

	if (bFurnitureMode)
	{
		UStaticMesh* FurnMesh = LoadObject<UStaticMesh>(nullptr, *ABuildPiece::FurniturePath(FurnitureIndex));
		BuildGhost->SetFurnitureMesh(FurnMesh);
	}
	else
	{
		BuildGhost->SetFurnitureMesh(nullptr);
		BuildGhost->SetPieceType(BuildPieceType);
	}

	bool bAffordable = true;
	if (const UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
	{
		const int32 Cost = bFurnitureMode ? ABuildPiece::FurnitureCost(FurnitureIndex) : ABuildPiece::GetWoodCost(BuildPieceType);
		bAffordable = Inv->HasResource(EResourceType::Wood, Cost);
	}
	BuildGhost->SetGhost(true, bAffordable);
}

void AEveraCharacter::ToggleBuildCategory()
{
	if (!bBuildMode)
	{
		return;
	}
	bFurnitureMode = !bFurnitureMode;
	RefreshBuildGhost();
}

void AEveraCharacter::CycleBuildPiece()
{
	if (!bBuildMode)
	{
		return;
	}
	if (bFurnitureMode)
	{
		FurnitureIndex = (FurnitureIndex + 1) % ABuildPiece::NumFurniture();
	}
	else
	{
		BuildPieceType = ABuildPiece::NextType(BuildPieceType);
	}
	RefreshBuildGhost();
}

void AEveraCharacter::SelectBuildPiece(int32 Index)
{
	if (!bBuildMode || Index < 0)
	{
		return;
	}
	if (bFurnitureMode)
	{
		if (Index >= ABuildPiece::NumFurniture())
		{
			return;
		}
		FurnitureIndex = Index;
	}
	else
	{
		if (Index > static_cast<int32>(EBuildPieceType::Pillar))
		{
			return;
		}
		BuildPieceType = static_cast<EBuildPieceType>(Index);
	}
	RefreshBuildGhost();
}

void AEveraCharacter::RotateBuildPiece()
{
	if (!bBuildMode)
	{
		return;
	}
	BuildYaw = FMath::Fmod(BuildYaw + 90.f, 360.f);
}

void AEveraCharacter::PlaceBuildPiece()
{
	if (!bBuildMode)
	{
		return;
	}
	FTransform Xform;
	if (ComputeBuildPlacement(Xform))
	{
		if (bFurnitureMode)
		{
			ServerPlaceFurniture(static_cast<uint8>(FurnitureIndex), Xform);
		}
		else
		{
			ServerPlaceBuildPiece(static_cast<uint8>(BuildPieceType), Xform);
		}
	}
}

void AEveraCharacter::ServerPlaceFurniture_Implementation(uint8 FurnitureIdx, FTransform Xform)
{
	const int32 Cost = ABuildPiece::FurnitureCost(FurnitureIdx);
	UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>();
	if (!Inv || !Inv->HasResource(EResourceType::Wood, Cost))
	{
		return;
	}

	UStaticMesh* FurnMesh = LoadObject<UStaticMesh>(nullptr, *ABuildPiece::FurniturePath(FurnitureIdx));
	if (!FurnMesh)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (ABuildPiece* Piece = GetWorld()->SpawnActor<ABuildPiece>(ABuildPiece::StaticClass(), Xform, SpawnParams))
	{
		Piece->SetFurnitureMesh(FurnMesh);
		Inv->RemoveResource(EResourceType::Wood, Cost);
	}
}

void AEveraCharacter::ServerPlaceBuildPiece_Implementation(uint8 TypeIndex, FTransform Xform)
{
	const EBuildPieceType Type = static_cast<EBuildPieceType>(TypeIndex);
	const int32 Cost = ABuildPiece::GetWoodCost(Type);

	UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>();
	if (!Inv || !Inv->HasResource(EResourceType::Wood, Cost))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// A campfire is its own actor (it can be lit); everything else is a box piece.
	if (Type == EBuildPieceType::Campfire)
	{
		if (GetWorld()->SpawnActor<ACampfire>(ACampfire::StaticClass(), Xform, SpawnParams))
		{
			Inv->RemoveResource(EResourceType::Wood, Cost);
		}
		return;
	}

	if (ABuildPiece* Piece = GetWorld()->SpawnActor<ABuildPiece>(ABuildPiece::StaticClass(), Xform, SpawnParams))
	{
		Piece->SetPieceType(Type);
		Inv->RemoveResource(EResourceType::Wood, Cost);
	}
}

void AEveraCharacter::RemoveBuildPiece()
{
	if (!bBuildMode || !FollowCamera)
	{
		return;
	}

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End = Start + FollowCamera->GetForwardVector() * BuildReach;

	FHitResult Hit;
	FCollisionQueryParams Params(FName(TEXT("BuildRemove")), /*bTraceComplex=*/false, this);
	if (BuildGhost)
	{
		Params.AddIgnoredActor(BuildGhost);
	}
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (ABuildPiece* Piece = Cast<ABuildPiece>(Hit.GetActor()))
		{
			ServerRemoveBuildPiece(Piece);
		}
	}
}

void AEveraCharacter::ServerRemoveBuildPiece_Implementation(ABuildPiece* Piece)
{
	if (!Piece)
	{
		return;
	}
	const int32 Refund = FMath::Max(1, ABuildPiece::GetWoodCost(Piece->GetPieceType()) / 2);
	if (UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
	{
		Inv->AddResource(EResourceType::Wood, Refund);
	}
	Piece->Destroy();
}

void AEveraCharacter::ServerGather_Implementation(AResourceNode* Node)
{
	if (!Node)
	{
		return;
	}

	// Basic validation: the node must be within reach of the character.
	if (FVector::Dist(GetActorLocation(), Node->GetActorLocation()) > MaxGatherDistance)
	{
		return;
	}

	// Wood nodes need an axe; stone nodes need a pickaxe (separate tools).
	UCraftingComponent* Craft = FindComponentByClass<UCraftingComponent>();
	const ECraftableItem NeededTool = (Node->GetResourceType() == EResourceType::Wood)
		? ECraftableItem::StoneAxe
		: ECraftableItem::StonePickaxe;
	if (!Craft || Craft->GetCraftedCount(NeededTool) <= 0)
	{
		return;
	}

	Node->Gather(this);
}

void AEveraCharacter::ServerGatherPickup_Implementation(AResourcePickup* Pickup)
{
	if (!Pickup)
	{
		return;
	}
	if (FVector::Dist(GetActorLocation(), Pickup->GetActorLocation()) > MaxGatherDistance)
	{
		return;
	}

	// Gathered by hand — no tool needed. Grants the loose material and a little XP.
	const EResourceType Type = Pickup->GetResourceType();
	if (UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
	{
		Inv->AddResource(Type, Pickup->GetAmount());
	}
	if (USkillsComponent* SkillsComp = FindComponentByClass<USkillsComponent>())
	{
		SkillsComp->AddXP(Type == EResourceType::Wood ? ESkillType::Woodcutting : ESkillType::Mining, 1.f);
	}
	Pickup->Destroy();
}

void AEveraCharacter::CraftStonePickaxe()
{
	ServerCraftStonePickaxe();
}

void AEveraCharacter::ServerCraftStonePickaxe_Implementation()
{
	if (Crafting)
	{
		Crafting->TryCraftStonePickaxe();
	}
}

void AEveraCharacter::CraftStoneAxe()
{
	ServerCraftStoneAxe();
}

void AEveraCharacter::ServerCraftStoneAxe_Implementation()
{
	if (Crafting)
	{
		Crafting->TryCraftStoneAxe();
	}
}

void AEveraCharacter::ToggleInventory()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (AEveraHUD* HUD = Cast<AEveraHUD>(PC->GetHUD()))
		{
			HUD->ToggleInventory();
		}
	}
}

void AEveraCharacter::UpdateHeldAxe()
{
	if (HeldAxe && Crafting)
	{
		HeldAxe->SetVisibility(Crafting->GetCraftedCount(ECraftableItem::StoneAxe) > 0);
	}
}

void AEveraCharacter::PlayGatherAnim()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || !HeroChopAnim)
	{
		return;
	}

	// One-shot swing (chop/gather); locomotion is suppressed until it finishes.
	MeshComp->PlayAnimation(HeroChopAnim, /*bLooping=*/false);
	CurrentHeroAnim = nullptr;
	GatherAnimEndTime = GetWorld()->GetTimeSeconds() + HeroChopAnim->GetPlayLength();
}

void AEveraCharacter::DoMove(float Right, float Forward)
{
	// While riding, the input steers the horse instead of the player (consumed in Tick).
	if (bMounted && CurrentHorse)
	{
		MountMoveInput = FVector2D(Right, Forward);
		return;
	}

	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AEveraCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AEveraCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AEveraCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
