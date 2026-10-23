#include "InstantCharacter.h"

#include "Instant/AbilitySystem/InstantAttributeSet.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayEffect.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

AInstantCharacter::AInstantCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = false;
	SetReplicateMovement(false);

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(
		TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 60.0f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	AbilitySystemComponent =CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false);

	AttributeSet = CreateDefaultSubobject<UInstantAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AInstantCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AInstantCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (DefaultAttributesEffect)
	{
		FGameplayEffectContextHandle Context =AbilitySystemComponent->MakeEffectContext();

		Context.AddSourceObject(this);

		const FGameplayEffectSpecHandle Spec =AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1.0f, Context);

		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	if (PrimaryAbility)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(PrimaryAbility, 1));
	}
}

void AInstantCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void AInstantCharacter::UnPossessed()
{
	StopSprinting();
	StopJumping();
	RemoveInputMappingContext();

	Super::UnPossessed();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void AInstantCharacter::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	RemoveInputMappingContext();

	Super::EndPlay(EndPlayReason);
}

void AInstantCharacter::ActivatePrimaryAbility()
{
	if (AbilitySystemComponent && PrimaryAbility)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(PrimaryAbility);
	}
}

bool AInstantCharacter::IsSprinting() const
{
	return bIsSprinting;
}

void AInstantCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!Input)
	{
		return;
	}

	AddInputMappingContext();

	if (MoveAction)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AInstantCharacter::Move);
	}

	if (LookAction)
	{
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AInstantCharacter::Look);
	}

	if (JumpAction)
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		Input->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ACharacter::StopJumping);
	}

	if (SprintAction)
	{
		Input->BindAction(SprintAction, ETriggerEvent::Started, this, &AInstantCharacter::StartSprinting);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AInstantCharacter::StopSprinting);
		Input->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AInstantCharacter::StopSprinting);
	}
}

void AInstantCharacter::AddInputMappingContext()
{
	const APlayerController* PlayerController =
		Cast<APlayerController>(GetController());

	if (!DefaultMappingContext ||
		!PlayerController ||
		!PlayerController->IsLocalController())
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(): nullptr;

	if (Subsystem)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
		InputSubsystem = Subsystem;
	}
}

void AInstantCharacter::RemoveInputMappingContext()
{
	if (InputSubsystem.IsValid() && DefaultMappingContext)
	{
		InputSubsystem->RemoveMappingContext(DefaultMappingContext);
	}

	InputSubsystem.Reset();
}

void AInstantCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D Direction =Value.Get<FVector2D>().GetClampedToMaxSize(1.0f);
	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Direction.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Direction.X);
}

void AInstantCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(-LookAxis.Y);
}

void AInstantCharacter::StartSprinting()
{
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AInstantCharacter::StopSprinting()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}