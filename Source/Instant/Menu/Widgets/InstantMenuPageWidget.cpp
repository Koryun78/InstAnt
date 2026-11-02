#include "InstantMenuPageWidget.h"

#include "Instant/Menu/Widgets/InstantMainMenuWidget.h"

void UInstantMenuPageWidget::InitializePage(UInstantMainMenuWidget* InMenu)
{
	Menu = InMenu;
}

UInstantMainMenuWidget* UInstantMenuPageWidget::GetMenu() const
{
	return Menu.Get();
}

bool UInstantMenuPageWidget::CanInteract() const
{
	return Menu.IsValid() &&
		Menu->GetActivePage() == this &&
		Menu->CanInteract();
}

void UInstantMenuPageWidget::RequestBack()
{
	if (CanInteract())
	{
		Menu->Back();
	}
}

void UInstantMenuPageWidget::ShowPage()
{
	SetVisibility(ESlateVisibility::Visible);

	HandlePageShown();
	OnPageShown();
}

void UInstantMenuPageWidget::HidePage()
{
	SetVisibility(ESlateVisibility::Collapsed);

	OnPageHidden();
}

void UInstantMenuPageWidget::HandlePageShown()
{
}