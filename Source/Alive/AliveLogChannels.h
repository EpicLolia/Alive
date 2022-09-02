#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"


ALIVE_API DECLARE_LOG_CATEGORY_EXTERN(LogAlive, Log, All);
ALIVE_API DECLARE_LOG_CATEGORY_EXTERN(LogAliveAbilitySystem, Log, All);
ALIVE_API DECLARE_LOG_CATEGORY_EXTERN(LogAliveWeapon, Log, All);

ALIVE_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
