#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InstantMenuPageWidget.generated.h"

class UInstantMainMenuWidget;

UCLASS(Blueprintable)
class INSTANT_API UInstantMenuPageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializePage(UInstantMainMenuWidget* InMenu);

	void ShowPage();
	void HidePage();

	UFUNCTION(BlueprintPure, Category = "Menu")
	UInstantMainMenuWidget* GetMenu() const;

	UFUNCTION(BlueprintPure, Category = "Menu")
	bool CanInteract() const;

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void RequestBack();

protected:
	virtual void HandlePageShown();

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
	void OnPageShown();

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
	void OnPageHidden();

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UInstantMainMenuWidget> Menu;
};