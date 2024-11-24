// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//lesson 202 take this check out
	//if (!HasAuthority()) return;	//we check to if we are on the server and only run when on server...so this is safe to call on client, nothing will happen

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();

	if (MuzzleFlashSocket && World && InstigatorPawn)
	{		
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//from muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();		//gets owner of weapon which sets its owner to Character
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;


		//we are not using server side rewind for rockets or gernades, that is a challange
		if (bUseServerSideRewind) 
		{
			if (InstigatorPawn->HasAuthority())
			{
				if (InstigatorPawn->IsLocallyControlled())	//Server, host -  user replicated projectile
				{
					//spawn projectile
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;	//no longer setting damage on the projectile blueprint or in the projectile class... now setting to weapon's that is firing damage
					//lesson 219
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				}
				else //Server, not locally controlled - spawn non-replicated projectile, SSR
				{
					if (ServerSideRewindProjectileClass == nullptr) //Check if set in blueprint...this is to fix bug where high lag cuases bUseServerSideRewind
					{												//to change and rocket launcher and grenade launcher are not setup to use ServerSideRewindProjectileClass
						if (InstigatorPawn->HasAuthority())			//we still need to shoot something so shoot normal projectile, other wise it uses ProjecileBullet
						{
							//in this case, all server characters are going to spawn replicated projectiles
							SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
							SpawnedProjectile->bUseServerSideRewind = false;
							SpawnedProjectile->Damage = Damage;	//no longer setting damage on the projectile blueprint or in the projectile class... now setting to weapon's that is firing damage

						}
					}
					else
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						SpawnedProjectile->bUseServerSideRewind = true; //should not cause damage on server
					}
				}
			}
			else //client, using SSR
 			{
				if (ServerSideRewindProjectileClass == nullptr) //Check if set in blueprint...this is to fix bug where high lag cuases bUseServerSideRewind
				{												//to change and rocket launcher and grenade launcher are not setup to use ServerSideRewindProjectileClass
					if (InstigatorPawn->HasAuthority())			//we still need to shoot something so shoot normal projectile, other wise it uses ProjecileBullet
					{
						//in this case, all server characters are going to spawn replicated projectiles
						SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						SpawnedProjectile->bUseServerSideRewind = false;
						SpawnedProjectile->Damage = Damage;	//no longer setting damage on the projectile blueprint or in the projectile class... now setting to weapon's that is firing damage

					}
				}
				else if (InstigatorPawn->IsLocallyControlled()) //Client - locally controlled - spawn non-replicated projectile, use SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					SpawnedProjectile->Damage = Damage;	//no longer setting damage on the projectile blueprint or in the projectile class... now setting to weapon's that is firing damage
				}
				else //client - not locally controlled...spawn non-replicated projectile, no SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind  = false;					                   
				}				
			}
		}
		else //weapon not using SSR
		{
			if (InstigatorPawn->HasAuthority())
			{
				//in this case, all server characters are going to spawn replicated projectiles
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;	//no longer setting damage on the projectile blueprint or in the projectile class... now setting to weapon's that is firing damage
				SpawnedProjectile->HeadShotDamage = HeadShotDamage;
			}
		}
	}
}
 