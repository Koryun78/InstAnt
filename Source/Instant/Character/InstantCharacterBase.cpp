#include "InstantCharacterBase.h"

#include "Instant/AbilitySystem/InstantAttributeSet.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Instant_State_Dead, "State.Dead");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Instant_State_Attacking, "State.Attacking");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Instant_State_Stunned, "State.Stunned");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Instant_State_Dodging, "State.Dodging");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Instant_State_Invulnerable, "State.Invulnerable");

AInstantCharacterBase::AInstantCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = false;
	SetReplicateMovement(false);

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false);

	AttributeSet = CreateDefaultSubobject<UInstantAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AInstantCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AInstantCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (DefaultAttributesEffect)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();

		Context.AddSourceObject(this);

		const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1.0f, Context);

		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	if (PrimaryAbility)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(PrimaryAbility, 1));
	}

	HealthChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UInstantAttributeSet::GetHealthAttribute()).AddUObject(this, &AInstantCharacterBase::HandleHealthChanged);

	if (GetHealth() <= 0.0f)
	{
		Die();
	}
}

void AInstantCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void AInstantCharacterBase::UnPossessed()
{
	CancelAttack();

	Super::UnPossessed();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void AInstantCharacterBase::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	CancelAttack();

	if (HealthChangedHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UInstantAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);

		HealthChangedHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void AInstantCharacterBase::ActivatePrimaryAbility()
{
	if (IsAlive() && PrimaryAbility)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(PrimaryAbility);
	}
}

bool AInstantCharacterBase::IsAlive() const
{
	return !bHasDied && !IsActorBeingDestroyed() && GetHealth() > 0.0f;
}

bool AInstantCharacterBase::IsAttacking() const
{
	return bIsAttacking || AbilitySystemComponent->HasMatchingGameplayTag(TAG_Instant_State_Attacking);
}

bool AInstantCharacterBase::CanAct() const
{
	return IsAlive() && !IsAttackBlocked();
}

float AInstantCharacterBase::GetHealth() const
{
	return AttributeSet->GetHealth();
}

float AInstantCharacterBase::GetMaxHealth() const
{
	return AttributeSet->GetMaxHealth();
}

bool AInstantCharacterBase::IsHostileTo(
	const AInstantCharacterBase* Other) const
{
	return IsValid(Other) && Other != this && CombatTeam != Other->CombatTeam;
}

bool AInstantCharacterBase::IsAttackBlocked() const
{
	return AbilitySystemComponent->HasMatchingGameplayTag(TAG_Instant_State_Dead) || AbilitySystemComponent->HasMatchingGameplayTag(TAG_Instant_State_Stunned) || AbilitySystemComponent->HasMatchingGameplayTag(TAG_Instant_State_Dodging);
}

bool AInstantCharacterBase::TryAttack()
{
	const UGameplayEffect* Effect = DamageEffect.GetDefaultObject();

	if (!CanAct() || IsAttacking() || !Effect || Effect->DurationPolicy != EGameplayEffectDurationType::Instant)
	{
		return false;
	}

	bIsAttacking = true;
	bAttackHitResolved = false;

	GetCharacterMovement()->StopMovementImmediately();
	ConsumeMovementInputVector();

	AbilitySystemComponent->AddLooseGameplayTag(TAG_Instant_State_Attacking);

	const float Windup = FMath::Max(0.0f, AttackWindup);

	GetWorldTimerManager().SetTimer(AttackEndTimer, this, &AInstantCharacterBase::CancelAttack, FMath::Max(AttackDuration, Windup + 0.01f), false);

	if (Windup > 0.0f)
	{
		GetWorldTimerManager().SetTimer(AttackHitTimer, this, &AInstantCharacterBase::PerformAttackHit, Windup, false);
	}

	BP_OnAttackStarted();

	if (Windup <= 0.0f)
	{
		PerformAttackHit();
	}

	return true;
}

void AInstantCharacterBase::CancelAttack()
{
	GetWorldTimerManager().ClearTimer(AttackHitTimer);
	GetWorldTimerManager().ClearTimer(AttackEndTimer);

	if (bIsAttacking)
	{
		bIsAttacking = false;

		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Instant_State_Attacking);
	}
}

void AInstantCharacterBase::PerformAttackHit()
{
	if (bAttackHitResolved)
	{
		return;
	}

	if (!bIsAttacking || !IsAlive() || IsAttackBlocked())
	{
		CancelAttack();
		return;
	}

	bAttackHitResolved = true;

	const FVector Start = GetActorLocation();
	const FVector Forward = GetActorForwardVector();
	const float Range = FMath::Max(0.0f, AttackRange);

	FCollisionQueryParams SweepParams(SCENE_QUERY_STAT(InstantMeleeSweep), false, this);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FHitResult> Hits;

	GetWorld()->SweepMultiByObjectType(Hits, Start, Start + Forward * Range, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(FMath::Max(1.0f, AttackRadius)), SweepParams);

	AInstantCharacterBase* Target = nullptr;
	float ClosestDistanceSquared = FMath::Square(Range);

	for (const FHitResult& Hit : Hits)
	{
		AInstantCharacterBase* Candidate = Cast<AInstantCharacterBase>(Hit.GetActor());

		if (!IsValid(Candidate) || !Candidate->IsAlive() || !IsHostileTo(Candidate))
		{
			continue;
		}

		const FVector ToCandidate = Candidate->GetActorLocation() - Start;

		const float DistanceSquared = ToCandidate.SizeSquared();

		if (DistanceSquared > ClosestDistanceSquared || FVector::DotProduct(Forward, ToCandidate) <= 0.0f)
		{
			continue;
		}

		// Проверяем, не закрывает ли цель стена.
		FCollisionQueryParams SightParams(SCENE_QUERY_STAT(InstantMeleeVisibility), false, this);

		SightParams.AddIgnoredActor(Candidate);

		FHitResult WallHit;

		if (GetWorld()->LineTraceSingleByChannel(WallHit, Start, Candidate->GetActorLocation(), ECC_Visibility, SightParams))
		{
			continue;
		}

		Target = Candidate;
		ClosestDistanceSquared = DistanceSquared;
	}

	if (!IsValid(Target) || !Target->IsAlive() || Target->AbilitySystemComponent->HasMatchingGameplayTag(TAG_Instant_State_Invulnerable))
	{
		return;
	}

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();

	Context.AddSourceObject(this);

	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(DamageEffect, 1.0f, Context);

	if (!Spec.IsValid())
	{
		return;
	}

	const FActiveGameplayEffectHandle Result = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), Target->AbilitySystemComponent);

	if (Result.WasSuccessfullyApplied())
	{
		BP_OnAttackHit(Target);
	}
}

void AInstantCharacterBase::HandleHealthChanged(
	const FOnAttributeChangeData& Data)
{
	if (bHasDied)
	{
		return;
	}

	if (Data.NewValue < Data.OldValue)
	{
		BP_OnDamageReceived(Data.OldValue - Data.NewValue);
	}

	if (GetHealth() <= 0.0f)
	{
		Die();
	}
}

void AInstantCharacterBase::Die()
{
	if (bHasDied)
	{
		return;
	}

	bHasDied = true;

	CancelAttack();

	AbilitySystemComponent->AddLooseGameplayTag(TAG_Instant_State_Dead);

	AbilitySystemComponent->CancelAllAbilities();

	StopJumping();

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	SetActorEnableCollision(false);

	BP_OnDeath();
}