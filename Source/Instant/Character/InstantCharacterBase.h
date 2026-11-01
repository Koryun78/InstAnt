#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "InstantCharacterBase.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
class UInstantAttributeSet;
struct FOnAttributeChangeData;

UENUM(BlueprintType)
enum class EInstantCombatTeam : uint8
{
	Player,
	Enemy
};

UCLASS()
class INSTANT_API AInstantCharacterBase
	: public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AInstantCharacterBase();

	virtual UAbilitySystemComponent*
		GetAbilitySystemComponent() const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	UFUNCTION(BlueprintCallable, Category = "Instant|Abilities")
	void ActivatePrimaryAbility();

	UFUNCTION(BlueprintPure, Category = "Instant|Combat")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "Instant|Combat")
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "Instant|Combat")
	bool CanAct() const;

	UFUNCTION(BlueprintPure, Category = "Instant|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Instant|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Instant|Combat")
	bool IsHostileTo(const AInstantCharacterBase* Other) const;

	UFUNCTION(BlueprintCallable, Category = "Instant|Combat")
	bool TryAttack();

	UFUNCTION(BlueprintCallable, Category = "Instant|Combat")
	void CancelAttack();

	UFUNCTION(BlueprintImplementableEvent, Category = "Instant|Combat")
	void BP_OnAttackStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Instant|Combat")
	void BP_OnAttackHit(AInstantCharacterBase* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Instant|Combat")
	void BP_OnDamageReceived(float Damage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Instant|Combat")
	void BP_OnDeath();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Die();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UInstantAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> PrimaryAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Instant|Combat")
	EInstantCombatTeam CombatTeam = EInstantCombatTeam::Player;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Instant|Combat")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Instant|Combat", meta = (ClampMin = "0"))
	float AttackRange = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Instant|Combat", meta = (ClampMin = "1"))
	float AttackRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Instant|Combat", meta = (ClampMin = "0"))
	float AttackWindup = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Instant|Combat", meta = (ClampMin = "0.01"))
	float AttackDuration = 0.6f;

private:
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void PerformAttackHit();

	bool IsAttackBlocked() const;

	bool bHasDied = false;
	bool bIsAttacking = false;
	bool bAttackHitResolved = false;

	FDelegateHandle HealthChangedHandle;

	FTimerHandle AttackHitTimer;
	FTimerHandle AttackEndTimer;
};