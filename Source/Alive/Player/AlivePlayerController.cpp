#include "AlivePlayerController.h"

#include "AlivePlayerState.h"
#include "GameModes/GameMode_Game.h"

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

void AAlivePlayerController::SpawnPlayerCharacterImmediately()
{
	check(HasAuthority());

	if (AAliveCharacter* OldCharacter = Cast<AAliveCharacter>(GetPawn()))
	{
		OldCharacter->FinishDeathImmediately();
	}

	if (AAlivePlayerState* PS = GetPlayerState<AAlivePlayerState>())
	{
		FTransform SpawnTransform = Cast<AGameMode_Game>(GetWorld()->GetAuthGameMode())->GetRandomSpawnTransform();
		FActorSpawnParameters Param;
		Param.Owner = this;
		APlayerCharacter* NewCharacter = GetWorld()->SpawnActor<APlayerCharacter>(PS->GetCharacterType(), SpawnTransform, Param);
		check(NewCharacter);
		Possess(NewCharacter);
	}
}

void AAlivePlayerController::RespawnPlayerCharacterAfterCooldown()
{
	if (HasAuthority())
	{
		AGameMode_Game* GM = Cast<AGameMode_Game>(GetWorld()->GetAuthGameMode());
		GetWorld()->GetTimerManager().SetTimer(
			RespawnCharacterTimerHandle, this, &AAlivePlayerController::SpawnPlayerCharacterImmediately,
			static_cast<float>(GM->GetPlayerRespawnCooldown()));
	}
}
