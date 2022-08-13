// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AliveHUD.generated.h"

UCLASS()
class AAliveHUD : public AHUD
{
	GENERATED_BODY()

public:
	AAliveHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;
};

