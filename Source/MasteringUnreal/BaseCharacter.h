// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class MASTERINGUNREAL_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

//Custom Character Declarations
public:

	UPROPERTY(EditDefaultsOnly, Category = "Base Character")
		USpringArmComponent *SpringArm;

	UPROPERTY(EditDefaultsOnly, Category = "Base Character")
		UCameraComponent *Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Base Character")
		UArrowComponent *ForwardDirection;
	
	UPROPERTY(EditDefaultsOnly, Category = "Base Character")
		UParticleSystemComponent *ExplosionEffect;

	//property for determining whether the firing action has been initiated -- used for animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character")
		bool bIsFiring;

	//is character sprinting
	UPROPERTY(EditDefaultsOnly, Category = "Base Character")
		bool bIsSprinting;

	UPROPERTY(EditDefaultsOnly, Category = "Base Character")
		float MaxSprint;

	UPROPERTY(EditDefaultsOnly, Category = "Base Character")
		float BaseRunSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Base Character")
		float TimeToMaxSpeed;

		float CurrentSprint;
	//Input Bindings
	void MoveForward(float amount);
	void MoveRight(float amount);
	void RotateCamera(float amount);
	void ChangeCameraHeight(float amount);
	void SprintStart();
	void SprintEnd();
	//we use two function to handle the firing animation
	//so we can tie in to animations later
	UFUNCTION(BlueprintCallable, Category = "Base Character")
		void FireStart();
	UFUNCTION(BlueprintCallable, Category = "Base Character")
		void Fired();
	
};
