// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
//#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
//#include "NiagaraSystemInstanceController.h

//#include "TimerManager.h"

// Sets default values
AProjectile::AProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	//ECC_SkeletalMesh is defined in Blaster.h... which is set to ECC_GameTraceChannel1 which we defined in unreal project settings under object and created a new channel
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	//lesson 136 ... moving this into ProjectiileBullet
	//ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
																	Tracer,
																	CollisionBox,
																	FName(),		//used to attach to bone, but we dont want that for projectile
																	GetActorLocation(),
																	GetActorRotation(),
																	EAttachLocation::KeepWorldPosition
																);
	}

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}

	//lesson 136... rocket sometimes hits player when firing launcher
	//IgnoreActorWhenMoving() wont really help becuause the rocket is hitting too early for this to detect
	//CollisionBox->IgnoreActorWhenMoving();  



}

void AProjectile::ExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // World context object
				Damage, // BaseDamage
				10.f, // MinimumDamage
				GetActorLocation(), // Origin
				DamageInnerRadius, // DamageInnerRadius
				DamageOuterRadius, // DamageOuterRadius
				1.f, // DamageFalloff
				UDamageType::StaticClass(), // DamageTypeClass
				TArray<AActor*>(), // IgnoreActors
				this, // DamageCauser
				FiringController // InstigatorController
			);
		}
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//this only runs on server because of authorty check in BeginPlay
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
	
	//if (TracerComponent)
	//{
	//	TracerComponent->SetVisibility(false); // If the component supports visibility
	//	TracerComponent->Deactivate(); // Stop the tracer effect
	//	TracerComponent->SetActive(false); // Stop the tracer effect
	//	TracerComponent->DestroyComponent(); // Stop the tracer effect
	//	// Alternatively, you can use:


	//}
	// 
	//if (HasAuthority())
	//{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);

	HitType = GetHitType(OtherActor);

	if (HitType == EHitType::EHT_Player)
	{
		//check to make sure particles for this type are set
		if (PlayerImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerImpactParticles, GetActorTransform());
		}

		if (PlayerImpactSound)
		{
			//Need to make player hit sound property to set in blueprint
			UGameplayStatics::PlaySoundAtLocation(this, PlayerImpactSound, GetActorLocation());
		}
	}


	if (HitType == EHitType::EHT_World)
	{
		//check to make sure particles for this type are set
		if (WorldImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WorldImpactParticles, GetActorTransform());
		}

		if (WorldImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, WorldImpactSound, GetActorLocation());
		}
	}

	MulticastSpawnImpactEffect(HitType, true);

		//FTimerHandle TimerHandle;
		//GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AProjectile::DelayDestroy, 0.1f, false);
		//Destroy();
	//}
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

EHitType AProjectile::GetHitType(AActor* OtherActor)
{
	//if (OtherActor && OtherActor->Implements<UInteractWithCrosshairsInterface>())
	if (OtherActor && OtherActor->GetClass()->ImplementsInterface(UInteractWithCrosshairsInterface::StaticClass()))
	{
		HitType = EHitType::EHT_Player;

		/*if (BlasterCharacter)
		{
			BlasterCharacter->MulticastHit();
		}*/
	}
	else
	{
		HitType = EHitType::EHT_World;
	}

	return HitType;
}

//replacing this with multicast call below...it is supposed to be more immediate 
//void AProjectile::OnRep_HitType()
//{
//	UE_LOG(LogTemp, Warning, TEXT("HitType %d"), HitType);
//}

void AProjectile::MulticastSpawnImpactEffect_Implementation(EHitType TargetHitType, bool CallDestroy)
{

	// Hide only the tracer component
	//if (TracerComponent)
	//{
	//	TracerComponent->SetVisibility(false); // If the component supports visibility
	//	TracerComponent->Deactivate(); // Stop the tracer effect
	//	TracerComponent->SetActive(false); // Stop the tracer effect
	//	TracerComponent->DestroyComponent(); // Stop the tracer effect
	//	// Alternatively, you can use:
	//	
	//}

	if (TargetHitType == EHitType::EHT_Player)
	{
		//check to make sure particles for this type are set
		if (PlayerImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerImpactParticles, GetActorTransform());
		}

		if (PlayerImpactSound)
		{
			//Need to make player hit sound property to set in blueprint
			UGameplayStatics::PlaySoundAtLocation(this, PlayerImpactSound, GetActorLocation());
		}
	}


	if (TargetHitType == EHitType::EHT_World)
	{
		//check to make sure particles for this type are set
		if (WorldImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WorldImpactParticles, GetActorTransform());
		}

		if (WorldImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, WorldImpactSound, GetActorLocation());
		}
	}
	
	if (CallDestroy)
	{
		Destroy();
	}

}

//void AProjectile::DelayDestroy()
//{
//	Destroy();
//}


void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectile::DestroyTimerFinished, DestroyTime);
}

//Delay calling destroy 
void AProjectile::DestroyTimerFinished()
{

	Destroy();
}


//because this object is replicated when we call destroy in OnHit it will call Destroyed on server and all client, 
//so we can use this instead of replicating Hit info to everyone.  we can just have each client and server play particles and sounds
//keeps network traffic down
//!!!!MOVED CODE INTO MulticastSpawnImpactEffect_Implementation
void AProjectile::Destroyed()
{
	Super::Destroyed();
		
	/*if (PlayerImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerImpactParticles, GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}*/

}

//void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	// Replicate HitType
//	DOREPLIFETIME(AProjectile, HitType);	
//}

