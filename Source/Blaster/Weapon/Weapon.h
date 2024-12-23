// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_Max UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	EFT_Max UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
// Called every frame
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);

	//any time we plan on replicating variables, we need this function
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Owner() override;
	void SetHUDAmmo();

	//virtual allows you to override it in child classes
	virtual void Fire(const FVector& HitTarget);	//sending in const reference is more efficient...other wise it will pass a copy of hittarget

	//lesson 235 makes this virtual so we can override in flag class
	virtual void Dropped();
	void AddAmmo(int32 AmmoToAdd);

	FVector TraceEndWithScatter(const FVector& HitTarget);

	//Textures for the weapon crosshairs
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Crosshairs")
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Crosshairs")
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Crosshairs")
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Crosshairs")
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Crosshairs")
	UTexture2D* CrosshairsBottom;

	//Automatic fire
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Combat")
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Combat")
	bool bAutomatic = true;


	//sounds
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Sounds")
	class USoundCue* EquipSound;

	///Enable or disable custom depth
	void EnableCustomDepth(bool bEnable);
		
	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Weapon Scatter")
	bool bUseScatter = false;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnEquippedSecondary();
	virtual void OnDropped();

	//UFunction()
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	//trace end with scatter
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Weapon Scatter")
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Weapon Scatter")
	float SphereRadius = 75.f;
	
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	float HeadShotDamage = 40.f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Default|Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Default|Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")	
	EWeaponState WeaponState;
	
	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Default|Weapon Properties|FXs")
	class UWidgetComponent*  PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|FXs")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Casing Properties")
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Casing Properties")
	float CasingEjectPitchMin;
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Casing Properties")
	float CasingEjectPitchMax;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Casing Properties")
	float CasingEjectYawMin;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Casing Properties")
	float CasingEjectYawMax;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Casing Properties")
	float CasingEjectRollMax;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Casing Properties")
	float CasingEjectRollMin;

	
	//zoomed fov while aiming
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Aiming")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Aiming")
	float ZoomInterpSpeed = 20.f;

	////lesson 183... we are going to be using rpcs for replication so we can do server reconciliation
	//UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo, Category = "Default|Weapon Properties|Ammo")
	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Ammo")
	int32 Ammo;

	void SpendRound();
	//lesson 183... we are going to be using rpcs for replication so we can do server reconciliation
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	//lesson 183... we are going to be using rpcs for replication so we can do server reconciliation
	UFUNCTION(Client, Reliable)	
	void ClientAddAmmo(int32 AmmoToAdd);
	//lesson 183... we are going to be using rpcs for replication so we can do server reconciliation
	/*UFUNCTION()
	void OnRep_Ammo();*/
		

	UPROPERTY(EditAnywhere, Category = "Default|Weapon Properties|Ammo")
	int32 MagCapacity;

	//the number of unprocessed server requests for ammo
	//incremented in SpendRound, decremented in ClientUpdateAmmo
	int32 Sequence = 0;


	

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	ETeam Team;

public:	
	void SetWeaponState(EWeaponState State);	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }	
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	bool IsEmpty();
	bool IsFull();
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	FORCEINLINE ETeam GetTeam() const { return Team; }
};
