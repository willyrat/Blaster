// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

//
//BLASTER_API is not required but allows this to be used like a dll in other thigs
namespace MatchState
{
	extern BLASTER_API const FName Cooldown;	//match duration has been reached. display winer and begin cooldown timer
}


/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;
	
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	
	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.0f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.0f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.0f;

	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:

	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
