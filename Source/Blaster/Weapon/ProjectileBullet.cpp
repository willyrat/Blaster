// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

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
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController,this,UDamageType::StaticClass());
		}

	}
	

	//super needs to be called last because in the parent destory is called when MulticastSpawnImpactEffect() is called from in it
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);



}
