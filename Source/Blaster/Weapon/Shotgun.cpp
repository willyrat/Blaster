// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"

// lesson 181 replaces fire() with FireShotgun

//void AShotgun::Fire(const FVector& HitTarget)
//{
//	//Super::Fire(HitTarget); dont call this...we want different behavior
//	AWeapon::Fire(HitTarget); //but we do want what the parent of HitScaneWeapon (this classes grandparent)...ammo will update properly and will play animation
//
//	APawn* OwnerPawn = Cast<APawn>(GetOwner());
//	if (OwnerPawn == nullptr)
//	{
//		return;
//	}
//	AController* InstigatorController = OwnerPawn->GetController();
//
//	//leaving this in so this can be tested later, but we put in fix for this
//	//if (!HasAuthority() && InstigatorController)
//	//{
//		///!!!! controllers only exist for players using them to control their pawn... 
//		///!!!! InstigatorController will be null on all simulated proxies, so server will not see effect
//		///!!!! so below take out the && InstigatorController check 
//		///!!!! we check for authority futher down to apply damage
//		/// 
//	//	UE_LOG(LogTemp, Warning, TEXT("Instigator valid"));
//	//}
//
//	//need to do line trace for hitScan weapon
//	//so first we need to get socket from weapon
//	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
//	//if (MuzzleFlashSocket && InstigatorController)
//	if (MuzzleFlashSocket)
//	{
//		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
//		FVector Start = SocketTransform.GetLocation();
//		//UINT32 Hits = 0;
//
//		TMap<ABlasterCharacter*, uint32>HitMap;
//
//		//make random linetraces
//		for (uint32 i = 0; i < NumberOfPellets; i++)
//		{
//			//FVector End = TraceEndWithScatter(Start, HitTarget);	//this is in HitScanWeapon
//			FHitResult FireHit;
//			WeaponTraceHit(Start, HitTarget, FireHit);
//
//			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());	//i guess when using shotgun doing this for each pellet is not
//																								//that much of a preformance impact...
//			if (BlasterCharacter && HasAuthority() && InstigatorController)
//			{
//				//Keep track of number of hits for each character hit
//				//++Hits;
//				if (HitMap.Contains(BlasterCharacter))	//already have this player in map
//				{
//					HitMap[BlasterCharacter]++;
//				}
//				else
//				{
//					HitMap.Emplace(BlasterCharacter, 1);	//new player hit
//				}
//			}
//
//			if (ImpactParticles)
//			{
//				UGameplayStatics::SpawnEmitterAtLocation(
//					GetWorld(),
//					ImpactParticles,
//					FireHit.ImpactPoint,			//should fix particle effect not always showing because of the 1.25f adjust above
//					FireHit.ImpactNormal.Rotation()
//				);
//			}
//			if (HitSound)
//			{
//				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, .5f, FMath::FRandRange(-.5f, .5f));
//			}
//		}
//
//		//loop through hit players
//		for (auto HitPair : HitMap)
//		{
//			if (HitPair.Key && HasAuthority() && InstigatorController) //only execute on server
//			{
//				//get total number of hits on each character hit and multiply that by damage
//				UGameplayStatics::ApplyDamage(
//					HitPair.Key,
//					Damage * HitPair.Value,
//					InstigatorController,
//					this,
//					UDamageType::StaticClass()
//				);
//				//}
//			}
//		}
//		
//	}
//
//}

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());	//but we do want what the parent of HitScaneWeapon (this classes grandparent)...ammo will update properly and will play animation
								//lesson 181... can pass in empty FVector since AWeapon::Fire does not actually use it

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
	
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		//Maps hit character to number of times hit
		TMap<ABlasterCharacter*, uint32> HitMap;		

		//UE_LOG(LogTemp, Log, TEXT("going into for loop"));

		for (const FVector_NetQuantize HitTarget : HitTargets)
		{
			//UE_LOG(LogTemp, Log, TEXT("Start values are X: %f, Y: %f, Z: %f"), Start.X, Start.Y, Start.Z);
			//UE_LOG(LogTemp, Log, TEXT("HitTarget values are X: %f, Y: %f, Z: %f"), HitTarget.X, HitTarget.Y, HitTarget.Z);
			FHitResult FireHit;
			//UE_LOG(LogTemp, Log, TEXT("in AShotgun::FireShotgun about to go into WeaponTraceHit"));
			/*if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Yellow,
					FString(TEXT("going into WeaponTraceHit"))
				);
			}*/
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());	//i guess when using shotgun doing this for each pellet is not
																								//that much of a preformance impact...
			if (BlasterCharacter)
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
		}

		TArray<ABlasterCharacter*> HitCharacters;

		//loop through hit players
		for (auto HitPair : HitMap)
		{			
			if (HitPair.Key && InstigatorController) //only execute on server
			{
				//ON SERVER AND NOT USING SERVER SIDE REWIND				
				//if (HasAuthority() && !bUseServerSideRewind) //i am changing this and putting in future code below so shotgun will do damage when playing as server
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					//get total number of hits on each character hit and multiply that by damage
					UGameplayStatics::ApplyDamage(
						HitPair.Key,				//character that was hit
						Damage * HitPair.Value,		//multiply damage by number of times hit
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);

				}

				HitCharacters.Add(HitPair.Key);

			}
		}

		//ON CLIENT AND USING SERVER SIDE REWIND		
		if (!HasAuthority() && bUseServerSideRewind)
		{
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
			BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
			if (BlasterOwnerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
			{
				BlasterOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime					
				);
			}
		}

	}
}



void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	//if (MuzzleFlashSocket && InstigatorController)
	if (MuzzleFlashSocket == nullptr)	
	{
		return;
	}
	//UE_LOG(LogTemp, Log, TEXT("in AShotgun::ShotgunTraceEndWithScatter"));
	//it is good practice to const correct variables you know will not be changing...
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		//ToEndLoc = FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}

}


