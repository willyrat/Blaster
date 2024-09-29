// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	//virtual allows you to override it in child classes
	virtual void Fire(const FVector& HitTarget) override;	//sending in const reference is more efficient...other wise it will pass a copy of hittarget


private:
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Weapon Scatter")
	uint32 NumberOfPellets = 10;

};
