#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "InstantCharacter.generated.h"

class UAbilitySystemComponent;
class UCameraComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UGameplayAbility;
class UGameplayEffect;
class UInputAction;
class UInputMappingContext;
class UInstantAttributeSet;
class USpringArmComponent;
struct FInputActionValue;

UCLASS()
class INSTANT_API AInstantCharacter
	: public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AInstantCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void SetupPlayerInputComponent(
		UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Instant|Abilities")
	void ActivatePrimaryAbility();

	UFUNCTION(BlueprintPure, Category = "Instant|Movement")
	bool IsSprinting() const;

	UFUNCTION(BlueprintCallable, Category = "Instant|Movement")
	void StopSprinting();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason) override;

	// Камера

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// Скорости

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement",
		meta = (ClampMin = "0"))
	float WalkSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement",
		meta = (ClampMin = "0"))
	float SprintSpeed = 750.0f;

	// GAS

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UInstantAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> PrimaryAbility;

	// Назначаются в Blueprint персонажа

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

private:
	void AddInputMappingContext();
	void RemoveInputMappingContext();

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartSprinting();

	bool bIsSprinting = false;

	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;
};