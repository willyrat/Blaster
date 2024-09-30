// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()
	
public:
	AProjectileGrenade();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	//adding this in because i thought i needed it, but doesnt look like i have to... keeping if i need to change later
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override; 

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	USoundCue* BounceSound;
};
