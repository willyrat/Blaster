// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
 //!!! if you get an error code 6 with very little info, this usually has to do with a UPROPERTY issue.  Launch unreal with previous build and do a hot reload
 //! unreal usually shows more info on code 6 errors
 //!
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	//replicatoin notifies
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	UFUNCTION()
	virtual void OnRep_KilledBy();

	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount );
	void UpdateKilledBy(FString killersName);

protected:

private:
	//these 2 variables are undefined and are not null...if we try to do if check on them if(Character) could pass
	//this is causing a hard crash at times in functions like AddToScore...
	//you can set to nullptr or set as a UPROPERTY()..both will initialize them and avoid checking undefined variables
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_KilledBy)
	FString killersName;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;
	UFUNCTION()
	void OnRep_Team();
	
public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);
};
