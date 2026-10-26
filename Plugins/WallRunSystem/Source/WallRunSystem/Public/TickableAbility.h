// Copyright KonekoSupremacy 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TickableAbility.generated.h"

UINTERFACE(MinimalAPI)
class UTickableAbility : public UInterface {
	GENERATED_BODY()
};

class WALLRUNSYSTEM_API ITickableAbility {
	GENERATED_BODY()

public:
	virtual void TickAbility(float DeltaSeconds) = 0;
};
