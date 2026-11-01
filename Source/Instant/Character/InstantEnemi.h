#pragma once

#include "CoreMinimal.h"
#include "InstantCharacterBase.h"
#include "TimerManager.h"
#include "InstantEnemi.generated.h"

UCLASS()
class INSTANT_API AInstantEnemi : public AInstantCharacterBase
{
	GENERATED_BODY()

public:
	AInstantEnemi();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason) override;

	virtual void Die() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Instant|AI", meta = (ClampMin = "0"))
	float AggroRange = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Instant|AI", meta = (ClampMin = "0.05"))
	float ThinkInterval = 0.2f;

private:
	void Think();
	void FaceTarget(const AActor* Target);

	FTimerHandle ThinkTimer;
};