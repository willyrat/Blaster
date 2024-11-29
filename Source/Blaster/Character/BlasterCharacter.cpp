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



//!!! if you get an error code 6 with very little info, this usually has to do with a UPROPERTY issue.  Launch unreal with previous build and do a hot reload
//! unreal usually shows more info on code 6 errors
//! 
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
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/GameState/BlasterGameState.h"




// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//this can be set in blueprint as well
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

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


	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	//Dont need to register this...components are special..so just set as replicated
	Buff->SetIsReplicated(true);

	//this is only used on server, so no need to replicate
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));


	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	//this can be set in the class default panel under replication as well...defaulting them here
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**
	* Hit boxes for server-side rewind
	*/
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));	
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));	
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);			
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//this will need #include "Net/UnrealNetwork.h"
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGamePlay);
}



//when trying to rotate character from tick the remote clients run on net tick which is not as fast as tick...so there are times when it is not updated
//so call simProxiesTurn on OnRep_ReplicatedMovement so the rotation happens when a tick happens
//this only gets call when movement changes
void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	//if (GetLocalRole() == ENetRole::ROLE_SimulatedProxy)
	//{
	SimProxiesTurn();
	//}

	TimeSinceLastMovementReplication = 0.f;

}




void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (EimBotComponent)
	{
		EimBotComponent->DestroyComponent();
	}
	
	// ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	//will not destroy the weapon if game is in progress state
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

//server only...this gets called from gamemode and gamemode only exists and runs on server
void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapons();
	//do multicast call
	MulticastElim(bPlayerLeftGame);
	
}
//Multicast
void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}

	bElimmed = true;
	PlayElimMontage();

	//start dissolve effect here
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	SatrtDissolve();

	//diable character movement
	GetCharacterMovement()->DisableMovement();			//stops movement with wasd
	GetCharacterMovement()->StopMovementImmediately();	//stops rotating with mouse
	if (BlasterPlayerController)
	{
		//DisableInput(BlasterPlayerController);	//turn off mouse input so player cannot fire
		bDisableGamePlay = true;
	}

	if(Combat)	//turn off firing of weapon
	{
		Combat->FireButtonPressed(false);
	}

	//disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	//Spawn elimBot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		EimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());

	}

	bool bHideSniperScope = IsLocallyControlled()
		&& Combat && Combat->bIsAiming 
		&& Combat->EquippedWeapon 
		&& Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;

	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
}

//should only be called by server
void ABlasterCharacter::ElimTimerFinished()
{
	// ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
		//BlasterGameMode->RequestRespawn(ElimmedCharacter, ElimmedController);
	}
	if (bLeftGame && IsLocallyControlled()) //only want to broadcast on client that is trying to leave, so check isLocallyControlled
	{
		OnLeftGame.Broadcast();
	}
	
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	// ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	//ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr)
	{
		return;
	}

	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}

		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
		if (Combat->TheFlag)
		{
			Combat->TheFlag->Dropped();
		}

	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance) 
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::SatrtDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		//DissolveCurve is set in blueprint
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

//void ABlasterCharacter::UpdateStealthMaterial(float CloakOpacity, float CloakRefraction)
//{
//	if (DynamicStealthMaterialInstance)
//	{
//		DynamicStealthMaterialInstance->SetScalarParameterValue(TEXT("CloakOpacity"), CloakOpacity);
//	}
//}

void ABlasterCharacter::StealthBuff(bool bIsStart)
{
	if (bIsStart)
	{
		StartStealth();
	}
	else
	{
		ResetStealth();
	}
}
void ABlasterCharacter::StartStealth()
{
	if (StealthMaterialInstance)
	{
		DynamicStealthMaterialInstance = UMaterialInstanceDynamic::Create(StealthMaterialInstance, this);
		
		if (DefaultMaterial)
		{
			GetMesh()->SetMaterial(0, DynamicStealthMaterialInstance);
			DynamicStealthMaterialInstance->SetScalarParameterValue(TEXT("CloakOpacity"), CloakOpacity);
			DynamicStealthMaterialInstance->SetScalarParameterValue(TEXT("CloakRefraction"), CloakRefraction);
		}
		
	}
	//SatrtDissolve();
}

void ABlasterCharacter::ResetStealth()
{
	if (DefaultMaterial)
	{		
		GetMesh()->SetMaterial(0, DefaultMaterial);
	}
	/*DynamicStealthMaterialInstance = UMaterialInstanceDynamic::Create(StealthMaterialInstance, this);
	
	DynamicStealthMaterialInstance->SetScalarParameterValue(TEXT("CloakOpacity"), CloakOpacity);
	DynamicStealthMaterialInstance->SetScalarParameterValue(TEXT("CloakRefraction"), CloakRefraction);*/
}

UMaterialInterface* GetMeshMaterial(UStaticMeshComponent* MeshComponent, int32 MaterialIndex)
{
	if (MeshComponent)
	{
		return MeshComponent->GetMaterial(MaterialIndex);
	}
	return nullptr;
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr)
	{
		return;
	}

	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}

	if (CrownComponent)
	{
		CrownComponent->Activate();
	}

}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr)
	{
		return;
	}
	
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, DefaultMaterial);
		DissolveMaterialInstance = BlueDisolveMatInst;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDisolveMatInst;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDisolveMatInst;
		break;
	}
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapon();	//lesson 170
	UpdateHUDAmmo();
	
	UpdateHUDHealth(); //need for when game starts...other wise on respanw it will be in updated in BlasterPlayerController
	UpdateHUDShield();


	if (HasAuthority())
	{
		//bind to applydamage event we setup in ProjectileBullet... ReceiveDamage is in this class
		//AddDynamic is a macro that calls __Internal_AddDynamic ... for some reason AddDynamic does not show in the intelli sense dropdown but does seem to work
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}

	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}

	
	//!!!below is needed for the enhanced input system that is not part of this course...do not remove!!!!
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController = BlasterPlayerController;
	//if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	if (BlasterPlayerController)
	{
		// Get the local player subsystem
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BlasterPlayerController->GetLocalPlayer()))
		{
			// Clear out existing mapping, and add our mapping
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(BlasterInputMapping, 0);
		}
	}

	

}

//This is needed for server...
void ABlasterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//!!!below is needed for the enhanced input system that is not part of this course...do not remove!!!!
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController = BlasterPlayerController;
	//if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	if (BlasterPlayerController)
	{
		// Get the local player subsystem
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BlasterPlayerController->GetLocalPlayer()))
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

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

//runs on both client and server
void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	if (bDisableGamePlay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		//now called in OnRep_ReplicatedMovement
		//SimProxiesTurn();

		CalculateAO_Pitch();
	}
	
}





void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	//https://www.youtube.com/watch?v=n3n8bqu7F9Q
	// 
	//const float DirectionValue = Value.Get<float>();

	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}
	
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
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	
	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}

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
	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}

	const bool equip = Value.Get<bool>();

	if (equip)
	{
		if (Combat)
		{
			if (Combat->bHoldingTheFlag)
			{
				//just return
				return;
			}
			
			if (Combat->CombatState == ECombatState::ECS_Unoccupied)
			{
				ServerEquipButtonPressed();
			}

			//dont need to play montage on server here...it will be done in CombatComponent::SwapWeapons
			//because ServerEquipButtonPressed call above calls Combat->SwapWeapons()
			bool bSwap = Combat->ShouldSwapWeapons() && 
				!HasAuthority() && 
				Combat->CombatState == ECombatState::ECS_Unoccupied && 
				OverlappingWeapon == nullptr;
			if (bSwap)
			{
				//lesson 207 hide lag by playing animation when switching weapons...
				PlaySwapMontage();
				Combat->CombatState = ECombatState::ECS_SwappingWeapons;
				bFinishedSwapping = false;
			}

			//lesson 171 below check is not needed, since calling ServerEquipButtonPressed() will happen on server whether it has authority or not...
			/*if(HasAuthority())
			{ 
				Combat->EquipWeapon(OverlappingWeapon);
			}
			else
			{
				ServerEquipButtonPressed();
			}*/
		}
	}
}

//setup input step 5 ...not always needed
//rpc call needs to have _Implementation appened...only need when defining function..to call this function just use ServerEquipButtonPressed
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{

	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if(Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}

	}

}


void ABlasterCharacter::CrouchButtonPressed(const FInputActionValue& Value)
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}

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
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}
	
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
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}
	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}

	if (Combat)
	{
		//UE_LOG(LogTemp, Warning, TEXT("fire button released"));
		Combat->FireButtonPressed(false);
	}
}


void ABlasterCharacter::AimButtonPressed(const FInputActionValue& Value)
{	

	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}

	if (Combat)
	{
		//UE_LOG(LogTemp, Warning, TEXT("aim button pressed"));
		Combat->SetAiming(true);
	}	
}

void ABlasterCharacter::AimButtonReleased(const FInputActionValue& Value)
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}

	if (Combat)
	{
		//UE_LOG(LogTemp, Warning, TEXT("aim button released"));
		Combat->SetAiming(false);
	}

}


void ABlasterCharacter::ReloadButtonPressed(const FInputActionValue& Value)
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	//there are times we want to restrict the players movement
	if (bDisableGamePlay)
	{
		return;
	}

	if (Combat)
	{
		Combat->Reload();
	}

}

void ABlasterCharacter::GrenadeButtonPressed(const FInputActionValue& Value)
{
	if (Combat)
	{
		if (Combat->bHoldingTheFlag)
		{
			return;
		}
		Combat->ThrowGrenade();
	}
}

//lesson 212 ... this is setup in playercontroller 
//void ABlasterCharacter::EscapeButtonPressed(const FInputActionValue& Value)
//{
//
//}




float ABlasterCharacter::CalculateSpeed()
{

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
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
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	//pitch gets messed up because of how unreal packages (compresses) data to send across network
	//this is done in CharacterMovementComponent.cpp in GetPackedAngles..it converst rotation to 5 bites (unsigned)
	//he goes over this around 8 min mark in lesson 58 pitch in multiplayer	
	//if (Speed > 0.f || bIsInAir) // running, or jumping
	//{
	//	StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	//	AO_Yaw = 0.f;
	//	bUseControllerRotationYaw = true;
	//	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	//}
	
	CalculateAO_Pitch();


	//if (!HasAuthority() && IsLocallyControlled)	//this should show on client that is being controlled by player
	//if (HasAuthority() && !IsLocallyControlled)	//this should show on server
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("AO_Pitch %f: "), AO_Pitch);
	//}
}

void ABlasterCharacter::CalculateAO_Pitch()
{

	float OriginalPitch = GetBaseAimRotation().Pitch;
	AO_Pitch = OriginalPitch;
	//AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}

	/*UE_LOG(LogTemp, Warning, TEXT("Client: %s | OriginalPitch: %f | MappedPitch: %f | IsLocallyControlled: %s"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"),
		OriginalPitch,
		AO_Pitch,
		IsLocallyControlled() ? TEXT("true") : TEXT("false"));
	*/
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	bRotateRootBone = false;

	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	//CalculateAO_Pitch();

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	//get difference in rotation since last frame
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	//UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		bUseControllerRotationYaw = true; // added this
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

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
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())		
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;			
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
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


void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
	
	//PlayHitReactMontage();
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Health < LastShield)
	{
		PlayHitReactMontage();
	}

	//PlayHitReactMontage();
}



void ABlasterCharacter::UpdateHUDHealth()
{
	//Use this instead of a straight cast so we only cast the first time through
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController = BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	//Use this instead of a straight cast so we only cast the first time through
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController = BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController = BlasterPlayerController;
	if (BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
			BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
			BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}


void ABlasterCharacter::SpawnDefaultWeapon()
{
	// ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true; //Only want to destroy default weapon...
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}

}



void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			//send in 0 just to trigger hud update...nothing is changing the score or defeats
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
			BlasterPlayerState->UpdateKilledBy("");
			SetTeamColor(BlasterPlayerState->GetTeam());
			
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}

			if (BlasterPlayerController)
			{
				BlasterPlayerController->SetHUDWeaponType(EWeaponType::EWT_MAX);
			}

			DefaultMaterial = GetMesh()->GetMaterial(0);
		}

	}
}



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
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ABlasterCharacter::EquipButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlasterCharacter::CrouchButtonPressed); // Triggered
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABlasterCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlasterCharacter::AimButtonReleased);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlasterCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlasterCharacter::FireButtonReleased);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(GrenadeAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::GrenadeButtonPressed);
		//below is bound in playercontroller...lesson 212... this was never setup here, started to but it needs to go in playercontroller
//		EnhancedInputComponent->BindAction(EscapeAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::EscapeButtonPressed);

	}


}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);

		}
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

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		//SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:	//...found in lesson 136
			SectionName = FName("RocketLauncher");
			break;	
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");	
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");	//using Pistol montage
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");	
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");	
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}	
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);		
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName("FromFront");		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	

	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	if (bElimmed || BlasterGameMode == nullptr)	//check to see if the player has been killed and is in elimination sequence...keeps from spam killing players
	{
		return;
	}
	Damage = BlasterGameMode->CalculateDamage(InstigatorController,Controller,Damage);
	
	float DamageToHealth = Damage;
	//check to see if player has shield...
	if (Shield > 0.f)
	{
		if (Shield >= Damage)	
		{
			//shield will take all the damgage
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			//shield will take some or none of the damage			
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	//health is replcated with rep notify... 
	//Using variabl replication is more efficient then sending an RPC, so try to avoid sending them.
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();

	PlayHitReactMontage();
	
	if (Health <= 0.f)
	{
		
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController = BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}

}

//void ABlasterCharacter::MulticastHit_Implementation()
//{
//	PlayHitReactMontage();
//
//}


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

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr)
	{
		return ECombatState::ECS_MAX;
	}

	return Combat->CombatState;	
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if (Combat == nullptr)
	{
		return false;
	}

	return Combat->bLocallyReloading;
}

bool ABlasterCharacter::IsHoldingTheFlag() const
{
	
	if (Combat == nullptr)
	{
		return false;
	}

	return Combat->bHoldingTheFlag;
}

ETeam ABlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterPlayerState == nullptr)
	{
		return ETeam::ET_NoTeam;
	}
	return BlasterPlayerState->GetTeam();
}



