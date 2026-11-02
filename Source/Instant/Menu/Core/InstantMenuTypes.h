#pragma once

#include "CoreMinimal.h"
#include "InstantMenuTypes.generated.h"

UENUM(BlueprintType)
enum class EInstantMenuPage : uint8
{
	Start,
	Main,
	Settings,
	LoadGame
};

UENUM(BlueprintType)
enum class EInstantBookState : uint8
{
	Closed,
	Opening,
	Open
};

UENUM(BlueprintType)
enum class EInstantMenuSide : uint8
{
	None,
	Left,
	Right
};

// Данные для будущей страницы загрузки.
USTRUCT(BlueprintType)
struct INSTANT_API FInstantMenuSaveSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Saves")
	FString SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Saves", meta = (ClampMin = "0"))
	int32 UserIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Saves")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Saves")
	bool bCanLoad = false;
};

// Независимая от визуала механика лоз.
// Повторный захват требует выхода из зоны и нового входа.
struct INSTANT_API FInstantMenuVineState
{
public:
	bool IsHolding() const
	{
		return ActiveSide != EInstantMenuSide::None;
	}

	EInstantMenuSide GetSide() const
	{
		return ActiveSide;
	}

	FVector2D GetPosition() const
	{
		return HeldPosition;
	}

	bool Release();
	void Reset();
	bool UpdateProximity(EInstantMenuSide Side, bool bInside, const FVector2D& Position, float Duration);
	bool Advance(float DeltaSeconds);

private:
	EInstantMenuSide ActiveSide = EInstantMenuSide::None;
	FVector2D HeldPosition = FVector2D::ZeroVector;

	float RemainingSeconds = 0.f;

	bool bLeftInside = false;
	bool bRightInside = false;
};