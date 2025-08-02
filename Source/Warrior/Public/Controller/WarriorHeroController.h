// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "WarriorHeroController.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorHeroController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	AWarriorHeroController();

	//~ Begin IAbilitySystemInterface Interface.
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~ End IAbilitySystemInterface Interface.

private:
	FGenericTeamId HeroTeamId;
};
