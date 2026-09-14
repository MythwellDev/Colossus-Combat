#pragma once

#include "CoreMinimal.h"
#include "ColossusMatchTypes.generated.h"

class ABrawlerCharacter;

USTRUCT(BlueprintType)
struct FColossusRoundDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Round")
	TSubclassOf<ABrawlerCharacter> FighterClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Round",	meta = (ClampMin = "0", UIMin = "0"))
	int32 SpawnIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Round")
	FName SpawnGroup = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Round",	meta = (ClampMin = "1.0", UIMin = "1.0"))
	float StartingHealth = 100.0f;
};

UENUM(BlueprintType)
enum class EColossusMatchMode : uint8
{
	Arcade,
	Versus,
	Training
};

UENUM(BlueprintType)
enum class EColossusWinCondition : uint8
{
	RoundWins,
	LastFighterStanding,
	MostEliminations,
	EliminationTarget,
	CompleteEncounterSequence
};

USTRUCT(BlueprintType)
struct FColossusMatchRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Rules")
	EColossusMatchMode MatchMode = EColossusMatchMode::Arcade;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Rules")
	EColossusWinCondition WinCondition = EColossusWinCondition::RoundWins;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Rules",
		meta = (ClampMin = "0"))
	int32 TimeLimitSeconds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Rules",
		meta = (ClampMin = "0"))
	int32 StartingStocks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Rules",
		meta = (ClampMin = "0"))
	int32 EliminationTarget = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Rules", meta = (ClampMin = "1", UIMin = "1", EditCondition = "WinCondition == EColossusWinCondition::RoundWins", EditConditionHides))
	int32 RoundsToWin = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colossus|Rules")
	bool bTeamsEnabled = false;
};