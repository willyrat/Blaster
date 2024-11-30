// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MatchTypesEnum.h"
#include "MultiplayerSessionsSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	GameState.Get()->PlayerArray.Num();
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance )
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);	//execution will hault if not valid

		if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;
				EMatchTypes MatchType = Subsystem->DesiredMatchType;
				if (MatchType == EMatchTypes::EMT_DeathMatch)
				{
					World->ServerTravel(FString("/Game/_MINE/Maps/BlasterMap?listen"));
				}
				else if (MatchType == EMatchTypes::EMT_TeamDeathMatch)
				{
					World->ServerTravel(FString("/Game/_MINE/Maps/TeamsMap?listen"));
				}
				else if (MatchType == EMatchTypes::EMT_CaptureTheFlagMatch)
				{
					World->ServerTravel(FString("/Game/_MINE/Maps/CaptureTheFlagMap?listen"));
				}


				
			}
		}
	}
	


}
