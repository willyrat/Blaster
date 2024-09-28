// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"


// HitScanWeapons will not fire projectile.  they use linetraces to see if they hit something.  So they register a hit pretty much instantaniously 


//Fire is called on all machines
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

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
	const USkeletalMeshSocket* MuzzleFlashSocket= GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	//if (MuzzleFlashSocket && InstigatorController)
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter && HasAuthority() && InstigatorController)
		{
			//we want to do the linetrace on all machines (which fire does),but only apply damage on the server
			//So inside an authority check we will apply damge
			//if ()	//moved to if above and added in && InstigatorController
			//{
			UGameplayStatics::ApplyDamage(
				BlasterCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
			//}
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
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}

		//FVector End = Start + (HitTarget - Start) * 1.25f;	//1.25 so end location will be just past the object we are looking at.
															//otherwise the end will be right on the oject's surface and there is a 50/50 
															//chance it will not register as hit
		
		//UWorld* World = GetWorld();
		//if (World)
		//{
		//	World->LineTraceSingleByChannel(
		//		FireHit,
		//		Start,
		//		End,
		//		ECollisionChannel::ECC_Visibility
		//	);
		//	FVector BeamEnd = End;		//Set to end of line trace...
		//	if (FireHit.bBlockingHit)
		//	{
		//		BeamEnd = FireHit.ImpactPoint;  //...unless we hit something then set end to impactpoint
		//		

		//	}
		//	if (BeamParticles)
		//	{
		//		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
		//			World,
		//			BeamParticles,
		//			SocketTransform
		//		);
		//		if (Beam)
		//		{
		//			Beam->SetVectorParameter(FName("Target"), BeamEnd);
		//		}
		//	}
		//}
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound,GetActorLocation());
		}
	}

}


void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	//FHitResult FireHit;
	UWorld* World = GetWorld();

	if (World)
	{
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;	//1.25 so end location will be just past the object we are looking at.
																																//otherwise the end will be right on the oject's surface and there is a 50/50 
																																//chance it will not register as hit

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;  //...unless we hit something then set end to impactpoint

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					TraceStart,
					FRotator::ZeroRotator

				);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}


FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();	 
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f,SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

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


