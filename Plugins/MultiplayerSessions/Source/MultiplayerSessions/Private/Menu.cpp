// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"



void UMenu::MenuSetup(int32 NumberOfPublicConnectons, EMatchTypes TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnectons;
	MatchType = TypeOfMatch;

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);

		}
	}

	//get our MultiplayerSessionsSubsystem
	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{		
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsSessionComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);

	}
}

//Initialize runs too early to setup properites like we do above.
bool UMenu::Initialize()
{
	if (!Super::Initialize()) 
	{
		return false;
	}

	//bind functions
	if (hostButton)
	{
		hostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}

	if (joinButton)
	{
		joinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}



//Unreal 5+ does not have OnLevelRemovedFromWorld use code just under this
//void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
//{
//	MenuTearDown();		
//	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
//
//}
void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("Session Created Successfully!"))
			);
		}

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{	
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}

		hostButton->SetIsEnabled(true);
	}
	

}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	for (auto Result : SessionResults)
	{
		//FString SettingsValue;
		//EMatchTypes SettingsValue;
		int32 SettingsValueAsInt = 0;
		//Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValueAsInt);
		//if (SettingsValue == MatchType)
		EMatchTypes SettingsValue = static_cast<EMatchTypes>(SettingsValueAsInt);
		if (SettingsValue == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		joinButton->SetIsEnabled(true);
	}
}


void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get();
	if (SubSystem)
	{
		IOnlineSessionPtr SessionInterface = SubSystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address );

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}

		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		joinButton->SetIsEnabled(true);
	}
	//OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address)
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Green,
			FString(TEXT("Session started Successfully!"))
		);

		/*UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel("/Game/ThirdPerson/Maps/Lobby?listen");
		}*/

	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString(TEXT("Failed to start session!"))
		);
	}
}


void UMenu::HostButtonClicked()
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString(TEXT("Host button clicked!"))
		);
	}*/

	hostButton->SetIsEnabled(false);

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections,MatchType); 
	}
}

void UMenu::JoinButtonClicked()
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString(TEXT("Join button clicked!"))
		);
	}*/

	joinButton->SetIsEnabled(false);

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(100000);
	}

}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);

		}

	}
}
