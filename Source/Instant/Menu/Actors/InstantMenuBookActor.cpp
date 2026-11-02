#include "InstantMenuBookActor.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"

AInstantMenuBookActor::AInstantMenuBookActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
	VisualRoot->SetupAttachment(RootComponent);
}

void AInstantMenuBookActor::OpenBook()
{
	if (bEndingPlay || State != EInstantBookState::Closed)
	{
		return;
	}

	State = EInstantBookState::Opening;

	if (bAutoCompleteOpening)
	{
		if (UWorld* World = GetWorld())
		{
			const float Duration = FMath::IsFinite(OpeningDuration) ? FMath::Max(OpeningDuration, 0.01f) : 1.5f;

			// Таймер запускается до Blueprint-событий:
			// они могут сразу завершить или отменить открытие.
			World->GetTimerManager().SetTimer(OpeningTimer, this, &ThisClass::FinishOpening, Duration, false);
		}
	}

	OnStateChanged.Broadcast(State);

	if (IsValid(this) && !bEndingPlay && State == EInstantBookState::Opening)
	{
		PlayOpeningAnimation();
	}
}

void AInstantMenuBookActor::FinishOpening()
{
	if (bEndingPlay || State != EInstantBookState::Opening)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(OpeningTimer);
	}

	State = EInstantBookState::Open;
	OnStateChanged.Broadcast(State);
}

void AInstantMenuBookActor::CancelOpening()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(OpeningTimer);
	}

	if (!bEndingPlay && State == EInstantBookState::Opening)
	{
		State = EInstantBookState::Closed;
		OnStateChanged.Broadcast(State);
	}
}

void AInstantMenuBookActor::EndPlay(
	const EEndPlayReason::Type Reason)
{
	bEndingPlay = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(OpeningTimer);
	}

	if (State == EInstantBookState::Opening)
	{
		State = EInstantBookState::Closed;
	}

	Super::EndPlay(Reason);
}