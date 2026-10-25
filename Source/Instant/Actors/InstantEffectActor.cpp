#include "InstantEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameplayEffect.h"

AInstantEffectActor::AInstantEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	Sphere = CreateDefaultSubobject<USphereComponent>(
		TEXT("Sphere"));

	SetRootComponent(Sphere);

	Sphere->InitSphereRadius(100.0f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetGenerateOverlapEvents(true);

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AInstantEffectActor::OnSphereOverlap);
}

bool AInstantEffectActor::ApplyEffectToActor(AActor* TargetActor)
{
	if (IsActorBeingDestroyed() || !IsValid(TargetActor) || TargetActor == this || !EffectToApply)
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!TargetASC)
	{
		return false;
	}

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();

	Context.AddSourceObject(this);

	const FGameplayEffectSpecHandle Spec = TargetASC->MakeOutgoingSpec(EffectToApply, EffectLevel, Context);

	if (!Spec.IsValid())
	{
		return false;
	}

	const FActiveGameplayEffectHandle Handle = TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (!Handle.WasSuccessfullyApplied())
	{
		return false;
	}

	if (bDestroyOnEffectApplied)
	{
		Destroy();
	}

	return true;
}

void AInstantEffectActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Обрабатываем капсулу персонажа, чтобы его меш
	// не вызвал повторное применение эффекта.
	if (const ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		if (OtherComp != Character->GetCapsuleComponent())
		{
			return;
		}
	}

	ApplyEffectToActor(OtherActor);
}