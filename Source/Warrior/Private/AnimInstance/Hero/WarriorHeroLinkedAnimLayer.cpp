// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstance/Hero/WarriorHeroLinkedAnimLayer.h"
#include "AnimInstance/Hero/WarriorHeroAnimInstance.h"

UWarriorHeroAnimInstance* UWarriorHeroLinkedAnimLayer::GetHeroAnimInstance() const
{
    return Cast<UWarriorHeroAnimInstance>(GetOwningComponent()->GetAnimInstance());
}
