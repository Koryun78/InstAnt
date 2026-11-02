#pragma once

#include "CoreMinimal.h"
#include "Instant/Menu/Widgets/InstantMenuPageWidget.h"
#include "InstantStartPageWidget.generated.h"

UCLASS(Blueprintable)
class INSTANT_API UInstantStartPageWidget : public UInstantMenuPageWidget
{
	GENERATED_BODY()

public:
	UInstantStartPageWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu")
	FText ContinuePrompt;

	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool RequestStart();
};