#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Instant/Menu/Core/InstantMenuTypes.h"
#include "InstantMainMenuWidget.generated.h"

class UInstantMenuPageWidget;
class UWidgetSwitcher;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInstantMenuPageChanged, EInstantMenuPage, Page);

UCLASS(Blueprintable)
class INSTANT_API UInstantMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInstantMainMenuWidget(const FObjectInitializer& ObjectInitializer);

	void InitializeMenu(class AInstantMenuPlayerController* InController);

	void ShutdownMenu();

	UPROPERTY(EditDefaultsOnly, Category = "Menu")
	TMap<EInstantMenuPage, TSubclassOf<UInstantMenuPageWidget>> PageClasses;

	UPROPERTY(BlueprintAssignable, Category = "Menu")
	FInstantMenuPageChanged OnPageChanged;

	UFUNCTION(BlueprintPure, Category = "Menu")
	class AInstantMenuPlayerController* GetMenuController() const;

	UFUNCTION(BlueprintPure, Category = "Menu")
	UInstantMenuPageWidget* GetActivePage() const
	{
		return ActivePage;
	}

	UFUNCTION(BlueprintPure, Category = "Menu")
	UInstantMenuPageWidget* GetPage(EInstantMenuPage Page) const;

	UFUNCTION(BlueprintPure, Category = "Menu")
	EInstantMenuPage GetCurrentPage() const
	{
		return CurrentPage;
	}

	UFUNCTION(BlueprintPure, Category = "Menu")
	bool CanInteract() const;

	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool ShowPage(EInstantMenuPage Page);

	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool BeginOpening();

	UFUNCTION(BlueprintCallable, Category = "Menu|Actions")
	bool ContinueGame();

	UFUNCTION(BlueprintCallable, Category = "Menu|Actions")
	bool NewGame();

	UFUNCTION(BlueprintCallable, Category = "Menu|Actions")
	bool RequestLoadGame(const FString& SlotName, int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void OpenSettings();

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void OpenLoadGame();

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void Back();

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void Quit();

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void RefreshInteractionState();

protected:
	// Позже добавить WidgetSwitcher с этим именем в UMG.
	// Без него логика существует, но страницы не отображаются.
	UPROPERTY(BlueprintReadOnly, Category = "Menu", meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> PageSwitcher;

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
	void OnBookStateUpdated(EInstantBookState State);

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
	void OnActionAvailabilityChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu|Vines")
	void OnVineInteractionUpdated(EInstantMenuSide Side, bool bHolding, FVector2D ViewportPosition);

	virtual void NativeDestruct() override;

	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;

	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<class AInstantMenuPlayerController> MenuController;

	UPROPERTY(Transient)
	TMap<EInstantMenuPage, TObjectPtr<UInstantMenuPageWidget>> Pages;

	UPROPERTY(Transient)
	TObjectPtr<UInstantMenuPageWidget> ActivePage;

	EInstantMenuPage CurrentPage = EInstantMenuPage::Start;

	UFUNCTION()
	void HandleBookStateChanged(EInstantBookState State);

	UFUNCTION()
	void HandleVineInteraction(EInstantMenuSide Side,bool bHolding, FVector2D Position);

	bool ActivatePage(EInstantMenuPage Page);
};