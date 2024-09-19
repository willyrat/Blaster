// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//returns the pawn that owns the weapon that fired the projectile
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
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
	
	//since we are overriding OnHit from Projectile.h, which calls destroy and then destroyed (code has been moved, see comments there)
	//so we call supper after we set things above
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse,Hit);



}
