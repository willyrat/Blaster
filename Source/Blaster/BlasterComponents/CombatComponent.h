// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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


private:
	class ABlasterCharacter* Character;

	//if we forward declare a variable we put class in front...if we do when we define the variable but we then reference that variable in a function 
	//we need to move the class word into the function parm list and remove from the declaration... 
	//OR we could forward declare just under the includes and then just use the variable throughout the header file without the class in front.
	UPROPERTY(Replicated)
	AWeapon* EquippedWeapon;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	

		
};
