// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InstantEffectActor.generated.h"

class UGameplayEffect;
class UPrimitiveComponent;
class USphereComponent;

UCLASS()
class INSTANT_API AInstantEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AInstantEffectActor();

	UFUNCTION(BlueprintCallable, Category = "Instant|Effects")
	bool ApplyEffectToActor(AActor* TargetActor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant|Effects")
	TSubclassOf<UGameplayEffect> EffectToApply;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant|Effects",
		meta = (ClampMin = "0"))
	float EffectLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant|Effects")
	bool bDestroyOnEffectApplied = true;

private:
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};