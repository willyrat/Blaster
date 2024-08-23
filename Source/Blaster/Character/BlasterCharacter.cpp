// Fill out your copyright notice in the Description page of Project Settings.


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// for net update frequency adjustsments checkout https://www.udemy.com/course/unreal-engine-5-cpp-multiplayer-shooter/learn/lecture/31457444#questions
// in the bp anim for the character we changed a setting on the rotate root bone node.  under yaw scale bias clamp we checked the interp result box... this helps smooth things out a bit
// can also adjust the interp speeds in this section as well
// in character class we went to class default and under replication changed net update frequency from 100 to 66 and min net update frequency from 2 to 33
// you change these to 4 each to test when you have a really bad internet connection
// we also changed them in this class to default them 
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"

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
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	//this can be set in the class default panel under replication as well...defaulting them here
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
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

	AimOffset(DeltaTime);
	HideCameraIfCharacterClose();

}


void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	//https://www.youtube.com/watch?v=n3n8bqu7F9Q
	// 
	//const float DirectionValue = Value.Get<float>();

	
	const FVector2D MovementVector = Value.Get<FVector2D>();

	
	//UE_LOG(LogTemp, Warning, TEXT("IA_Move triggered"));
				
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
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}



//void ABlasterCharacter::Jump()
//{
//	Super::Jump();
//}

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

void ABlasterCharacter::FireButtonPressed(const FInputActionValue& Value)
{
	
	if (Combat)
	{
		//UE_LOG(LogTemp, Warning, TEXT("fire button pressed"));
		Combat->FireButtonPressed(true);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("combat is null"));
	}
}

void ABlasterCharacter::FireButtonReleased(const FInputActionValue& Value)
{
	
	if (Combat)
	{
		//UE_LOG(LogTemp, Warning, TEXT("fire button released"));
		Combat->FireButtonPressed(false);
	}
}


void ABlasterCharacter::AimButtonPressed(const FInputActionValue& Value)
{	
	
	if (Combat)
	{
		//UE_LOG(LogTemp, Warning, TEXT("aim button pressed"));
		Combat->SetAiming(true);
	}	
}

void ABlasterCharacter::AimButtonReleased(const FInputActionValue& Value)
{	
	
	if (Combat)
	{
		//UE_LOG(LogTemp, Warning, TEXT("aim button released"));
		Combat->SetAiming(false);
	}

}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	//pitch gets messed up because of how unreal packages (compresses) data to send across network
	//this is done in CharacterMovementComponent.cpp in GetPackedAngles..it converst rotation to 5 bites (unsigned)
	//he goes over this around 8 min mark in lesson 58 pitch in multiplayer	
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}


	//if (!HasAuthority() && IsLocallyControlled)	//this should show on client that is being controlled by player
	//if (HasAuthority() && !IsLocallyControlled)	//this should show on server
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("AO_Pitch %f: "), AO_Pitch);
	//}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("AO_Yaw %f: "), AO_Yaw);
		
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
		//UE_LOG(LogTemp, Warning, TEXT("turning right"));
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
		//UE_LOG(LogTemp, Warning, TEXT("turning left"));
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
			//UE_LOG(LogTemp, Warning, TEXT("not turning"));
		}
		
	}

	/*if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;		
		UE_LOG(LogTemp, Warning, TEXT("turning right"));
	}
	else if (AO_Yaw < -90.f)	
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
		UE_LOG(LogTemp, Warning, TEXT("turning left"));
	}
	
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);		
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)		
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);			
		}
		UE_LOG(LogTemp, Warning, TEXT("not turning"));
	}	*/
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) 	
	{
		return;
	}
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)	
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())		
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;			
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
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
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlasterCharacter::CrouchButtonPressed); // Triggered
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlasterCharacter::AimButtonReleased);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlasterCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlasterCharacter::FireButtonReleased);

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

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
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

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr)
	{
		return nullptr;
	}

	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr)
	{
		return FVector();
	}

	return Combat->HitTarget;
}



