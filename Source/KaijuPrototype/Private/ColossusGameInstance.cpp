// Fill out your copyright notice in the Description page of Project Settings.


#include "ColossusGameInstance.h"

void UColossusGameInstance::StartPendingMatchSetup(const FColossusMatchSetup& NewMatchSetup)
{
	PendingMatchSetup = NewMatchSetup;
	bHasPendingMatchSetup = true;
}

FColossusMatchSetup UColossusGameInstance::GetPendingMatchSetup() const
{
	return PendingMatchSetup;
}

bool UColossusGameInstance::HasPendingMatchSetup() const
{
	return bHasPendingMatchSetup;
}

void UColossusGameInstance::ClearPendingMatchSetup()
{
	PendingMatchSetup = FColossusMatchSetup();
	bHasPendingMatchSetup = false;
}
