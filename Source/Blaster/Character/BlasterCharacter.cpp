// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"


// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);


	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	//Dont need to register this...components are special..so just set as replicated
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//this will need #include "Net/UnrealNetwork.h"
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);

}



// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	;
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		// Get the local player subsystem
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Clear out existing mapping, and add our mapping
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(BlasterInputMapping, 0);
		}
	}

}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//this is ont optimal..but for now this will work.  will learn better in later class
	/*if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}*/
}


void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	//https://www.youtube.com/watch?v=n3n8bqu7F9Q
	// 
	//const float DirectionValue = Value.Get<float>();

	
	const FVector2D MovementVector = Value.Get<FVector2D>();

	
	UE_LOG(LogTemp, Warning, TEXT("IA_Move triggered"));
				
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	
	const FVector ForwardDirectoin = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);  
	AddMovementInput(ForwardDirectoin, MovementVector.Y);
	FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);  
	AddMovementInput(RightDirection, MovementVector.X);
		 
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}

}

void ABlasterCharacter::Jump()
{
	Super::Jump();
}

//setup input step 4
//this will be called on any machine where the player pressed the e key..both server and client
//things should be done on server...so dont equip things on client
void ABlasterCharacter::EquipButtonPressed(const FInputActionValue& Value)
{
	const bool equip = Value.Get<bool>();

	if (equip)
	{
		if (Combat)
		{
			if(HasAuthority())
			{ 
				Combat->EquipWeapon(OverlappingWeapon);
			}
			else
			{
				ServerEquipButtonPressed();
			}
		}
	}
}

//setup input step 5 ...not always needed
//rpc call needs to have _Implementation appened...only need when defining function..to call this function just use ServerEquipButtonPressed
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{

	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}

}


void ABlasterCharacter::CrouchButtonPressed(const FInputActionValue& Value)
{
	const bool crouch = Value.Get<bool>();

	//character has a replicated crouch and uncrouch functions... we can override it or jsut call it...does a lot of work, adjusts speed and collision size
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}



//void ABlasterCharacter::ServerEquipButtonPressed()
//{
//
//}


void ABlasterCharacter::AimButtonPressed(const FInputActionValue& Value)
{
	
	if (Combat)
	{
		Combat->SetAiming(true);
	}	
}

void ABlasterCharacter::AimButtonReleased(const FInputActionValue& Value)
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}

}

//void ABlasterCharacter::ServerCrouchButtonPressed_Implementation()
//{
//
//	if (Combat)
//	{
//		//Combat->EquipWeapon(OverlappingWeapon);
//	}
//
//}


void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	//before we set OverlappingWeapon = Weapon check to see if OverlappingWeapon is set...
	//if it is then we stop shwoing widget
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	//this will be true when called only on the player that is being controlled
	//and since SetOverlappingWeapon is only run on the server...it will be true (on listening servers)
	if (IsLocallyControlled())	
	{
		//we are on the character being controlled by the player that is hosting the server...
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}

	}

}


void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}



// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{

		//setup input step 6
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::EquipButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlasterCharacter::AimButtonReleased);

	}


}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}


bool ABlasterCharacter::IsWeaponEquipped()
{
	//returns true if both of these are true
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bIsAiming);

}



