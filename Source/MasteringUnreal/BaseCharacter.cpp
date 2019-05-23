// Fill out your copyright notice in the Description page of Project Settings.

#include "MasteringUnreal.h"
#include "BaseCharacter.h"


// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create spring arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));

	//Keep the spring arm from rotating with the character
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bAbsoluteRotation = true;

	//define the distance between the character and the camera
	SpringArm->TargetArmLength = 1000.f;

	//Attach the SpringArm to the RootComponent
	SpringArm->SetupAttachment(RootComponent);

	//Create camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	//Give it a larger FOV, of 110 degrees
	Camera->FieldOfView = 110.f;

	//Attach the camera to the SpringArm
	Camera->SetupAttachment(SpringArm);

	//Set up our forward arrow component
	ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("Forward Direction"));
	//Attach the arrow to the root component
	ForwardDirection->SetupAttachment(RootComponent);

	//turn off collision for our raycasts on our mesh.
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECollisionResponse::ECR_Ignore);

	//set default value for bIsFiring
	bIsFiring = false;

	//ensure our character is always facing the direction he's moving
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//Allows the character to walk up stairs
	GetCharacterMovement()->SetWalkableFloorAngle(50.f);
	GetCharacterMovement()->MaxStepHeight = 45.f;

	//Set up our explosion particle effect system
	ExplosionEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Explosion Effect"));
	//ensure it doesn't activate on startup
	ExplosionEffect->bAutoActivate = false;
	//Ensure we don't interrupt our raycast with our particle effect
	ExplosionEffect->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECollisionResponse::ECR_Ignore);


	//Jump tweaking
	JumpMaxHoldTime = 0.25f;
	//movement control while in the air
	GetCharacterMovement()->AirControl = 0.5f;

	//jump height
	GetCharacterMovement()->JumpZVelocity = 800.f;
	//Fall speed
	GetCharacterMovement()->GravityScale = 2.0f;

	//Support double jumps
	JumpMaxCount = 2;

	//Set walking parameters
	//not sprinting by default
	bIsSprinting = false;
	
	//default run speed
	BaseRunSpeed = 600.f;

	//set our default maxsprint to 2x normal speed
	MaxSprint = 2.0f;

	//CurrentSprint starts at 1x multiple of base speed
	CurrentSprint = 1.0f;

	//Set our MaxWalkSpeed in our movement component to our base run speed
	GetCharacterMovement()->MaxWalkSpeed = BaseRunSpeed;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//Set the arm behind the character and a little off the ground
	SpringArm->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, -25, 0)));

	//Activate the camera on start
	Camera->Activate();
}

// Called every frame
void ABaseCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	//how much speed to increase this frame
	float rampThisFrame = (DeltaTime / TimeToMaxSpeed) * MaxSprint;

	if (bIsSprinting) {
		CurrentSprint += rampThisFrame;
	}
	else {
		CurrentSprint -= rampThisFrame;
	}

	//Clamp our CurrentSprint between 1 and MaxSprint
	CurrentSprint = FMath::Clamp(CurrentSprint, 1.f, MaxSprint);

	//Apply the movement change to the character
	GetCharacterMovement()->MaxWalkSpeed = BaseRunSpeed * CurrentSprint;
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Bind action mappings
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);
	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ABaseCharacter::FireStart);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &ABaseCharacter::SprintStart);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &ABaseCharacter::SprintEnd);

	//bind the axis mappings to our function
	InputComponent->BindAxis("ChangeCameraHeight", this, &ABaseCharacter::ChangeCameraHeight);
	InputComponent->BindAxis("RotateCamera", this, &ABaseCharacter::RotateCamera);
	InputComponent->BindAxis("MoveForward", this, &ABaseCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABaseCharacter::MoveRight);

}

void ABaseCharacter::MoveForward(float amount)
{
	//Ensure our character controller is valid and that our amount is not 0
	if (Controller && amount) {
		//add movement in the camera facing direction
		AddMovementInput(SpringArm->GetForwardVector(), amount);
	}
}

void ABaseCharacter::MoveRight(float amount)
{
	//Ensure our character controller is valid and that our amount is not 0
	if (Controller && amount) {
		// add movement in the camera 'right' direction
		AddMovementInput(SpringArm->GetRightVector(), amount);
	}
}

void ABaseCharacter::RotateCamera(float amount)
{
	//Ensure our character controller is valid and that our amount is not 0
	if (Controller && amount) {
		//Get the current spring arm rotation as a vector
		FVector rot = SpringArm->GetComponentRotation().Euler();

		//add the amount to the vector
		rot += FVector(0, 0, amount);

		//Apply the new rotation to the springarm
		SpringArm->SetWorldRotation(FQuat::MakeFromEuler(rot));
	}
}

void ABaseCharacter::ChangeCameraHeight(float amount)
{
	//Ensure our character controller is valid and that our amount is not 0
	if (Controller && amount) {
		//Get current rotation as vector
		FVector rot = SpringArm->GetComponentRotation().Euler();

		//separate the height out into its own variable
		float newHeight = rot.Y;
		//add the desired rotation
		newHeight += amount;
		//clamp the height to comfortable levels
		newHeight = FMath::Clamp(newHeight, -45.f, -5.f);

		//rebuild the rotation vector
		rot = FVector(0, newHeight, rot.Z);

		//apply the new rotation vector to the spring arm
		SpringArm->SetWorldRotation(FQuat::MakeFromEuler(rot));
	}
}

void ABaseCharacter::SprintStart()
{
	bIsSprinting = true;
}

void ABaseCharacter::SprintEnd()
{
	bIsSprinting = false;
}

void ABaseCharacter::FireStart()
{
	//make sure that we're not already firing
	if (!bIsFiring) {
		bIsFiring = true;
	}
}

void ABaseCharacter::Fired()
{
	//if we're not firing, return
	if (!bIsFiring) return;
	//tell the animation engine that we've eaten the call to Fired()
	bIsFiring = false;

	FVector HandLocation = GetMesh()->GetBoneLocation(FName("LeftHandMiddle1"));

	//check to see if an effect has been provided
	if (ExplosionEffect) {
		//put the effect at our hand location
		ExplosionEffect->SetWorldLocation(HandLocation);
		//Activate the effect, restarting the system
		ExplosionEffect->Activate(true);
	}
	//Set our raycast distance
	float distance = 10000;

	//get our start location, raycast direction and set our end location
	FVector direction = ForwardDirection->GetForwardVector();
	FVector start = HandLocation;
	FVector end = start + (direction * distance);

	//Declare our hit result
	FHitResult outHit;
	//do the raycast
	bool HasHitSomething = GetWorld()->LineTraceSingleByChannel(outHit, start, end, ECollisionChannel::ECC_Destructible);

	//choose a random color
	FColor color = FColor::MakeRandomColor();

	if (HasHitSomething) {
		//draw a line to the object
		DrawDebugLine(GetWorld(), start, outHit.ImpactPoint, color, true, 1.f, 0, 12.333);

		//check to make sure the hit actor is movable, return otherwise
		if (!outHit.GetActor()->IsRootComponentMovable()) return;

		//Create an array to hold any mesh components on the actor
		TArray<UStaticMeshComponent*> Components;

		//Get all the static mesh components on hit actor
		outHit.GetActor()->GetComponents<UStaticMeshComponent>(Components);

		//iterate through the array and blast away any found meshes
		for (auto &mesh : Components) {
			mesh->AddForce(direction * 100000000.f);
		}
	}
	else {
		DrawDebugLine(GetWorld(), start, end, color, true, 1.f, 0, 12.333);
	}
}

