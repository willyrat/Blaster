// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"


#include "BlasterCharacter.generated.h"

//try to forward declare when you can when in .h files
//forward declare
//pointers you can forward declare, but structs you cannot... structs are generally small so should be safe...see Move function below
class UInputMappingContext;


UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void  Jump() override;

	//any time we plan on replicating variables, we need this function
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* BlasterInputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* CrouchAction;

	//use this to add a new input action
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	//class UInputAction* LookAction;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void EquipButtonPressed(const FInputActionValue& Value);
	void CrouchButtonPressed(const FInputActionValue& Value);
	



private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	//This setups a repnotified along with the function OnRep_OverlappingWeapon
	//Repnotifies do not get called on server...they are are only called when the variable replicates from the server...replication only works from server to client
	//so on listening servers we have to do something so server player gets replications.
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	//these are called automatically when the variable is replicated, so we cannot only send in the varible being replciated
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;


	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	//UFUNCTION(Server, Reliable)
	//void ServerCrouchButtonPressed();

public:
	//getters and setters
	//getters and setters
	//below is taken out and replaced so server can get repnotified changes
	//FORCEINLINE void SetOverlappingWeapon(AWeapon* Weapon) { OverlappingWeapon = Weapon; }
	//now it will have more logic and will be defined in cpp file
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();

};
