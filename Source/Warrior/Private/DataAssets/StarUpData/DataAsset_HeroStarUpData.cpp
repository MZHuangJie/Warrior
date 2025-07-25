// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/StarUpData/DataAsset_HeroStarUpData.h"
#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"

void UDataAsset_HeroStarUpData::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
    Super::GiveToAbilitySystemComponent(InASCToGive, ApplyLevel);

    for (const FWarriorHeroAbilitySet& Abilityset : HeroStartUpAbilitySets)
    {
        if (!Abilityset.IsVaild()) continue;

        FGameplayAbilitySpec AbilitySpec(Abilityset.AbilityToGrant);
        AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
        AbilitySpec.Level = ApplyLevel;
        AbilitySpec.GetDynamicSpecSourceTags().AddTag(Abilityset.InputTag);
        InASCToGive->GiveAbility(AbilitySpec);

    }
}
