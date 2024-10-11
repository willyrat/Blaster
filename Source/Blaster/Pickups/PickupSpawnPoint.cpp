// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"
#include "Pickup.h"

// Sets default values
APickupSpawnPoint::APickupSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
}

// Called when the game starts or when spawned
void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();	
	StartSpawnPickupTimer((AActor*)nullptr);	//c style casts are considered bad practice but here it is ok...needed a nullptr in the form of an AActor
												//this is used in the unreal engine code a lot, but you need to know when to use...
												//when casting from a child class to a parent class you would never use c style cast
												//it will perform the cast with no type cast checking so no safty involved...will do it whether it failed or not
												//in this case we are casting from a nullptr to a AActor nullptr so it should be safe.
	
}

void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);

		//store spawned pickup so we can bind to its destroy event and know when to spane a new pickup
		//you can see all the delgates(events) in actor.h towards the top... we are going to use FActorDestroyedSignature
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		if (HasAuthority() && SpawnedPickup)
		{			
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}

}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	//check for authorty, only done because this actor is replicated...
	if (HasAuthority())
	{
		SpawnPickup();
	}

}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax); //random time before we spawn a new pickup
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&APickupSpawnPoint::SpawnPickupTimerFinished,
		SpawnTime
	);

	
}

// Called every frame
void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

