// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncement.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void SetElimAnnouncementText(FString AttackerName, FString VictimName);

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* AnnouncmentBox; //make sure this name is same as one used in WBP_ElimAnnouncement
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncmentText;	//make sure this name is same as one used in WBP_ElimAnnouncement

protected:


private:



};
