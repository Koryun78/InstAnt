#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Instant/Menu/Core/InstantMenuTypes.h"
#include "InstantMenuPlayerController.generated.h"

class AInstantMenuBookActor;
class UInstantMainMenuWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInstantMenuObservedBookStateChanged,EInstantBookState, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInstantMenuVineInteractionChanged,EInstantMenuSide, Side,bool, bHolding,FVector2D, ViewportPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInstantMenuActionRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInstantMenuLoadRequested,const FString&, SlotName,int32, UserIndex);

UCLASS(Blueprintable)
class INSTANT_API AInstantMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
		AInstantMenuPlayerController();
	
		virtual void Tick(float DeltaSeconds) override;
	
		UPROPERTY(EditDefaultsOnly, Category = "Menu")
		TSubclassOf<UInstantMainMenuWidget> MenuWidgetClass;
	
		// Карта новой игры. Обработчик события имеет приоритет.
		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu")
		TSoftObjectPtr<UWorld> NewGameMap;
	
		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu|Vines",meta = (ClampMin = "0.01"))
		float VineHoldSeconds = 1.f;
	
		// Включить после подключения визуала и зон лоз.
		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu|Vines")
		bool bHoldSystemCursor = false;
	
		UPROPERTY(BlueprintAssignable, Category = "Menu")
		FInstantMenuObservedBookStateChanged OnBookStateChanged;
	
		UPROPERTY(BlueprintAssignable, Category = "Menu|Vines")
		FInstantMenuVineInteractionChanged OnVineInteractionChanged;
	
		UPROPERTY(BlueprintAssignable, Category = "Menu|Actions")
		FInstantMenuActionRequested OnContinueRequested;
	
		UPROPERTY(BlueprintAssignable, Category = "Menu|Actions")
		FInstantMenuActionRequested OnNewGameRequested;
	
		UPROPERTY(BlueprintAssignable, Category = "Menu|Actions")
		FInstantMenuLoadRequested OnLoadGameRequested;
	
		UFUNCTION(BlueprintCallable, Category = "Menu")
		void SetMenuBook(AInstantMenuBookActor* InBook);
	
		UFUNCTION(BlueprintPure, Category = "Menu")
		AInstantMenuBookActor* GetMenuBook() const { return Book; }
	
		UFUNCTION(BlueprintPure, Category = "Menu")
		UInstantMainMenuWidget* GetMenuWidget() const { return Menu; }
	
		UFUNCTION(BlueprintPure, Category = "Menu")
		EInstantBookState GetBookState() const;
	
		UFUNCTION(BlueprintCallable, Category = "Menu")
		bool RequestStart();
	
		UFUNCTION(BlueprintPure, Category = "Menu")
		bool CanInteractWithMenu() const;
	
		UFUNCTION(BlueprintPure, Category = "Menu|Vines")
		bool IsVineHolding() const { return VineState.IsHolding(); }
	
		// Позиция передаётся в пикселях viewport.
		UFUNCTION(BlueprintCallable, Category = "Menu|Vines")
		bool UpdateVineProximity(EInstantMenuSide Side,bool bInside,FVector2D ViewportPosition);
	
		UFUNCTION(BlueprintCallable, Category = "Menu|Vines")
		void CancelVineInteraction();
	
		UFUNCTION(BlueprintCallable, Category = "Menu|Actions")
		void SetContinueAvailable(bool bAvailable);
	
		UFUNCTION(BlueprintPure, Category = "Menu|Actions")
		bool CanContinueGame() const;
	
		UFUNCTION(BlueprintPure, Category = "Menu|Actions")
		bool CanStartNewGame() const;
	
		UFUNCTION(BlueprintPure, Category = "Menu|Actions")
		bool CanLoadGame() const;
	
		UFUNCTION(BlueprintCallable, Category = "Menu|Actions")
		bool RequestContinue();
	
		UFUNCTION(BlueprintCallable, Category = "Menu|Actions")
		bool RequestNewGame();
	
		UFUNCTION(BlueprintCallable, Category = "Menu|Actions")
		bool RequestLoadGame(const FString& SlotName, int32 UserIndex);
	
		UFUNCTION(BlueprintCallable, Category = "Menu|Actions")
		void RequestQuit();
	
	protected:
		virtual void BeginPlay() override;
		virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	
	private:
		UPROPERTY(Transient)
		TObjectPtr<UInstantMainMenuWidget> Menu;
	
		UPROPERTY(Transient)
		TObjectPtr<AInstantMenuBookActor> Book;
	
		EInstantBookState LastBookState = EInstantBookState::Closed;
	
		bool bEndingPlay = false;
		bool bContinueAvailable = false;
		bool bRequestedOpening = false;
	
		FInstantMenuVineState VineState;
	
		UFUNCTION()
		void HandleBookStateChanged(EInstantBookState State);
	
		bool HasMenuViewportFocus() const;
	
		void NotifyVineReleased(EInstantMenuSide Side,FVector2D Position);
};















