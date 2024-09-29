// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UENUM(BlueprintType)
enum class EHitType : uint8
{
	EHT_None UMETA(DisplayName = "None"),
	EHT_Player UMETA(DisplayName = "Player"),
	EHT_World UMETA(DisplayName = "World"),
	EHT_NPC UMETA(DisplayName = "NPC"),
	EHT_Other UMETA(DisplayName = "Other")
};


UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	//any time we plan on replicating variables, we need this function
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//when we bind to overlap and hit functions they have to be ufunctions
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties")
	float Damage = 20;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	class UParticleSystem* WorldImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	class USoundCue* WorldImpactSound;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	UParticleSystem* PlayerImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	class USoundCue* PlayerImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	//lesson 136 ...moved here from private
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnImpactEffect(EHitType TargetHitType, bool CallDestroy);

	//This setups a repnotified along with the function OnRep_OverlappingWeapon
	//Repnotifies do not get called on server...they are are only called when the variable replicates from the server...replication only works from server to client
	//so on listening servers we have to do something so server player gets replications.
	//UPROPERTY(ReplicatedUsing = OnRep_HitType)
	EHitType HitType;

	EHitType GetHitType(AActor* OtherActor);

private:
	

	

	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	
	
	
	////these are called automatically when the variable is replicated, so we cannot only send in the varible being replciated
	//UFUNCTION()
	//void OnRep_HitType();

	

	//void DelayDestroy();

public:	
	

};
