// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"




//if we forward declare a variable we put class in front...if we do when we define the variable but we then reference that variable in a function 
//we need to move the class word into the function parm list and remove from the declaration... 
//OR we could forward declare just under the includes and then just use the variable throughout the header file without the class in front.
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	friend class ABlasterCharacter; //friend classes have full access to to private, protected and public members and methods
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//any time we plan on replicating variables, we need this function
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

	//if we forward declare a variable we put class in front...if we do when we define the variable but we then reference that variable in a function 
	//we need to move the class word into the function parm list and remove from the declaration... 
	//OR we could forward declare just under the includes and then just use the variable throughout the header file without the class in front.
	void EquipWeapon(AWeapon* WeaponToEquip);
	void SwapWeapons(); //lesson 172

	void Reload();
	UFUNCTION(BlueprintCallable)
	void finishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();
	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& target);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	bool bLocallyReloading = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;


	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachFlagToLeftHand(AActor* Flag);
	void AttachActorToBackpack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquippedWeaponSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);


private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;


	//if we forward declare a variable we put class in front...if we do when we define the variable but we then reference that variable in a function 
	//we need to move the class word into the function parm list and remove from the declaration... 
	//OR we could forward declare just under the includes and then just use the variable throughout the header file without the class in front.
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bIsAiming = false;

	//local bool that is the true value of whether the aiming button is being held
	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	//FVector HitTarget;

	//HUD and Crosshairs
	
	float CrosshairVelocityFactor;
	/*UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float CrosshairInAirSpreadValue;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float CrosshairInAirSpreadInterpSpeed;*/
	FHUDPackage HUDPackage;
	
	float CrosshairInAirFactor;
	UPROPERTY(EditAnywhere, Category = "Default|Combat|Crosshairs")
	float CrosshairInAirSpreadValue;
	UPROPERTY(EditAnywhere, Category = "Default|Combat|Crosshairs")
	float CrosshairInAirInterpSpeed;

	
	float CrosshairAimFactor;
	UPROPERTY(EditAnywhere, Category = "Default|Combat|Crosshairs")
	float CrosshairAimSpreadValue;
	UPROPERTY(EditAnywhere, Category = "Default|Combat|Crosshairs")
	float CrosshairAimInterpSpeed;
		
	float CrosshairShootingFactor;

	
	float CrosshairHaveTargetFactor;
	UPROPERTY(EditAnywhere, Category = "Default|Combat|Crosshairs")
	float CrosshairHaveTargetSpreadValue;
	UPROPERTY(EditAnywhere, Category = "Default|Combat|Crosshairs")
	float CrosshairHaveTargetInterpSpeed;
	bool HaveTarget = false;


	FVector HitTarget;

	//Aiming and FOV
	//Field of view when not aiming, set to the camera's base FOV in beginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = "Default|Combat|Weapon Properties")
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Default|Combat|Weapon Properties")
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	//automatic fire
	FTimerHandle FireTimer;

	
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();
	
	bool CanFire();

	//CarriedAmmo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere, Category = "Default|Combat|Ammo")
	int32 MaxCarriedAmmo = 500;
	
	UPROPERTY(EditAnywhere, Category = "Default|Combat|Ammo")
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Default|Combat|Ammo")
	int32 StartingRocketAmmo = 4;

	UPROPERTY(EditAnywhere, Category = "Default|Combat|Ammo")
	int32 StartingPistolAmmo = 15;

	UPROPERTY(EditAnywhere, Category = "Default|Combat|Ammo")
	int32 StartingSMGAmmo = 20;

	UPROPERTY(EditAnywhere, Category = "Default|Combat|Ammo")
	int32 StartingSniperAmmo = 10;

	UPROPERTY(EditAnywhere, Category = "Default|Combat|Ammo")
	int32 StartingShotgunAmmo = 10;

	UPROPERTY(EditAnywhere, Category = "Default|Combat|Ammo")
	int32 StartingGrenadeLauncherAmmo = 15;


	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 5;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 5;

	void UpdateHUDGrenades();

	void ShowAttachedGrenade(bool bShowGrenade);

	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;
	UFUNCTION()
	void OnRep_HoldingTheFlag();
	UPROPERTY()
	AWeapon* TheFlag;
	
public:	
	//getters and setters
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool ShouldSwapWeapons();
		
};




