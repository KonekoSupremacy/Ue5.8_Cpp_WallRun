// Copyright KonekoSupremacy 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WallRunInputProvider.generated.h"

class UInputAction;

UINTERFACE(MinimalAPI)
class UWallRunInputProvider : public UInterface {
	GENERATED_BODY()
};

/** implement on your character so UGA_WallRun can find its jump input */
class WALLRUNSYSTEM_API IWallRunInputProvider {
	GENERATED_BODY()

public:
	virtual UInputAction* GetWallRunJumpAction() const = 0;
};
