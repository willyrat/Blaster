// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"

UENUM(BlueprintType)
enum class EMatchTypes : uint8
{
	EMT_DeathMatch UMETA(DisplayName = "Free For All"),
	EMT_TeamDeathMatch UMETA(DisplayName = "Teams"),
	EMT_CaptureTheFlagMatch UMETA(DisplayName = "Capture The Flag"),
	EMT_Max UMETA(DisplayName = "DefaultMAX")
};
/**
 *
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnectons = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	EMatchTypes MatchTypeEnum;
	
protected:
	virtual bool Initialize() override;	
	//virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override; //No longer suppurted...use line below
	virtual void NativeDestruct() override;
	
	//CAllbacks for the custom delegates on the MultiplayerSessionsSubsystem
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);	
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	//these are setup to bind to the buttons in the widget. the names have to be exact.
	UPROPERTY(meta = (BindWidget))
	class UButton* hostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* joinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	//Subsystem designed to handle all online session functionality
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	//Initialize with {x}
	UPROPERTY(BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	int32 NumPublicConnections{4};
	UPROPERTY(BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	FString MatchType{TEXT("FreeForAll")};
	FString PathToLobby{ TEXT("") };

	
};