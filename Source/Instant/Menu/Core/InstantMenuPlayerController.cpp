#include "InstantMenuPlayerController.h"

#include "Instant/Menu/Actors/InstantMenuBookActor.h"
#include "Instant/Menu/Widgets/InstantMainMenuWidget.h"

#include "GameFramework/PlayerController.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "UnrealClient.h"

AInstantMenuPlayerController::AInstantMenuPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	MenuWidgetClass = UInstantMainMenuWidget::StaticClass();
	bShowMouseCursor = true;
}

void AInstantMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	// Подключаем существующую книгу на карте.
	if (!IsValid(Book))
	{
		for (TActorIterator<AInstantMenuBookActor> It(GetWorld()); It; ++It)
		{
			SetMenuBook(*It);
			break;
		}
	}

	if (!IsValid(Book))
	{
		UE_LOG(LogTemp, Warning, TEXT("Menu: place an InstantMenuBookActor or call SetMenuBook."));
	}

	if (!MenuWidgetClass)
	{
		return;
	}

	Menu = CreateWidget<UInstantMainMenuWidget>(this, MenuWidgetClass);

	if (!Menu)
	{
		return;
	}

	Menu->AddToViewport();
	Menu->InitializeMenu(this);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Menu->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
}

void AInstantMenuPlayerController::SetMenuBook(
	AInstantMenuBookActor* InBook)
{
	if (Book == InBook)
	{
		return;
	}

	CancelVineInteraction();

	if (IsValid(Book))
	{
		Book->OnStateChanged.RemoveDynamic(this, &ThisClass::HandleBookStateChanged);

		if (bRequestedOpening)
		{
			Book->CancelOpening();
		}
	}

	bRequestedOpening = false;
	Book = IsValid(InBook) ? InBook : nullptr;

	if (Book)
	{
		Book->OnStateChanged.AddUniqueDynamic(this, &ThisClass::HandleBookStateChanged);
	}

	HandleBookStateChanged(GetBookState());
}

EInstantBookState AInstantMenuPlayerController::GetBookState() const
{
	return IsValid(Book) ? Book->State : EInstantBookState::Closed;
}

void AInstantMenuPlayerController::HandleBookStateChanged(
	EInstantBookState State)
{
	LastBookState = State;

	if (State != EInstantBookState::Opening)
	{
		bRequestedOpening = false;
	}

	CancelVineInteraction();
	OnBookStateChanged.Broadcast(State);
}

bool AInstantMenuPlayerController::RequestStart()
{
	if (bEndingPlay || !Menu || !Menu->IsInViewport() || VineState.IsHolding() || !IsValid(Book) || Book->State != EInstantBookState::Closed)
	{
		return false;
	}

	bRequestedOpening = true;
	Book->OpenBook();

	return true;
}

bool AInstantMenuPlayerController::CanInteractWithMenu() const
{
	return !bEndingPlay && Menu && Menu->IsInViewport() && GetBookState() == EInstantBookState::Open && !VineState.IsHolding();
}

bool AInstantMenuPlayerController::HasMenuViewportFocus() const
{
	const ULocalPlayer* Local = GetLocalPlayer();

	if (bEndingPlay || !Menu || !Menu->IsInViewport() || !Local || !Local->ViewportClient || !Local->ViewportClient->Viewport || !Local->ViewportClient->Viewport->IsForegroundWindow())
	{
		return false;
	}

	// При UIOnly фокус находится на UMG или его дочерней кнопке.
	class APlayerController* L_Owner = Menu->GetOwningPlayer();

	return L_Owner &&(Menu->HasUserFocus(L_Owner) ||Menu->HasUserFocusedDescendants(L_Owner));
}

bool AInstantMenuPlayerController::UpdateVineProximity(EInstantMenuSide Side, bool bInside, FVector2D ViewportPosition)
{
	if (!HasMenuViewportFocus() || GetBookState() == EInstantBookState::Opening)
	{
		return false;
	}

	if (!VineState.UpdateProximity(Side, bInside, ViewportPosition, VineHoldSeconds))
	{
		return false;
	}

	OnVineInteractionChanged.Broadcast(Side, true, ViewportPosition);

	return true;
}

void AInstantMenuPlayerController::NotifyVineReleased(EInstantMenuSide Side, FVector2D Position)
{
	OnVineInteractionChanged.Broadcast(Side, false, Position);
}

void AInstantMenuPlayerController::CancelVineInteraction()
{
	const EInstantMenuSide Side = VineState.GetSide();
	const FVector2D Position = VineState.GetPosition();
	const bool bWasHolding = VineState.IsHolding();

	VineState.Reset();

	if (bWasHolding)
	{
		NotifyVineReleased(Side, Position);
	}
}

void AInstantMenuPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Книга могла быть удалена во время перехода.
	if (GetBookState() != LastBookState)
	{
		HandleBookStateChanged(GetBookState());
	}

	if (!HasMenuViewportFocus())
	{
		CancelVineInteraction();
		return;
	}

	if (!VineState.IsHolding())
	{
		return;
	}

	const EInstantMenuSide Side = VineState.GetSide();
	const FVector2D Position = VineState.GetPosition();

	if (VineState.Advance(DeltaSeconds))
	{
		NotifyVineReleased(Side, Position);
		return;
	}

	if (bHoldSystemCursor)
	{
		int32 Width = 0;
		int32 Height = 0;

		GetViewportSize(Width, Height);

		if (Width > 0 && Height > 0)
		{
			const int32 CursorX = FMath::RoundToInt(FMath::Clamp(Position.X, 0., static_cast<double>(Width - 1)));
			const int32 CursorY = FMath::RoundToInt(FMath::Clamp(Position.Y, 0., static_cast<double>(Height - 1)));

			SetMouseLocation(CursorX, CursorY);
		}
	}
}

void AInstantMenuPlayerController::SetContinueAvailable(bool bAvailable)
{
	bContinueAvailable = bAvailable;

	if (Menu)
	{
		Menu->RefreshInteractionState();
	}
}

bool AInstantMenuPlayerController::CanContinueGame() const
{
	return CanInteractWithMenu() && bContinueAvailable && OnContinueRequested.IsBound();
}

bool AInstantMenuPlayerController::CanStartNewGame() const
{
	return CanInteractWithMenu() && (OnNewGameRequested.IsBound() || !NewGameMap.IsNull());
}

bool AInstantMenuPlayerController::CanLoadGame() const
{
	return CanInteractWithMenu() && OnLoadGameRequested.IsBound();
}

bool AInstantMenuPlayerController::RequestContinue()
{
	if (!CanContinueGame())
	{
		return false;
	}

	OnContinueRequested.Broadcast();

	return true;
}

bool AInstantMenuPlayerController::RequestNewGame()
{
	if (!CanStartNewGame())
	{
		return false;
	}

	if (OnNewGameRequested.IsBound())
	{
		OnNewGameRequested.Broadcast();
	}
	else
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, NewGameMap);
	}

	return true;
}

bool AInstantMenuPlayerController::RequestLoadGame(const FString& SlotName, int32 UserIndex)
{
	if (!CanLoadGame() || SlotName.IsEmpty() || UserIndex < 0)
	{
		return false;
	}

	OnLoadGameRequested.Broadcast(SlotName, UserIndex);

	return true;
}

void AInstantMenuPlayerController::RequestQuit()
{
	if (CanInteractWithMenu())
	{
		UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
	}
}

void AInstantMenuPlayerController::EndPlay(
	const EEndPlayReason::Type Reason)
{
	bEndingPlay = true;

	CancelVineInteraction();

	if (Menu)
	{
		Menu->ShutdownMenu();
		Menu->RemoveFromParent();
		Menu = nullptr;
	}

	if (IsValid(Book))
	{
		Book->OnStateChanged.RemoveDynamic(this, &ThisClass::HandleBookStateChanged);

		if (bRequestedOpening)
		{
			Book->CancelOpening();
		}
	}

	Super::EndPlay(Reason);
}
