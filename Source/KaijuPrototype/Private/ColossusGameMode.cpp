// Fill out your copyright notice in the Description page of Project Settings.


#include "ColossusGameMode.h"
#include "ColossusFighterStart.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "ColossusGameInstance.h"


AColossusGameMode::AColossusGameMode()
{
	GameStateClass = AColossusGameState::StaticClass();
}

AActor* AColossusGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart called with null Player."));
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	const int32 PlayerSlot = GetOrAssignPlayerSlot(Player);

	const FColossusCompetitorDefinition* Competitor = FindHumanCompetitorByPlayerSlot(PlayerSlot);

	if (Competitor)
	{
		AColossusFighterStart* FighterStart = FindFighterStart(Competitor->SpawnIndex, Competitor->SpawnGroup);

		if (FighterStart)
		{
			UE_LOG(LogTemp, Log, TEXT("Using configured Fighter Start for competitor %s in player slot %d."), *Competitor->CompetitorId.ToString(), PlayerSlot);

			return FighterStart;
		}

		UE_LOG(LogTemp, Warning, TEXT("No Fighter Start found for competitor %s at index %d in group %s."),	*Competitor->CompetitorId.ToString(), Competitor->SpawnIndex, *Competitor->SpawnGroup.ToString());
	}

	// Preserve the existing player spawn if no competitor is configured.
	if (AColossusFighterStart* DefaultStart = FindFighterStart(0, FName(TEXT("Default"))))
	{
		return DefaultStart;
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void AColossusGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	const UColossusGameInstance* ColossusGameInstance = Cast<UColossusGameInstance>(GetGameInstance());
	
	if (!ColossusGameInstance || !ColossusGameInstance->HasPendingMatchSetup())
	{
		UE_LOG(LogTemp, Warning, TEXT("No pending match setup. Using GameMode defaults."));

		return;
	}

	MatchSetup = ColossusGameInstance->GetPendingMatchSetup();

	UE_LOG(LogTemp, Log, TEXT("InitGameState could not find ColossusGameState."));
}

void AColossusGameMode::InitGameState()
{
	Super::InitGameState();
	
	AColossusGameState* ColossusGameState = GetColossusGameState();

	if (!ColossusGameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitGameState: ColossusGameState is null"));
		return;
	}

	ColossusGameState->SetActiveMatchRules(MatchSetup.Rules);
	ColossusGameState->SetActiveCompetitors(MatchSetup.Competitors);

	UE_LOG(LogTemp, Log, TEXT("Published active match rules with %d configured competitors to ColossusGameState."), MatchSetup.Competitors.Num());
}

void AColossusGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	GetOrAssignPlayerSlot(NewPlayer);
}

void AColossusGameMode::Logout(AController* Exiting)
{
	AssignedPlayerSlots.Remove(Exiting);

	Super::Logout(Exiting);
}

UClass* AColossusGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (!InController)
	{
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	const int32 PlayerSlot = GetOrAssignPlayerSlot(InController);

	const FColossusCompetitorDefinition* Competitor = FindHumanCompetitorByPlayerSlot(PlayerSlot);

	if (Competitor && Competitor->FighterClass)
	{
		UE_LOG(LogTemp, Log, TEXT("Using configured fighter class %s for competitor %s in player slot %d."), *Competitor->FighterClass->GetName(), *Competitor->CompetitorId.ToString(), PlayerSlot);
		return Competitor->FighterClass.Get();
	}

	UE_LOG(LogTemp, Warning, TEXT("No configured fighter class for player slot %d. Using default pawn class."), PlayerSlot);
	
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

int32 AColossusGameMode::FindAvailablePlayerSlot() const
{
	TSet<int32> UsedSlots;
	for (const TPair<AController*, int32>& Assignment : AssignedPlayerSlots)
	{
		if (IsValid(Assignment.Key))
		{
			UsedSlots.Add(Assignment.Value);
		}
	}

	for (const FColossusCompetitorDefinition& Competitor : MatchSetup.Competitors)
	{
		if (Competitor.ControlType == EColossusControlType::Human && !UsedSlots.Contains(Competitor.PlayerSlot))
		{
			return Competitor.PlayerSlot;
		}
	}

	int32 FallbackSlot = 0;

	while (UsedSlots.Contains(FallbackSlot))
	{
		FallbackSlot++;
	}
	
	return FallbackSlot;
}

const FColossusCompetitorDefinition* AColossusGameMode::FindHumanCompetitorByPlayerSlot(int32 PlayerSlot) const
{
	return MatchSetup.Competitors.FindByPredicate([PlayerSlot](const FColossusCompetitorDefinition& Competitor)
	{
		return Competitor.ControlType == EColossusControlType::Human && Competitor.PlayerSlot == PlayerSlot;
	}
	);
}

int32 AColossusGameMode::GetOrAssignPlayerSlot(AController* Player)
{
	if (!Player) return INDEX_NONE;

	if (const int32* ExistingSlot = AssignedPlayerSlots.Find(Player))
	{
		return *ExistingSlot;
	}

	const int32 NewPlayerSlot = FindAvailablePlayerSlot();
	AssignedPlayerSlots.Add(Player, NewPlayerSlot);

	UE_LOG(LogTemp, Log, TEXT("Assigned player controller to match slot %d."), NewPlayerSlot);
	return NewPlayerSlot;
}

AColossusFighterStart* AColossusGameMode::FindFighterStart(int32 SpawnIndex, FName SpawnGroup) const
{
	if (!GetWorld()) return nullptr;

	for (TActorIterator<AColossusFighterStart> It(GetWorld()); It; ++It)
	{
		AColossusFighterStart* FighterStart = *It;
		if (FighterStart && FighterStart->bEnabled && FighterStart->SpawnIndex == SpawnIndex && FighterStart->SpawnGroup == SpawnGroup)
		{
			return FighterStart;
		}
	}

	return nullptr;
}

ABrawlerCharacter* AColossusGameMode::SpawnCPUFighterAtIndex(TSubclassOf<ABrawlerCharacter> FighterClass, int32 SpawnIndex, FName SpawnGroup)
{
	if (!FighterClass || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCPUFighterAtIndex: Invalid FighterClass or World"));
		return nullptr;
	}

	AColossusFighterStart* FighterStart = FindFighterStart(SpawnIndex, SpawnGroup);
	
	if (!FighterStart)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCPUFighterAtIndex: Invalid FighterStart"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ABrawlerCharacter* SpawnedFighter = GetWorld()->SpawnActor<ABrawlerCharacter>(FighterClass, FighterStart->GetActorTransform(), SpawnParameters);

	if (!SpawnedFighter)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCPUFighterAtIndex: Failed to spawn fighter"));
		return nullptr;
	}

	if (!SpawnedFighter->GetController())
	{
		SpawnedFighter->SpawnDefaultController();
	}

	return SpawnedFighter;
}

void AColossusGameMode::BeginNextRound()
{
	AColossusGameState* ColossusGameState = GetColossusGameState();

	if (!ColossusGameState) return;

	if (MatchSetup.Rules.MatchMode != EColossusMatchMode::Arcade)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginNextRound currently supports Arcade matches only"));
		return;
	}

	const int32 EncounterCount = RoundDefinitions.Num();
	if (EncounterCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginNextRound: No round definitions found"));
		return;
	}

	const int32 NextRound = ColossusGameState->GetCurrentRound() + 1;
	if (NextRound > EncounterCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginNextRound: No more rounds available"));
		return;
	}

	ColossusGameState->SetRoundState(NextRound, ColossusGameState->GetTotalRounds(), EColossusRoundPhase::Intro);
}

void AColossusGameMode::ActivateCurrentRound()
{
	AColossusGameState* ColossusGameState = GetColossusGameState();
	if (!ColossusGameState)
	{
		return;
	}

	ColossusGameState->SetRoundState(ColossusGameState->GetCurrentRound(), ColossusGameState->GetTotalRounds(), EColossusRoundPhase::Active);
}

bool AColossusGameMode::BeginBetweenRounds()
{
	AColossusGameState* ColossusGameState = GetColossusGameState();
	if (!ColossusGameState)
	{
		return false;
	}
	if (ColossusGameState->GetCurrentRound() >= ColossusGameState->GetTotalRounds())
	{
		return false;
	}

	ColossusGameState->SetRoundState(ColossusGameState->GetCurrentRound(), ColossusGameState->GetTotalRounds(), EColossusRoundPhase::BetweenRounds);
	
	return true;
}

void AColossusGameMode::CompleteMatch()
{
	AColossusGameState* ColossusGameState = GetColossusGameState();
	if (!ColossusGameState)
	{
		return;
	}

	ColossusGameState->SetRoundState(ColossusGameState->GetCurrentRound(), ColossusGameState->GetTotalRounds(), EColossusRoundPhase::Complete);
}

AColossusGameState* AColossusGameMode::GetColossusGameState() const
{
	return Cast<AColossusGameState>(GameState);
}

bool AColossusGameMode::GetCurrentRoundDefinition(FColossusRoundDefinition& OutRoundDefinition) const
{
	const AColossusGameState* ColossusGameState = GetColossusGameState();

	if (!ColossusGameState)
	{
		return false;
	}
	
	const int32 DefinitionIndex = ColossusGameState->GetCurrentRound() - 1;

	if (!RoundDefinitions.IsValidIndex(DefinitionIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetCurrentRoundDefinition: Invalid round index %d"), ColossusGameState->GetCurrentRound());
		return false;
	}

	OutRoundDefinition = RoundDefinitions[DefinitionIndex];

	if (!OutRoundDefinition.FighterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetCurrentRoundDefinition: Invalid FighterClass in round definition %d"), ColossusGameState->GetCurrentRound());
		return false;
	}

	return true;
}