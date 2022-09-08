#include "AliveTypes.h"


bool FDamageResult::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	OppositeLocation.NetSerialize(Ar, Map, bOutSuccess);
	Ar << Damage;
	// pack bitfield with flags
	uint8 Flags = 0 | (bBlock << 0) | (bCriticalHit << 1) | (bWeakness << 2);
	Ar.SerializeBits(&Flags, 8);
	bBlock = (Flags & (1 << 0)) ? 1 : 0;
	bCriticalHit = (Flags & (1 << 1)) ? 1 : 0;
	bWeakness = (Flags & (1 << 2)) ? 1 : 0;

	bOutSuccess = true;
	return true;
}
