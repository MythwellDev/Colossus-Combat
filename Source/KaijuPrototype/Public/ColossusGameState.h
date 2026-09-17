// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ColossusMatchTypes.h"
#include "ColossusGameState.generated.h"

UENUM(BlueprintType)
enum class EColossusRoundPhase : uint8
{
	Waiting UMETA(DisplayName = "Waiting"),
	Intro UMETA(DisplayName = "Round Intro"),
	Active UMETA(DisplayName = "Active"),
	BetweenRounds UMETA(DisplayName = "Between Rounds"),
	Complete UMETA(DisplayName = "Match Complete")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FColossusRoundStateChangedSignature, int32, CurrentRound, int32, TotalRounds, EColossusRoundPhase, RoundPhase);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FColossusMatchRulesChangedSignature, FColossusMatchRules, MatchRules);

UCLASS()
class KAIJUPROTOTYPE_API AColossusGameState : public AGameState
{
	GENERATED_BODY()
	
public: 
	AColossusGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetRoundState(int32 NewRound, int32 NewTotalRounds, EColossusRoundPhase NewRoundPhase);

	void SetActiveMatchRules(const FColossusMatchRules& NewMatchRules);

	void SetActiveCompetitors(const TArray<FColossusCompetitorDefinition>& NewCompetitors);

	UFUNCTION(BlueprintPure, Category = "Colossus|Match")
	TArray<FColossusCompetitorDefinition> GetActiveCompetitors() const { return ActiveCompetitors; }

	UFUNCTION(BlueprintPure, Category = "Colossus|Match")
	int32 GetCurrentRound() const { return CurrentRound; }

	UFUNCTION(BlueprintPure, Category = "Colossus|Match")
	int32 GetTotalRounds() const { return TotalRounds; }

	UFUNCTION(BlueprintPure, Category = "Colossus|Match")
	EColossusRoundPhase GetRoundPhase() const { return RoundPhase; }

	UFUNCTION(BlueprintPure, Category = "Colossus|Match")
	FColossusMatchRules GetActiveMatchRules() const { return ActiveMatchRules; }

	UPROPERTY(BlueprintAssignable, Category = "Colossus|Match|Events")
	FColossusRoundStateChangedSignature OnRoundStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Colossus|Match|Events")
	FColossusMatchRulesChangedSignature OnMatchRulesChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_RoundState, VisibleAnywhere, Category = "Colossus|Match")
	int32 CurrentRound = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_RoundState, EditDefaultsOnly, Category = "Colossus|Match")
	int32 TotalRounds = 2;
	
	UPROPERTY(ReplicatedUsing = OnRep_RoundState, VisibleAnywhere, Category = "Colossus|Match")
	EColossusRoundPhase RoundPhase;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveMatchRules, EditDefaultsOnly, Category = "Colossus|Match")
	FColossusMatchRules ActiveMatchRules;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Colossus|Match")
	TArray<FColossusCompetitorDefinition> ActiveCompetitors;
	
	UFUNCTION()
	void OnRep_RoundState();

	UFUNCTION()
	void OnRep_ActiveMatchRules();

private:
	void BroadcastRoundState();
};
