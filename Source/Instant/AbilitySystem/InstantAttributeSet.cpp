// Fill out your copyright notice in the Description page of Project Settings.


#include "InstantAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"

void UInstantAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UInstantAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInstantAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UInstantAttributeSet::OnRep_Health(
    const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UInstantAttributeSet, Health, OldValue);
}

void UInstantAttributeSet::OnRep_MaxHealth(
    const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UInstantAttributeSet, MaxHealth, OldValue);
}