// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;  //this needs to be the same name as the blueprint variable for the text widget

	void SetDisplayText(FString TextToDisplay);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);


protected:
	//virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);	//not supported in UE 5.1+
	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

};
