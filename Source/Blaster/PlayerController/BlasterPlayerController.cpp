// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Components/Image.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//ServerCheckMatchState();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());

	ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);
	PollInit();

	CheckPing(DeltaTime);
}

void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)	
	{
		//PlayerState = GetPlayerState<APlayerState>();
		PlayerState = PlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : PlayerState;
		if (PlayerState)
		{
			int8 pingTime = PlayerState->GetCompressedPing() * 4;
			//if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold)		//lesson 176 GetPing() id deprecated... ping is compressed (it is divided by 4) so we * 4 to get full ping
			if (pingTime  > HighPingThreshold)		//lesson 176 GetPing() id deprecated... ping is compressed (it is divided by 4) so we * 4 to get full ping
			{
				HighPingWarnging();
				PingAnimationRunningTime = 0.f;
			}
		}

		HighPingRunningTime = 0.f;
	}

	bool bHighPingAnimationPlaying = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingAnimation &&
		BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);

	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}




void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmUpTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CooldownTime = GameMode->CooldownTime;
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);

		/*
		if (BlasterHUD && MatchState == MatchState::WaitingToStart)
		{
			BlasterHUD->AddAnnouncement();
		}*/
	}
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);
	
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}


void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth)
				{
					SetHUDHealth(HUDHealth, HUDMaxHealth);
				}
				if (bInitializeShield) 
				{
					SetHUDShield(HUDShield,HUDMaxShield);
				}
				if (bInitializeScore) 
				{
					SetHUDScore(HUDScore);
				}
				if (bInitializeDefeats) 
				{
					SetHUDDefeats(HUDDefeats);
				}
				if (bInitializeCarriedAmmo)
				{
					SetHUDCarriedAmmo(HUDCarriedAmmo);
				}
				if (bInitializeWeaponAmmo)
				{
					SetHUDWeaponAmmo(HUDWeaponAmmo);
				}


				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if (BlasterCharacter && BlasterCharacter->GetCombat())
				{
					if (bInitializeGrenades) 
					{
						SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades());
					}
				}				
			}
		}
	}
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
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	//this only casts the first time through, otherwise it just assigns itself to itself...no cast then
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldBar &&
		BlasterHUD->CharacterOverlay->ShieldText;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
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
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
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
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
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
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;

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
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;

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

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		if(CountdownTime < 0.f)
		{		
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.0f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountDownText =FString::Printf(TEXT("%02d:%02d"),Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountDownText));
		
		if(Minutes == 0 && Seconds <= 30)
		{
			if (Seconds % 2 == 0)
			{
				BlasterHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			}
			else
			{
				BlasterHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));			
			}
		}
		
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.0f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));

	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadesText;

	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}

}


void ABlasterPlayerController::SetHUDTime()
{
	
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		//WarmupTime is entire duration of warm up state that has happened - server time (time since game has launched which gets bigger)
		//+ levelStartingTime (could be time spent in menu)
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		//now add on MatchTime
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{		
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	//if (HasAuthority())
	//{
	//	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	//	if (BlasterGameMode) //should have if we are on the server
	//	{
	//		SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
	//	}
	//}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{			
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			//time to update the hud
			//SetHUDMatchCountdown(MatchTime - GetServerTime()); //secondsleft is rounded so dont use that here
			SetHUDMatchCountdown(TimeLeft);
		}		
	}

	CountdownInt = SecondsLeft;
}


void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

//this takes into account amount of time it takes to send message to server and how much time it took to come back (RountTripTime)
void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime); //this calculates the time on server
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds(); //difference between server and client starting times

}



float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
	
}

//this is the earliest we can get time
void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController()) //when called on client
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

//only runs on server...will need to update client so we made OnRep_MatchState 
void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	
	if (MatchState == MatchState::InProgress)
	{
		//BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}
void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD)
	{
		if (BlasterHUD->CharacterOverlay == nullptr)
		{
			BlasterHUD->AddCharacterOverlay();
		}

		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		//BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

		HandleCooldown();
	}
	
}
void ABlasterPlayerController::HighPingWarnging()
{
	//this only casts the first time through, otherwise it just assigns itself to itself...no cast then
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;
	
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->HighPingImage->SetRenderOpacity(1.f);
		//BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation, 0.f, 5);
	}

}
void ABlasterPlayerController::StopHighPingWarning()
{
	//this only casts the first time through, otherwise it just assigns itself to itself...no cast then
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetRenderOpacity(0.f);
		//BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0);
		if (BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}


void ABlasterPlayerController::HandleCooldown()
{

	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = BlasterHUD->Announcement &&
			BlasterHUD->Announcement->AnnouncementText &&
			BlasterHUD->Announcement->InfoText;
		if (bHUDValid)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if (BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				FString InfoTextString;

				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner...");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)	//current player won
				{
					InfoTextString = FString("You are the winner!!!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win:\n");
					for (auto TiedPlayers : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayers->GetPlayerName()));
					}
				}

				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}			
		}
	}

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter && BlasterCharacter->GetCombat())
	{
		BlasterCharacter->bDisableGamePlay = true;
		BlasterCharacter->GetCombat()->FireButtonPressed(false);
	}
}

