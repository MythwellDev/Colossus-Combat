// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ColossusMatchTypes.h"
#include "ColossusGameInstance.generated.h"

UCLASS()
class KAIJUPROTOTYPE_API UColossusGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Colossus|Match")
	void StartPendingMatchSetup(const FColossusMatchSetup& NewMatchSetup);

	UFUNCTION(BlueprintCallable, Category = "Colossus|Match")
	FColossusMatchSetup GetPendingMatchSetup() const;

	UFUNCTION(BlueprintCallable, Category = "Colossus|Match")
	bool HasPendingMatchSetup() const;

	UFUNCTION(BlueprintCallable, Category = "Colossus|Match")
	void ClearPendingMatchSetup();

private:
	UPROPERTY(Transient)
	FColossusMatchSetup PendingMatchSetup;

	UPROPERTY(Transient)
	bool bHasPendingMatchSetup = false;

};
