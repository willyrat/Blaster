// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	class UTexture2D* CrosshairsLeft;
	class UTexture2D* CrosshairsRight;
	class UTexture2D* CrosshairsTop;
	class UTexture2D* CrosshairsBottom;
	float CrosshairSpread;	//this is used for all crosshair parts 

	FLinearColor CrosshairsColor;
};

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	void AddCharacterOverlay();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	
	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<class UUserWidget> AnnouncementClass;
	UPROPERTY(EditAnywhere, Category = "Announcements")
	class UAnnouncement* Announcement;

	void AddAnnouncement();
	void AddElimAnnouncement(FString Attacker, FString Victim);

protected:
	virtual void BeginPlay() override;
	

private:
	UPROPERTY()
	class APlayerController* OwningPlayer;
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texutre, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere, Category = "Announcements")
	float ElimAnnouncementTime = 3.f;
	UFUNCTION()
	void ElimAnnouncementTimerFinish(UElimAnnouncement* MsgToRemove);
	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;


public:
	//getters and setters

	FORCEINLINE void SetFHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
