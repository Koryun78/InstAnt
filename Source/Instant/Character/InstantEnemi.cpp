#include "InstantEnemi.h"

#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AInstantEnemi::AInstantEnemi()
{
	CombatTeam = EInstantCombatTeam::Enemy;

	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->MaxWalkSpeed = 250.0f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AInstantEnemi::BeginPlay()
{
	Super::BeginPlay();

	if (IsAlive())
	{
		GetWorldTimerManager().SetTimer(ThinkTimer, this, &AInstantEnemi::Think, FMath::Max(ThinkInterval, 0.05f), true);
	}
}

void AInstantEnemi::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ThinkTimer);

	Super::EndPlay(EndPlayReason);
}

void AInstantEnemi::Die()
{
	GetWorldTimerManager().ClearTimer(ThinkTimer);

	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
		AI->ClearFocus(EAIFocusPriority::Gameplay);
	}

	Super::Die();
}

void AInstantEnemi::Think()
{
	if (!IsAlive())
	{
		return;
	}

	AAIController* AI = Cast<AAIController>(GetController());

	if (!AI)
	{
		return;
	}

	if (!CanAct())
	{
		AI->StopMovement();
		AI->ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	AInstantCharacterBase* Target = Cast<AInstantCharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (!IsValid(Target) || !Target->IsAlive() || !IsHostileTo(Target) || GetSquaredDistanceTo(Target) >
		FMath::Square(FMath::Max(AggroRange, 0.0f)))
	{
		AI->StopMovement();
		AI->ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	if (IsAttacking())
	{
		AI->StopMovement();
		AI->SetFocus(Target);
		FaceTarget(Target);
		return;
	}

	if (GetDistanceTo(Target) > AttackRange || !AI->LineOfSightTo(Target))
	{
		AI->ClearFocus(EAIFocusPriority::Gameplay);

		AI->MoveToActor(Target, FMath::Max(AttackRange * 0.5f, 1.0f), false);

		return;
	}

	AI->StopMovement();
	AI->SetFocus(Target);

	FaceTarget(Target);
	TryAttack();
}

void AInstantEnemi::FaceTarget(const AActor* Target)
{
	FVector Direction = Target->GetActorLocation() - GetActorLocation();

	Direction.Z = 0.0f;

	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
}