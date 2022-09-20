// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UGameplayTagsManager;

/**
 * FAliveGameplayTags
 *
 * Singleton containing native gameplay tags.
 */
struct FAliveGameplayTags
{
	static FGameplayTag FindTagByString(FString TagString, bool bMatchPartialString = false);
	static void InitializeNativeTags();
	static const FAliveGameplayTags& Get(){return GameplayTags;}

public:
	// FGameplayTag Ability_ActivateFail_IsDead;
	// FGameplayTag Ability_ActivateFail_Cooldown;
	// FGameplayTag Ability_ActivateFail_Cost;
	// FGameplayTag Ability_ActivateFail_TagsBlocked;
	// FGameplayTag Ability_ActivateFail_TagsMissing;
	// FGameplayTag Ability_ActivateFail_Networking;
	// FGameplayTag Ability_ActivateFail_ActivationGroup;

private:
	void AddAllTags(UGameplayTagsManager& Manager);
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);
	
	FAliveGameplayTags(){return;}
	static FAliveGameplayTags GameplayTags;
};
