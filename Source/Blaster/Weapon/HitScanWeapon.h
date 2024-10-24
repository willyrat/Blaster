// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	//virtual allows you to override it in child classes
	virtual void Fire(const FVector& HitTarget) override;	//sending in const reference is more efficient...other wise it will pass a copy of hittarget

	

protected:
	
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	USoundCue* HitSound;
	
	


private:
	

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	USoundCue* FireSound;

	

	
	

};
