// Copyright KonekoSupremacy 2026 All Rights Reserved.

// Game Includes
#include "GASPlayerCharacter.h"
#include "TickableAbility.h"
#include "WallRunGameplayTags.h"

// Engine Includes
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"

AGASPlayerCharacter::AGASPlayerCharacter() {
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
}

void AGASPlayerCharacter::Tick(const float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (AbilitySystemComponent == nullptr) {
		return;
	}

	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities()) {
		if (!Spec.IsActive()) {
			continue;
		}

		if (ITickableAbility* TickableAbility = Cast<ITickableAbility>(Spec.GetPrimaryInstance())) {
			TickableAbility->TickAbility(DeltaSeconds);
		}
	}
}

void AGASPlayerCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();
	GrantDefaultAbilities();
}

UAbilitySystemComponent* AGASPlayerCharacter::GetAbilitySystemComponent() const {
	return AbilitySystemComponent;
}

bool AGASPlayerCharacter::CanMove() const {
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(WallRunGameplayTags::Status_WallRunning)) {
		return false;
	}
	return true;
}

void AGASPlayerCharacter::DoJump() {
	if (AbilitySystemComponent &&
		(AbilitySystemComponent->HasMatchingGameplayTag(WallRunGameplayTags::Status_WallRunning) ||
		 AbilitySystemComponent->HasMatchingGameplayTag(WallRunGameplayTags::Status_WallJumping))) {
		return;
	}

	Super::DoJump();
}

void AGASPlayerCharacter::InitializeAbilitySystem() {
	if (AbilitySystemComponent == nullptr) {
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void AGASPlayerCharacter::GrantDefaultAbilities() {
	if (AbilitySystemComponent == nullptr || bAbilitiesGranted) {
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities) {
		if (AbilityClass) {
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass));
		}
	}

	bAbilitiesGranted = true;
}

void AGASPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	ActivateDefaultAbilities();
}

void AGASPlayerCharacter::ActivateDefaultAbilities() {
	if (AbilitySystemComponent == nullptr) {
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities) {
		if (AbilityClass) {
			AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
		}
	}
}
