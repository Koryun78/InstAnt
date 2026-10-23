#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InstantCharacter.generated.h"

class UAbilitySystemComponent;
class UInstantAttributeSet;
class UGameplayEffect;
class UGameplayAbility;

UCLASS()
class INSTANT_API AInstantCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AInstantCharacter();

    virtual UAbilitySystemComponent*
        GetAbilitySystemComponent() const override;

    UFUNCTION(BlueprintCallable, Category = "Instant|Abilities")
    void ActivatePrimaryAbility();

protected:
	virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UInstantAttributeSet> AttributeSet;

    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TSubclassOf<UGameplayAbility> PrimaryAbility;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
