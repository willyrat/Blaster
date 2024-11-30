#pragma once

UENUM(BlueprintType)
enum class EMatchTypes : uint8
{
	EMT_DeathMatch UMETA(DisplayName = "Free For All"),
	EMT_TeamDeathMatch UMETA(DisplayName = "Teams"),
	EMT_CaptureTheFlagMatch UMETA(DisplayName = "Capture The Flag"),
	EMT_Max UMETA(DisplayName = "DefaultMAX")
};