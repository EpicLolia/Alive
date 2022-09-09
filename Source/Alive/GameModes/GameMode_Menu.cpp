// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode_Menu.h"

AGameMode_Menu::AGameMode_Menu()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGameMode_Menu::RestartPlayer(AController* NewPlayer)
{
	// don't restart
}

void AGameMode_Menu::BeginPlay()
{
	Super::BeginPlay();
	
}
