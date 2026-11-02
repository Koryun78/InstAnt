#include "InstantMainMenuWidget.h"

#include "Instant/Menu/Core/InstantMenuPlayerController.h"

#include "Instant/Menu/Widgets/InstantMenuPageWidget.h"
#include "Instant/Menu/Widgets/InstantStartPageWidget.h"

#include "Components/WidgetSwitcher.h"
#include "InputCoreTypes.h"

UInstantMainMenuWidget::UInstantMainMenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsFocusable(true);

	PageClasses.Add(EInstantMenuPage::Start, UInstantStartPageWidget::StaticClass());
	PageClasses.Add(EInstantMenuPage::Main, UInstantMenuPageWidget::StaticClass());
	PageClasses.Add(EInstantMenuPage::Settings, UInstantMenuPageWidget::StaticClass());
	PageClasses.Add(EInstantMenuPage::LoadGame, UInstantMenuPageWidget::StaticClass());
}

void UInstantMainMenuWidget::InitializeMenu(AInstantMenuPlayerController* InController)
{
	ShutdownMenu();

	if (!IsValid(InController))
	{
		return;
	}

	MenuController = InController;

	InController->OnBookStateChanged.AddUniqueDynamic(this, &ThisClass::HandleBookStateChanged);

	InController->OnVineInteractionChanged.AddUniqueDynamic(this, &ThisClass::HandleVineInteraction);

	for (const auto& Entry : PageClasses)
	{
		if (!Entry.Value)
		{
			continue;
		}

		UInstantMenuPageWidget* Page = CreateWidget<UInstantMenuPageWidget>(InController, Entry.Value);

		if (!Page)
		{
			continue;
		}

		Page->InitializePage(this);
		Page->SetVisibility(ESlateVisibility::Collapsed);

		Pages.Add(Entry.Key, Page);

		if (PageSwitcher)
		{
			PageSwitcher->AddChild(Page);
		}
	}

	HandleBookStateChanged(InController->GetBookState());
}

void UInstantMainMenuWidget::ShutdownMenu()
{
	if (MenuController.IsValid())
	{
		MenuController->OnBookStateChanged.RemoveDynamic(this, &ThisClass::HandleBookStateChanged);
		MenuController->OnVineInteractionChanged.RemoveDynamic(this, &ThisClass::HandleVineInteraction);
		MenuController->CancelVineInteraction();
	}

	if (ActivePage)
	{
		ActivePage->HidePage();
	}

	ActivePage = nullptr;

	for (const auto& Entry : Pages)
	{
		if (Entry.Value)
		{
			Entry.Value->InitializePage(nullptr);
			Entry.Value->RemoveFromParent();
		}
	}

	Pages.Empty();
	MenuController.Reset();

	CurrentPage = EInstantMenuPage::Start;
}

void UInstantMainMenuWidget::NativeDestruct()
{
	ShutdownMenu();

	Super::NativeDestruct();
}

AInstantMenuPlayerController* UInstantMainMenuWidget::GetMenuController() const
{
	return MenuController.Get();
}

UInstantMenuPageWidget* UInstantMainMenuWidget::GetPage(EInstantMenuPage Page) const
{
	const TObjectPtr<UInstantMenuPageWidget>* Found = Pages.Find(Page);

	return Found ? Found->Get() : nullptr;
}

bool UInstantMainMenuWidget::CanInteract() const
{
	return MenuController.IsValid() && MenuController->CanInteractWithMenu();
}

bool UInstantMainMenuWidget::ActivatePage(EInstantMenuPage Page)
{
	UInstantMenuPageWidget* Next = GetPage(Page);

	if (!Next)
	{
		return false;
	}

	if (ActivePage == Next)
	{
		RefreshInteractionState();
		return true;
	}

	if (MenuController.IsValid())
	{
		MenuController->CancelVineInteraction();
	}

	if (ActivePage)
	{
		ActivePage->HidePage();
	}

	CurrentPage = Page;
	ActivePage = Next;

	if (PageSwitcher)
	{
		PageSwitcher->SetActiveWidget(Next);
	}

	RefreshInteractionState();

	Next->ShowPage();
	OnPageChanged.Broadcast(Page);

	return true;
}

bool UInstantMainMenuWidget::ShowPage(
	EInstantMenuPage Page)
{
	return Page != EInstantMenuPage::Start && CanInteract() && ActivatePage(Page);
}

void UInstantMainMenuWidget::HandleBookStateChanged(
	EInstantBookState State)
{
	const EInstantMenuPage TargetPage = State == EInstantBookState::Open ? EInstantMenuPage::Main : EInstantMenuPage::Start;

	ActivatePage(TargetPage);
	RefreshInteractionState();

	OnBookStateUpdated(State);
}

void UInstantMainMenuWidget::HandleVineInteraction(EInstantMenuSide Side, bool bHolding, FVector2D Position)
{
	RefreshInteractionState();

	OnVineInteractionUpdated(Side, bHolding, Position);
}

void UInstantMainMenuWidget::RefreshInteractionState()
{
	if (ActivePage && MenuController.IsValid())
	{
		const bool bStartEnabled = CurrentPage == EInstantMenuPage::Start && MenuController->GetBookState() == EInstantBookState::Closed && !MenuController->IsVineHolding();

		ActivePage->SetIsEnabled(bStartEnabled || CanInteract());
	}

	OnActionAvailabilityChanged();
}

bool UInstantMainMenuWidget::BeginOpening()
{
	return CurrentPage == EInstantMenuPage::Start && MenuController.IsValid() && MenuController->RequestStart();
}

FReply UInstantMainMenuWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (MenuController.IsValid())
	{
		const bool bOpening = MenuController->GetBookState() == EInstantBookState::Opening;

		if (MenuController->IsVineHolding() || bOpening)
		{
			return FReply::Handled();
		}

		if (CurrentPage == EInstantMenuPage::Start &&
			BeginOpening())
		{
			return FReply::Handled();
		}

		if (Event.GetKey() == EKeys::Escape &&
			CurrentPage != EInstantMenuPage::Main)
		{
			Back();
			return FReply::Handled();
		}
	}

	return Super::NativeOnPreviewKeyDown(Geometry, Event);
}

FReply UInstantMainMenuWidget::NativeOnPreviewMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event)
{
	if (MenuController.IsValid())
	{
		const bool bOpening = MenuController->GetBookState() == EInstantBookState::Opening;

		if (MenuController->IsVineHolding() || bOpening)
		{
			return FReply::Handled();
		}

		if (CurrentPage == EInstantMenuPage::Start &&
			BeginOpening())
		{
			return FReply::Handled();
		}
	}

	return Super::NativeOnPreviewMouseButtonDown(Geometry, Event);
}

bool UInstantMainMenuWidget::ContinueGame()
{
	return MenuController.IsValid() && MenuController->RequestContinue();
}

bool UInstantMainMenuWidget::NewGame()
{
	return MenuController.IsValid() && MenuController->RequestNewGame();
}

bool UInstantMainMenuWidget::RequestLoadGame(const FString& SlotName, int32 UserIndex)
{
	return MenuController.IsValid() && MenuController->RequestLoadGame(SlotName, UserIndex);
}

void UInstantMainMenuWidget::OpenSettings()
{
	ShowPage(EInstantMenuPage::Settings);
}

void UInstantMainMenuWidget::OpenLoadGame()
{
	ShowPage(EInstantMenuPage::LoadGame);
}

void UInstantMainMenuWidget::Back()
{
	ShowPage(EInstantMenuPage::Main);
}

void UInstantMainMenuWidget::Quit()
{
	if (MenuController.IsValid())
	{
		MenuController->RequestQuit();
	}
}