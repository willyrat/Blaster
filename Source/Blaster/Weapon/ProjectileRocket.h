// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
	
public:
	AProjectileRocket();
	virtual void Destroyed() override;

//#if is a compilation condition so it can go here in header file.  with_editor will not be compiled into a packaged build, only for the editor
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif	

protected:
	

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
	
	
	
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	USoundCue* ProjectileLoop;
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	USoundAttenuation* LoopingSoundAttenuation;

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;


private:
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	float InnerBlastRadius = 200.f;
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	float OuterBlastRadius = 500.f;
	

	
};
