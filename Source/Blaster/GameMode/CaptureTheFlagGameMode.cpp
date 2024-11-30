// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "Blaster/Weapon/Flag.h"
#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter,
	class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController)
{
	//Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	//we want functionaity of BlasterGameMode not TeamsGameMode...so dont call super....just call ABlasterGameMode::PlayerEliminated
	ABlasterGameMode::PlayerEliminated( ElimmedCharacter, VictimController, AttackerController);

	
}

void ACaptureTheFlagGameMode::FlagCaputured(class AFlag* Flag, class AFlagZone* Zone)
{
	bool bValidCaputer = Flag->GetTeam() != Zone->Team;
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(GameState);
	if (BGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();			
		}
		else if (Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();			
		}
	}
}
