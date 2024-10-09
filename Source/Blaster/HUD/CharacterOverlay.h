// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	//Bind these to widget components in HUD...names must be exact
	
	//health bar
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))	
	class UTextBlock* HealthText;

	//Shield bar
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;



	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DefeatsAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KilledByTextMessage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KilledByText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WeaponAmmoAmount;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WeaponType;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GrenadesText;


	/*UFUNCTION()
	void ShowKilledBy();*/
};
