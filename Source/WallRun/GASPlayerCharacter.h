// Copyright: KonekoSupremacy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "PlayerCharacter.h"
#include "GASPlayerCharacter.generated.h"

// Forward Declarations
class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS(abstract)
class WALLRUN_API AGASPlayerCharacter : public APlayerCharacter, public IAbilitySystemInterface {
	GENERATED_BODY()

public:
	//================================================================================================================
	// FUNCTIONS
	//================================================================================================================
	AGASPlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual bool CanMove() const override;
	virtual void DoJump() override;

	//================================================================================================================
	// PROPERTIES & VARIABLES
	//================================================================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

protected:
	//================================================================================================================
	// FUNCTIONS
	//================================================================================================================
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void InitializeAbilitySystem();
	void GrantDefaultAbilities();
	void ActivateDefaultAbilities();

	//================================================================================================================
	// PROPERTIES & VARIABLES
	//================================================================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	bool bAbilitiesGranted = false;
};
