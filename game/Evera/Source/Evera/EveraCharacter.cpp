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
#include "ResourceNode.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Evera.h"

AEveraCharacter::AEveraCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
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

void AEveraCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !bSpawnTestResourceNodes)
	{
		return;
	}

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

		// Drop the node onto the ground beneath the ring position.
		FHitResult Ground;
		const FVector TraceStart = SpawnLocation + FVector(0.f, 0.f, 300.f);
		const FVector TraceEnd = SpawnLocation - FVector(0.f, 0.f, 1000.f);
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
		if (AResourceNode* Node = Cast<AResourceNode>(Hit.GetActor()))
		{
			ServerGather(Node);
		}
	}
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

	Node->Gather(this);
}

void AEveraCharacter::DoMove(float Right, float Forward)
{
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
