#include "InstantMenuGameMode.h"
#include "Instant/Menu/Core/InstantMenuPlayerController.h"

AInstantMenuGameMode::AInstantMenuGameMode()
{
	PlayerControllerClass = AInstantMenuPlayerController::StaticClass();

	DefaultPawnClass = nullptr;
}