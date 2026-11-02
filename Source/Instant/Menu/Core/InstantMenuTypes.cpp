#include "InstantMenuTypes.h"

bool FInstantMenuVineState::UpdateProximity(EInstantMenuSide Side, bool bInside, const FVector2D& Position, float Duration)
{
	if (Side != EInstantMenuSide::Left && Side != EInstantMenuSide::Right)
	{
		return false;
	}

	bool& bWasInside = Side == EInstantMenuSide::Left ? bLeftInside : bRightInside;

	const bool bEntered = bInside && !bWasInside;
	bWasInside = bInside;

	if (!bEntered || IsHolding() || !FMath::IsFinite(Duration) || !FMath::IsFinite(Position.X) || !FMath::IsFinite(Position.Y))
	{
		return false;
	}

	ActiveSide = Side;
	HeldPosition = Position;
	RemainingSeconds = FMath::Max(Duration, 0.01f);

	return true;
}

bool FInstantMenuVineState::Release()
{
	const bool bWasHolding = IsHolding();

	ActiveSide = EInstantMenuSide::None;
	RemainingSeconds = 0.f;

	return bWasHolding;
}

bool FInstantMenuVineState::Advance(float DeltaSeconds)
{
	if (!IsHolding() || !FMath::IsFinite(DeltaSeconds))
	{
		return false;
	}

	RemainingSeconds -= FMath::Max(DeltaSeconds, 0.f);

	return RemainingSeconds <= 0.f && Release();
}

void FInstantMenuVineState::Reset()
{
	Release();

	HeldPosition = FVector2D::ZeroVector;
	bLeftInside = false;
	bRightInside = false;
}

