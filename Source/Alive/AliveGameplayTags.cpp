// Copyright Epic Games, Inc. All Rights Reserved.

#include "AliveGameplayTags.h"
#include "AliveLogChannels.h"
#include "GameplayTagsManager.h"

FAliveGameplayTags FAliveGameplayTags::GameplayTags;

void FAliveGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	GameplayTags.AddAllTags(Manager);
	// Notify manager that we are done adding native tags.
	Manager.DoneAddingNativeTags();
}

FGameplayTag FAliveGameplayTags::FindTagByString(FString TagString, bool bMatchPartialString)
{
	const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

	if (!Tag.IsValid() && bMatchPartialString)
	{
		FGameplayTagContainer AllTags;
		Manager.RequestAllGameplayTags(AllTags, true);

		for (const FGameplayTag TestTag : AllTags)
		{
			if (TestTag.ToString().Contains(TagString))
			{
				UE_LOG(LogAlive, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
				Tag = TestTag;
				break;
			}
		}
	}
	return Tag;
}

void FAliveGameplayTags::AddAllTags(UGameplayTagsManager& Manager)
{
	//AddTag(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	// AddTag(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	// AddTag(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	// AddTag(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	// AddTag(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	// AddTag(Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	// AddTag(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

}

void FAliveGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName), FString(TEXT("(Native) ")) + FString(TagComment));
}
