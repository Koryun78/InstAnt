#include "InstantCharacter.h"

#include "Instant/AbilitySystem/InstantAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
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
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Instant_State_Sprinting, "State.Sprinting");

AInstantCharacter::AInstantCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 60.0f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AInstantCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AInstantCharacter::UnPossessed()
{
	StopSprinting();
	StopJumping();
	RemoveInputMappingContext();

	Super::UnPossessed();
}

void AInstantCharacter::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	StopSprinting();
	RemoveInputMappingContext();

	Super::EndPlay(EndPlayReason);
}

void AInstantCharacter::Die()
{
	Super::Die();

	StopSprinting();
	StopJumping();
	RemoveInputMappingContext();

}

void AInstantCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsSprinting)
	{
		return;
	}

	if (AttributeSet->GetStamina() <= 0.0f || !CanAct() || IsAttacking())
	{
		StopSprinting();
		return;
	}

	if (!GetCharacterMovement()->IsMovingOnGround() || GetVelocity().SizeSquared2D() <= 1.0f)
	{
		return;
	}

	SprintStaminaElapsed += DeltaSeconds;

	const float Interval = FMath::Max(StaminaCostInterval, 0.05f);

	while (SprintStaminaElapsed >= Interval)
	{
		const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(StaminaCostEffect, 1.0f, AbilitySystemComponent->MakeEffectContext());

		if (!Spec.IsValid())
		{
			StopSprinting();
			return;
		}

		const FActiveGameplayEffectHandle Result = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

		if (!Result.WasSuccessfullyApplied())
		{
			StopSprinting();
			return;
		}

		SprintStaminaElapsed -= Interval;

		if (AttributeSet->GetStamina() <= 0.0f)
		{
			StopSprinting();
			return;
		}
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

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);

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
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &AInstantCharacter::JumpPressed);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		Input->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ACharacter::StopJumping);
	}

	if (SprintAction)
	{
		Input->BindAction(SprintAction, ETriggerEvent::Started, this, &AInstantCharacter::StartSprinting);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AInstantCharacter::StopSprinting);
		Input->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AInstantCharacter::StopSprinting);
	}

	if (AttackAction)
	{
		Input->BindAction(AttackAction, ETriggerEvent::Started, this, &AInstantCharacter::Attack);
	}
}

void AInstantCharacter::AddInputMappingContext()
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!DefaultMappingContext || !PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;

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
	if (!Controller || !CanAct() || IsAttacking())
	{
		return;
	}

	const FVector2D Direction = Value.Get<FVector2D>().GetClampedToMaxSize(1.0f);
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

void AInstantCharacter::JumpPressed()
{
	if (CanAct() && !IsAttacking())
	{
		Jump();
	}
}

void AInstantCharacter::StartSprinting()
{
	if (bIsSprinting || !CanAct() || IsAttacking())
	{
		return;
	}

	const UGameplayEffect* CostEffect = StaminaCostEffect.GetDefaultObject();

	if (!CostEffect || CostEffect->DurationPolicy != EGameplayEffectDurationType::Instant || AttributeSet->GetStamina() <= 0.0f)
	{
		return;
	}

	bIsSprinting = true;

	AbilitySystemComponent->AddLooseGameplayTag(TAG_Instant_State_Sprinting);

	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AInstantCharacter::StopSprinting()
{
	if (bIsSprinting)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Instant_State_Sprinting);
	}

	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AInstantCharacter::Attack()
{
	if (!CanAct() || IsAttacking())
	{
		return;
	}

	StopSprinting();

	if (Controller)
	{
		SetActorRotation(FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f));
	}

	TryAttack();
}