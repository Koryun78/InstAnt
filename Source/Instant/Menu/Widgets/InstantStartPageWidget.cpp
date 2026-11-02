#include "InstantStartPageWidget.h"
#include "Instant/Menu/Widgets/InstantMainMenuWidget.h"

UInstantStartPageWidget::UInstantStartPageWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ContinuePrompt = NSLOCTEXT("InstantMenu", "ContinuePrompt", "Press any key to continue");
}

bool UInstantStartPageWidget::RequestStart()
{
	UInstantMainMenuWidget* Root = GetMenu();

	return Root && Root->GetActivePage() == this && Root->BeginOpening();
	return false;
}