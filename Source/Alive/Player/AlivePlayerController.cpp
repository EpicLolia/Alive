#include "AlivePlayerController.h"

AAlivePlayerController::AAlivePlayerController()
{
}

void AAlivePlayerController::ClientRepDamageResultAsSource_Implementation(FDamageResult DamageResult)
{
	K2_OnCauseDamage(DamageResult);
}

void AAlivePlayerController::ClientRepDamageResultAsTarget_Implementation(FDamageResult DamageResult)
{
	K2_OnSufferDamage(DamageResult);
}
