// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	//Super::Fire(HitTarget); dont call this...we want different behavior
	AWeapon::Fire(HitTarget); //but we do want what the parent of HitScaneWeapon (this classes grandparent)...ammo will update properly and will play animation

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return;
	}
	AController* InstigatorController = OwnerPawn->GetController();

	//leaving this in so this can be tested later, but we put in fix for this
	//if (!HasAuthority() && InstigatorController)
	//{
		///!!!! controllers only exist for players using them to control their pawn... 
		///!!!! InstigatorController will be null on all simulated proxies, so server will not see effect
		///!!!! so below take out the && InstigatorController check 
		///!!!! we check for authority futher down to apply damage
		/// 
	//	UE_LOG(LogTemp, Warning, TEXT("Instigator valid"));
	//}

	//need to do line trace for hitScan weapon
	//so first we need to get socket from weapon
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	//if (MuzzleFlashSocket && InstigatorController)
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		//UINT32 Hits = 0;

		TMap<ABlasterCharacter*, uint32>HitMap;

		//make random linetraces
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			//FVector End = TraceEndWithScatter(Start, HitTarget);	//this is in HitScanWeapon
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());	//i guess when using shotgun doing this for each pellet is not
																								//that much of a preformance impact...
			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				//Keep track of number of hits for each character hit
				//++Hits;
				if (HitMap.Contains(BlasterCharacter))	//already have this player in map
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1);	//new player hit
				}
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,			//should fix particle effect not always showing because of the 1.25f adjust above
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, .5f, FMath::FRandRange(-.5f, .5f));
			}
		}

		//loop through hit players
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				//get total number of hits on each character hit and multiply that by damage
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
				//}
			}
		}
		
	}

}

