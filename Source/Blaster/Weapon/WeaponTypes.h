#pragma once


//This is just a header file. there is no WeaponTypes.cpp file.

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssultRifle UMETA(DisplayName = "Assult Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_MAX UMETA(DisplayName = "DefaultMax")

};


