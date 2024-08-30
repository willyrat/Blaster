// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictemPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictemPlayerState)
	{
		//update attacker's score
		AttackerPlayerState->AddToScore(1.f);
	}
	if (VictemPlayerState && AttackerPlayerState)
	{
		FString killersName = AttackerPlayerState->GetPlayerName();
		//update victem's defeats
		VictemPlayerState->AddToDefeats(1);
		VictemPlayerState->UpdateKilledBy(killersName);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}


void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{

	if (ElimmedCharacter)
	{
		//we are going to destroy the character, but keep the controller and gamestate to put into another character

		//Reset is inherited ... detatches player and controller...unposses the controller
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		//get random player start location
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this,APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}

}
