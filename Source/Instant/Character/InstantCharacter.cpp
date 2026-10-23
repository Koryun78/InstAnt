#include "InstantCharacter.h"
#include "Instant/AbilitySystem/InstantAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"

AInstantCharacter::AInstantCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

    AbilitySystemComponent =
        CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

    AttributeSet = CreateDefaultSubobject<UInstantAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AInstantCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}


void AInstantCharacter::BeginPlay()
{
	Super::BeginPlay();

    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    if (!HasAuthority())
    {
        return;
    }

    if (DefaultAttributesEffect)
    {
        FGameplayEffectSpecHandle Spec =
            AbilitySystemComponent->MakeOutgoingSpec(
                DefaultAttributesEffect,
                1.0f,
                AbilitySystemComponent->MakeEffectContext()
            );

        if (Spec.IsValid())
        {
            AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }

        if (PrimaryAbility)
        {
            AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(PrimaryAbility, 1));

            // Temporary GAS test:
            // AbilitySystemComponent->TryActivateAbilityByClass(
            //     PrimaryAbility);
        }
    }
}

void AInstantCharacter::ActivatePrimaryAbility()
{
    if (AbilitySystemComponent && PrimaryAbility)
    {
        AbilitySystemComponent->TryActivateAbilityByClass(
            PrimaryAbility);
    }
}

void AInstantCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInstantCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

