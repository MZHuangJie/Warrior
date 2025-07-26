// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/StarUpData/DataAsset_StarUpDataBase.h"
#include "DataAsset_EnemyStarUpDataBase.generated.h"

class UWarriorEnemyGameplayAbility;
/**
 * 
 */
UCLASS()
class WARRIOR_API UDataAsset_EnemyStarUpDataBase : public UDataAsset_StarUpDataBase
{
	GENERATED_BODY()

public:
	virtual void GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel = 1) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "StarUpData")
	TArray<TSubclassOf<UWarriorEnemyGameplayAbility>> EnemyCombatAbilities;
};
