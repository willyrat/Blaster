// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "CombatComponent.generated.h"


#define TRACE_LENGTH 80000.f

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

	UPROPERTY(Replicated)	
	bool bIsAiming;

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
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float CrosshairInAirSpreadValue;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float CrosshairInAirInterpSpeed;

	
	float CrosshairAimFactor;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float CrosshairAimSpreadValue;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float CrosshairAimInterpSpeed;
		
	float CrosshairShootingFactor;

	
	float CrosshairHaveTargetFactor;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float CrosshairHaveTargetSpreadValue;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float CrosshairHaveTargetInterpSpeed;
	bool HaveTarget = false;


	FVector HitTarget;

	//Aiming and FOV
	//Field of view when not aiming, set to the camera's base FOV in beginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	//automatic fire
	FTimerHandle FireTimer;

	
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();
	
	bool CanFire();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);


	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);


public:	
	//getters and setters
	

		
};
