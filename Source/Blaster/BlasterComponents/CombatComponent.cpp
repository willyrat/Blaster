// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/Shotgun.h"


//Add variables here so they are replicated to all clients from server
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);  //only replicate from server to owner
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
}

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 400.f;

}



void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast < ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}						
	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}

}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}

}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}

}



void UCombatComponent::FireButtonPressed(bool bPressed)
{
	//UE_LOG(LogTemp, Warning, TEXT("in CombatComponent::FireButtonpPressed"));
	bFireButtonPressed = bPressed;
	
	if (bFireButtonPressed)
	{		
		//UE_LOG(LogTemp, Warning, TEXT("call Fire()"));
		Fire();
	}
}

//this gets called from anim BP when the shell anim notify event is triggered in montage shotgun section
void UCombatComponent::ShotgunShellReload()
{
	
	//want to load 1 round... only do this on server since ammo is replicated
	if(Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}



void UCombatComponent::Fire()
{
	//UE_LOG(LogTemp, Warning, TEXT("bCanFire %d"), static_cast<int32>(bCanFire));
	if (CanFire() && EquippedWeapon)
	{
		bCanFire = false;

		//moved into weapontype fire functions below ...lesson 179
		//ServerFire(HitTarget);	//executed on server
		//LocalFire(HitTarget);


		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 1.75f;

			switch (EquippedWeapon->FireType)
			{
				case EFireType::EFT_Projectile:		
					FireProjectileWeapon();
					break;
				case EFireType::EFT_HitScan:
					FireHitScanWeapon();
					break;
				case EFireType::EFT_Shotgun:
					FireShotgun();
					break;
			}
		}

		StartFireTimer();
	}

	
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority())
		{
			LocalFire(HitTarget);
		}

		ServerFire(HitTarget, EquippedWeapon->FireDelay);	//executed on server
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority())
		{
			LocalFire(HitTarget);
		}
		ServerFire(HitTarget, EquippedWeapon->FireDelay);	//executed on server
		
	}
	//TraceEndWithScatter(const FVector & HitTarget);
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);

	if(Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority())
		{
			ShotgunLocalFire(HitTargets);
		}
		ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
	}
}



void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
	{
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("start fire timer"));
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);


}

void UCombatComponent::FireTimerFinished()
{
	//UE_LOG(LogTemp, Warning, TEXT("fire timer finished"));

	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		//UE_LOG(LogTemp, Warning, TEXT("call fire()2"));
		Fire();
	}

	ReloadEmptyWeapon();

	//if (EquippedWeapon->IsEmpty())
	//{
	//	Reload();
	//}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr)
	{
		return false;
	}

	

	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		return true;
	}

	//lesson 207 moved this check below one above so shotgun can now shoot while reloading
	if (bLocallyReloading)
	{
		return false;
	}

	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
	
}






//all RPCs have to have _Implementation added to function name... to call just use ServerFire()
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget); //executed on server and all clients

	/*if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (Character)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire();
		
	}*/
}
//we set the UPROPERTY for ServerFire to use WithValidation, so we do not need to declare ServerFire_Validate in the .h file
bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		//checking floating value, so we cannot assume they will be exact
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}

	return true;  //if player does not have weapon then they could not have a firedelay value, so just let it go...
}

//call multicast rpc instead of replicating bFireButtonPressed so we save some bandwidth
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//only want to run when on client
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())
	{
		return;
	}

	LocalFire(TraceHitTarget);
}

//lesson 181
void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets); //executed on server and all clients
}
void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{		
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())	
	{
		UE_LOG(LogTemp, Log, TEXT("we are locally controlled and we do not have authority"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("going into ShotgunLocalFire from UCombatComponent::ServerShotgunFire"));
	ShotgunLocalFire(TraceHitTargets);
}
//we set the UPROPERTY for ServerFire to use WithValidation, so we do not need to declare ServerFire_Validate in the .h file
bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)
	{
		//checking floating value, so we cannot assume they will be exact
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}

	return true;  //if player does not have weapon then they could not have a firedelay value, so just let it go...
}

//lesson 177
void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	//removed check in lesson 181 and replaced with ShotgunLocalFire()
	/*if (Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}*/
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		bLocallyReloading = false;
	}

}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);

	if (Shotgun == nullptr || Character == nullptr)
	{
		return;
	}
		
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		UE_LOG(LogTemp, Log, TEXT("in UCombatComponent::ShotgunLocalFire going into Shotgun->FireShotgun()"));
		Character->PlayFireMontage(bIsAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
		bLocallyReloading = false;
	}

}


void UCombatComponent::OnRep_EquippedWeapon()
{

	if (EquippedWeapon && Character)
	{
		//already called on server, but that may not have propagated over network to all clients yet so do here as well
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		/*if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}*/
		AttachActorToRightHand(EquippedWeapon);	//lesson 151

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		/*if (EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound, Character->GetActorLocation());

		}*/
		PlayEquippedWeaponSound(EquippedWeapon);//lesson 151
		EquippedWeapon->EnableCustomDepth(false);
		
		EquippedWeapon->SetHUDAmmo(); //lesson 172...make sure hud is updated on client when swaping weapons

		if (Controller)
		{
			Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
		}
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);	//lesson 171
		PlayEquippedWeaponSound(EquippedWeapon);
		/*if (SecondaryWeapon->GetWeaponMesh()) //took out in lesson 172
		{
			SecondaryWeapon->GetWeaponMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
			SecondaryWeapon->GetWeaponMesh()->MarkRenderStateDirty();
		}*/
	}
}



void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	if (CombatState != ECombatState::ECS_Unoccupied)	//lesson 150
	{
		return;
	}


	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else //if EquippedWeapon == nullptr || SecondaryWeapon != null
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}

	

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;

}

void UCombatComponent::SwapWeapons()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr)
	{
		return;
	}

	Character->PlaySwapMontage();	//lesson 207 hide lag by playing animation when switching weapons...
	Character->bFinishedSwapping = false;
	CombatState = ECombatState::ECS_SwappingWeapons;	//changing combatstat triggers rep_notify in onrep_combatstat

	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr)
	{
		return;
	}
	//if player already has weapon
	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToRightHand(EquippedWeapon);


	EquippedWeapon->SetOwner(Character);	//owner is replicated to clients by default...no need to setup
	//EquippedWeapon->ShowPickupWidget(false); EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//to update ammo on the hud when a player first pics up a gun, we can set right here for the server, but the clients may not have an owner available yet
	//could not go into weapon::weaponstate to update... 
	//owner is replicated in Actor.h in OnRep_Owner which we can override
	//so in weapon class we will override...
	//so in weapon class we will override... and have it call SetHUDAmmo which will happen on client
	EquippedWeapon->SetHUDAmmo();	//we need to make sure it is called on server as well

	UpdateCarriedAmmo();

	PlayEquippedWeaponSound(WeaponToEquip);

	ReloadEmptyWeapon();


	
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr)
	{
		return;
	}

	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);
	PlayEquippedWeaponSound(WeaponToEquip);
	/*if (SecondaryWeapon->GetWeaponMesh()) //took out in lesson 172
	{
		SecondaryWeapon->GetWeaponMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		SecondaryWeapon->GetWeaponMesh()->MarkRenderStateDirty();
	}*/
		
	SecondaryWeapon->SetOwner(Character);	//owner is replicated to clients by default...no need to setup
	
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bIsAiming = bAimButtonPressed;
	}
}


void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr)
	{
		return;
	}

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}

}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}

	//We have 2 sockets we can choose from based on weapon type... pistol and smg need to use PistolSocket, all others use LeftHandSocket
	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);

	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
		
	const USkeletalMeshSocket* backpackSocket = Character->GetMesh()->GetSocketByName(FName("backpackSocket"));
	if (backpackSocket)
	{
		backpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast < ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
		Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());

	}
}

void UCombatComponent::PlayEquippedWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation());

	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}




//this could be called on client or server
void UCombatComponent::Reload()
{
	//if we are on the client we can check to see if they have any carried ammo... if not then there is no need to 
	//do a call to server(save bandwidth)
	//if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && //..lesson 150
		EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)	//lesson 152
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}



//set to only run on server
void UCombatComponent::ServerReload_Implementation()
{
	//already checked for ammo...
	if (Character == nullptr || EquippedWeapon== nullptr)
	{
		return;
	}
	
	CombatState = ECombatState::ECS_Reloading;

	//call to run on server now...changed in lesson 185
	if (!Character->IsLocallyControlled())
	{
		HandleReload();
	}
	

}

void UCombatComponent::finishReloading()
{
	if (Character == nullptr)
	{
		return;
	}

	bLocallyReloading = false;
	
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}

	if (bFireButtonPressed)
	{
		Fire();
	}
	
}

void UCombatComponent::FinishSwap()
{	
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;			
	}
	if (Character)
	{
		Character->bFinishedSwapping = true;
	}
}
void UCombatComponent::FinishSwapAttachWeapons() //happens on all machines
{	
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();	//we need to make sure it is called on client as well..this is different than below
	UpdateCarriedAmmo();
	PlayEquippedWeaponSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);

}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if (Character && !Character->IsLocallyControlled())
		{
			HandleReload();
		}		
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled()) //dont need to play fro player that is throwing grenade since it already started playing for them
		{
			Character->PlayThrowGrenadeMontage();  
			AttachActorToLeftHand(EquippedWeapon);	//these are also called locally and on on server in ThrowGrenade and ServerThrowGrenade
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapons:
		if (Character && !Character->IsLocallyControlled()) //dont need to play fro player that is swapping since it already started playing for them
		{
			Character->PlaySwapMontage();			
		}
		break;
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}

	int32 ReloadAmount = AmountToReload();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast < ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character && Character->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("in UpdateShotgunAmmoValues...has authority"));		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("in UpdateShotgunAmmoValues...has authority"));
	}
	if (Character && Character->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("in UpdateShotgunAmmoValues...is locally controlled"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("in UpdateShotgunAmmoValues...is NOT locally controlled"));
	}

	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	//this is my code...this stops reload from removing more ammo from carried ammo than exists and avoids negative numbers in UI
	//maybe this gets addressed in future videos...current lesson is 148
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()) && CarriedAmmoMap[EquippedWeapon->GetWeaponType()] <= 0)
	{
		JumpToShotgunEnd();
		return;
	}

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}	
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(1);
	bCanFire = true;			//once we have 1 shell in shotgun we can allow player to start firing
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
		//this runs on server but we need it to run on client as well... so in weapon.cpp we do a call to JumpToShotgunEnd() in OnRep_Ammo()
	}
}


void UCombatComponent::JumpToShotgunEnd()
{
	//jump to ShotgunEnd section in montage
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		/*AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");*/
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	//this will be called on all machines at the end of throwGernadeMontage
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}


void UCombatComponent::LaunchGrenade() //This is called on all machines from animation bp...so we will need to check if local or hasauthority when needed, which we do
{
	ShowAttachedGrenade(false);

	//need to make sure this is done on locally controlled characters
	if (Character && Character->IsLocallyControlled())
	{
		//HitTarget is calculated every frame in tick component...which is setup to do calculation on on locally controlled characters
		//so we only call ServerLaunchGrenade when we are locally controlled
		ServerLaunchGrenade(HitTarget); 
	}
	

	//moved below code into ServerLaunchGrenade
	/*if (Character && Character->HasAuthority() && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = HitTarget - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);
		}
	}*/
}


void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();		//this will update hud on the server...repnotify will do on clients
	}
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}



void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& target)
{
	
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);
		}
	}
}



//set to run on both client and server
void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
	}
	
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr)
	{
		return 0;
	}

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);	//clamp is not neccessary, but it helps to avoid negative values

		//CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	return 0;
}

//executed locally
void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0)
	{
		return;
	}
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr )
	{
		return;
	}

	//can be called on server controlled character or on client but server wont know about it
	CombatState = ECombatState::ECS_ThrowingGrenade; //replicated variables are replicated only when their value changes... 

	//play montage right away ... for player that is throwing
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();  //this is happening locally, so wont happen on server if calling from client
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	if(Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();	//now call on server...
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades); //only set on server... this will trigger OnRep_Gernades
		UpdateHUDGrenades();
	}

}
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0)	//make sure player is not cheating
	{
		return;
	}
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();  //this is happening on server, so wont happen on other clients
		AttachActorToLeftHand(EquippedWeapon);	//also need to do these things in OnRep_CombatState
		ShowAttachedGrenade(true);
	}

	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades); //only set on server... this will trigger OnRep_Gernades
	UpdateHUDGrenades();
}
void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}

}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}



void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr)
	{
		return;
	}

	//this compact line sets controller variable...after storing the controller we will not do the expensive cast again...
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else //if we dont have a weapon then dont set crosshairs
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			//calculate crosshair spread

			//if max speed is 600 ... [0,600] -> [0,1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, CrosshairInAirSpreadValue, DeltaTime, CrosshairInAirInterpSpeed);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bIsAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, CrosshairAimSpreadValue, DeltaTime, CrosshairAimInterpSpeed);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			if (HaveTarget)
			{
				CrosshairHaveTargetFactor = FMath::FInterpTo(CrosshairHaveTargetFactor, CrosshairHaveTargetSpreadValue, DeltaTime, CrosshairHaveTargetInterpSpeed);
			}
			else
			{
				CrosshairHaveTargetFactor = FMath::FInterpTo(CrosshairHaveTargetFactor, 0.f, DeltaTime, 30.f);
			}


			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread = 0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor -
				CrosshairHaveTargetFactor;

			HUD->SetFHUDPackage(HUDPackage);
		}
	}
}







void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2d ViewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);

	}

	FVector2d CrosshairLocation(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	//This turns the crosshair 2d position into a 3d world space positon...this will be the screen center
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;	//3d world position 

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 45.f);
			//DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red, false);


		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;	//straight out 80k units outward

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility); //line trace straight out 80k units

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
			HaveTarget = true;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
			HaveTarget = false;
		}


		//this is not in course, unless it is in future video...from comments in github
		if (!TraceHitResult.bBlockingHit) {
			TraceHitResult.ImpactPoint = End;
		}

		//if (!TraceHitResult.bBlockingHit)//nothing hit
		//{
		//	TraceHitResult.ImpactPoint = End;	//set the impactpoint at the end 
		//	HitTarget = End;
		//}
		//else
		//{
		//	HitTarget = TraceHitResult.ImpactPoint;
		//	//DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		//}

	}

}
void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		//this will zoom back to normal at the same speed for all weapons.
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bAiming)
{
	if (Character == nullptr || EquippedWeapon==nullptr)
	{
		return;
	}
	//if on client this will set righ away for them...so they see it with no delay...
	bIsAiming = bAiming;

	//Dont need to check for authority, but this will run on server
	//if (!Character->HasAuthority())
	//{
	ServerSetAiming(bAiming);
	//}

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}

	if (Character->IsLocallyControlled())	//SetAiming is only called locally but we can check here anyway
	{
		bAimButtonPressed = bAiming;
	}
	

}
//this sends out update to all other clients...
void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

