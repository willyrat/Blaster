// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);	//BLUEPRINT CAN OVERRIDE THIS SO CHECK IN BLUEPRINT THAT THIS IS SET

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	//WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent); 
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	//if (GetLocalRole() == ENetRole::ROLE_Authority)
	// 
	//if (HasAuthority())	//same as above //take this check out in lesson 178
	//{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	//}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}



// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->WakeAllRigidBodies();
	}*/

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}



//this happens on all machines
void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation,false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());
			
			UWorld* World = GetWorld();
			if (World)
			{	
				// Get the current rotation from the SocketTransform
				FRotator Rotation = SocketTransform.GetRotation().Rotator();

				// Add random values to pitch, yaw, and roll
				Rotation.Pitch += FMath::RandRange(CasingEjectPitchMin, CasingEjectPitchMax);
				Rotation.Yaw += FMath::RandRange(CasingEjectYawMin, CasingEjectYawMax);
				Rotation.Roll += FMath::RandRange(CasingEjectRollMin, CasingEjectRollMax);

				// Spawn the actor with the modified rotation
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					Rotation
				);				
			}
		}
	}
	if(HasAuthority())	//lesson 177... only do this on server, but this could cause delay in updating hud on client...
	{
		SpendRound();
	}
}



void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}


void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;

	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}
//this is only called on server...
void AWeapon::SpendRound() //this gets called when player fires weapon
{
	//this is replicated which will trigger OnRep_Ammo
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);	
	SetHUDAmmo();
}
void AWeapon::OnRep_Ammo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombat() && IsFull())
	{
		BlasterOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : BlasterOwnerCharacter;
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
		
	}
}


//this gets run on server...which triggers the OnRep_WeaponState below... after OnWeaponStateSet, added in lesson 172
void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}
void AWeapon::OnWeaponStateSet() //lesson 172
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:		
		OnEquipped();
		//EnableCustomDepth(false); //removed in lesson 171
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();		
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}
//This runs on cleint (runs things that did not propagate like showpickupwidget
void AWeapon::OnRep_WeaponState()
{

	OnWeaponStateSet(); //lesson 172 replaces below with this line... we made sure server/client calls were setup properly

	//dont need to worry about the AreaSphere on client
	//switch (WeaponState)
	//{
	//case EWeaponState::EWS_Equipped:
	//	ShowPickupWidget(false);
	//	WeaponMesh->SetSimulatePhysics(false);
	//	WeaponMesh->SetEnableGravity(false);
	//	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	//	{
	//		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);	//for strap
	//		WeaponMesh->SetEnableGravity(true);	//for strap  ... if this is set to false the strap may look like it is moving under water
	//		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//	}

	//	//EnableCustomDepth(false);//removed in lesson 171
	//	break;
	//case EWeaponState::EWS_Dropped:		
	//	WeaponMesh->SetSimulatePhysics(true);
	//	WeaponMesh->SetEnableGravity(true);
	//	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);		
	//	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	//	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	//	WeaponMesh->MarkRenderStateDirty();
	//	EnableCustomDepth(true);
	//	break;
	//}	
}
void AWeapon::OnEquipped()	//lesson 172 //this gets called on server and client
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//this is not needed on client but does not hurt anything if it is called on client
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);	//for strap
		WeaponMesh->SetEnableGravity(true);	//for strap ... if this is set to false the strap may look like it is moving under water
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);
}
void AWeapon::OnEquippedSecondary()	//lesson 172 //this gets called on server and client
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//this is not needed on client but does not hurt anything if it is called on client
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);	//for strap
		WeaponMesh->SetEnableGravity(true);	//for strap ... if this is set to false the strap may look like it is moving under water
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);
	/*
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}*/
	
}
void AWeapon::OnDropped() //lesson 172 //this gets called on server and client
{
	if (HasAuthority())	//check hasAuthority so it does not run on client...
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}


void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);

	}
}


bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	//if (MuzzleFlashSocket && InstigatorController)
	if (MuzzleFlashSocket == nullptr)
	{
		return FVector();
	}

	//it is good practice to const correct variables you know will not be changing...
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);	//for setup and debug
	//DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);	//for setup and debug
	//DrawDebugLine(
	//	GetWorld(),
	//	TraceStart,
	//	FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
	//	FColor::Cyan,
	//	true
	//);


	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());

}
