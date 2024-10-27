// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()  //this is needed because we moved these 2 lines from AProjectile...this is for lesson 136
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

//#if is a compilation condition so it can go here in header file.  with_editor will not be compiled into a packaged build, only for the editor
#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	//!!! THIS IS VERY USEFUL IF YOU HAVE A PROPERTY THAT CAN AFFECTED OTHER PROPERTIES WHEN IT CHANGES...CAN ADD FUNCTIONALITY TO BLUEPRINTS
	//Projectile::InitialSpeed is a UPROPERTY and unreal will track it's changes, so we can check it by name
	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			//Damage is in parent class
			//This triggers a damage event.  A class will need to bind a callback to that event
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());			
		}

	}
	

	//super needs to be called last because in the parent destory is called when MulticastSpawnImpactEffect() is called from in it
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);



}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	FPredictProjectilePathParams PathParms;
	PathParms.bTraceWithChannel = true;
	PathParms.bTraceWithCollision = true;
	PathParms.DrawDebugTime = 5.f;
	PathParms.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParms.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParms.MaxSimTime = 4.f;
	PathParms.ProjectileRadius = 5.f;
	PathParms.SimFrequency = 30.f;
	PathParms.StartLocation = GetActorLocation();
	PathParms.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParms.ActorsToIgnore.Add(this); //set to this bullet so we dont hit the projectile we are tracing for


	FPredictProjectilePathResult PathResults;

	UGameplayStatics::PredictProjectilePath(this, PathParms, PathResults);

}
