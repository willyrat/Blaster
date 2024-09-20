// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstanceController.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"


AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent")); //lesson 136
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);

}



void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	//only on client.. parent class does on server but rocket needs client side as well
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);		
	}

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

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false, 
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,	//we want a nullptr so we can set the last param...both are optional... this creates a nullptr of type USoundConcurrency
			false							//if we dont set this then it defaults to true
		);
	}
}

//Delay calling destroy 
void AProjectileRocket::DestroyTimerFinished()
{

	Destroy();
}



void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//lesson 136...
	// this doesnt help... rocket will not do hit so explosion does not happen...so rocket just hangs in air
	// see rocket collision box...keep moving when it hits something we dont want it to hit
	if (OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit self!!!"));
		return; //ignore hit
	}
	
	//returns the pawn that owns the weapon that fired the projectile
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())	//only applying damage on server
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this,						//World context oubject
															Damage,						//damage within the inner circle of blast radius
															10.f,						//the minimum amount of damage from furthest part of outer circle of blast radius
															GetActorLocation(),			//Origin
															InnerBlastRadius,			//Inner radius ...can make this variables for blueprint
															OuterBlastRadius,			//Outer radius ...can make this variables for blueprint
															1.f,						//Damage falloff ...1.f is linear	
															UDamageType::StaticClass(),	//Damage type
															TArray<AActor*>(),			//Actors to ignore... that will not take damage
															this,						//Damage causer
															FiringController			//Instigator controller
															);
		}		
	}
	
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectileRocket::DestroyTimerFinished, DestroyTime);
	
	//since we are overriding OnHit from Projectile.h, which calls destroy and then destroyed (code has been moved, see comments there)
	//so we call supper after we set things above
	// lesson 135 removes this to use timer
	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse,Hit);

	

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
	else if (HitType == EHitType::EHT_World)
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

	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		//TrailSystemComponent->GetSystemInstance()->Deactivate(); //has been deprecated use below line instead
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}

	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	//should not bee needed any more
	//MulticastSpawnImpactEffect(HitType, false);
}


void AProjectileRocket::Destroyed()
{
	//not calling super here on purpose
}