#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Instant/Menu/Core/InstantMenuTypes.h"
#include "TimerManager.h"
#include "InstantMenuBookActor.generated.h"

class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInstantMenuBookStateChanged, EInstantBookState, NewState);

UCLASS(Blueprintable)
class INSTANT_API AInstantMenuBookActor : public AActor
{
	GENERATED_BODY()

public:
	AInstantMenuBookActor();

	// Сюда позже прикрепим визуальные компоненты книги.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu|Book")
	TObjectPtr<USceneComponent> VisualRoot;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Menu|Book")
	EInstantBookState State = EInstantBookState::Closed;

	UPROPERTY(BlueprintAssignable, Category = "Menu|Book")
	FInstantMenuBookStateChanged OnStateChanged;

	// Пока нет анимации, завершать открытие по таймеру.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Book")
	bool bAutoCompleteOpening = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Book", meta = (ClampMin = "0.01", EditCondition = "bAutoCompleteOpening"))
	float OpeningDuration = 1.5f;

	UFUNCTION(BlueprintCallable, Category = "Menu|Book")
	void OpenBook();

	// Позже вызывается из Blueprint по завершении анимации.
	UFUNCTION(BlueprintCallable, Category = "Menu|Book")
	void FinishOpening();

	UFUNCTION(BlueprintCallable, Category = "Menu|Book")
	void CancelOpening();

	// Точка подключения будущей анимации.
	UFUNCTION(BlueprintImplementableEvent, Category = "Menu|Book")
	void PlayOpeningAnimation();

protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	FTimerHandle OpeningTimer;
	bool bEndingPlay = false;
};