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
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	USoundCue* HitSound;
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	float Damage = 20.f;


private:
	

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	USoundCue* FireSound;

	

	//trace end with scatter
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Weapon Scatter")
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Weapon Scatter")
	float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Weapon Scatter")
	bool bUseScatter = false;


};
