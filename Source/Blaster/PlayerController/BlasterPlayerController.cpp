// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}


void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
}


void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}




void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	//this only casts the first time through, otherwise it just assigns itself to itself...no cast then
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid =	BlasterHUD && 
						BlasterHUD->CharacterOverlay && 
						BlasterHUD->CharacterOverlay->HealthBar && 
						BlasterHUD->CharacterOverlay->HealthText;
	/*if (BlasterHUD)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlasterHUD valid"));		
	}
	if (BlasterHUD && BlasterHUD->CharacterOverlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlasterHUD->CharacterOverlay valid"));
	}*/
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));

	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}

}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;

	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{	
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}

}



void ABlasterPlayerController::SetHUDWeaponType(EWeaponType Weapontype)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponType;

	if (bHUDValid)
	{
		FString type = GetWeaponName(Weapontype);
		if(type.Len()>0)
		{
			BlasterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(type));
			BlasterHUD->CharacterOverlay->WeaponType->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->CharacterOverlay->WeaponType->SetVisibility(ESlateVisibility::Visible);			
		}
		else
		{
			BlasterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(type));
			BlasterHUD->CharacterOverlay->WeaponType->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->WeaponType->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

FString ABlasterPlayerController::GetWeaponName(EWeaponType Weapontype)
{
	FString type;

	switch (Weapontype)
	{
		case EWeaponType::EWT_AssultRifle:
			type = FString(TEXT("Assult Rifle"));
			break;
		case EWeaponType::EWT_MAX:
			type = FString(TEXT(""));
			break;
		default:
			type = FString(TEXT(""));
	}

	return type;
}

void ABlasterPlayerController::SetHUDKilledBy(FString killersName)
{
	UE_LOG(LogTemp, Warning, TEXT("~~~in SetHUDKilledBy"));

	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->KilledByTextMessage &&
		BlasterHUD->CharacterOverlay->KilledByText;

	if (bHUDValid)
	{
		
		if (killersName.Len() > 0)
		{
			BlasterHUD->CharacterOverlay->KilledByText->SetText(FText::FromString(killersName));
			BlasterHUD->CharacterOverlay->KilledByText->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->CharacterOverlay->KilledByTextMessage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			BlasterHUD->CharacterOverlay->KilledByText->SetText(FText::FromString(killersName));
			BlasterHUD->CharacterOverlay->KilledByText->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->KilledByTextMessage->SetVisibility(ESlateVisibility::Hidden);
		}
	}

}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountDownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.0f);
		int32 Seconds = CountDownTime - Minutes * 60;

		FString CountDownText =FString::Printf(TEXT("%02d:%02d"),Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountDownText));
	}
}


void ABlasterPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetWorld()->GetTimeSeconds());
	if (CountdownInt != SecondsLeft)
	{
		//time to update the hud
		SetHUDMatchCountdown(MatchTime - GetWorld()->GetTimeSeconds()); //secondsleft is rounded so dont use that here
	}

	CountdownInt = SecondsLeft;
}


