// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMyProject2Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"


//////////////////////////////////////////////////////////////////////////
// AMyProject2Character

AMyMyProject2Character::AMyMyProject2Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)



}

//////////////////////////////////////////////////////////////////////////
// Input

void AMyMyProject2Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{

	// Set up gameplay key bindings
	check(PlayerInputComponent);

	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Привязка действия к нажатию и отпусканию клавиши Shift
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyMyProject2Character::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyMyProject2Character::StopSprinting);

	// Привязка оси движения вперед/назад к функции MoveForward
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMyMyProject2Character::MoveForward);
	// Привязка оси движения вправо/влево к функции MoveRight

	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMyMyProject2Character::MoveRight);


	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMyMyProject2Character::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMyMyProject2Character::StopCrouching);


	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMyMyProject2Character::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMyMyProject2Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMyMyProject2Character::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AMyMyProject2Character::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMyMyProject2Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMyMyProject2Character::TouchStopped);
}

void AMyMyProject2Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AMyMyProject2Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AMyMyProject2Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMyMyProject2Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMyMyProject2Character::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyMyProject2Character::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AMyMyProject2Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CharacterVelocity = GetVelocity();
	float Speed = CharacterVelocity.Size();
	FString SpeedString = FString::Printf(TEXT("Speed: %.2f"), Speed);

	// Очистите предыдущие отладочные строки на экране
	GEngine->ClearOnScreenDebugMessages();

	// Выведите отладочное сообщение на экран
	float MessageTime = 5.0f; // Время отображения сообщения на экране (в секундах)
	FColor MessageColor = FColor::Red; // Цвет сообщения (красный)
	GEngine->AddOnScreenDebugMessage(-1, MessageTime, MessageColor, SpeedString);
}

void AMyMyProject2Character::StartSprinting()
{
	// Установка скорости усиленного бега
	GetCharacterMovement()->MaxWalkSpeed *= 2.0f;
}

void AMyMyProject2Character::StopSprinting()
{
	// Восстановление обычной скорости движения
	GetCharacterMovement()->MaxWalkSpeed /= 2.0f;
}

void AMyMyProject2Character::StartCrouching()
{
	Crouch();
}

void AMyMyProject2Character::StopCrouching()
{
	UnCrouch();
	
}



void AMyMyProject2Character::Crouch(bool bClientSimulation /*= false*/)
{
	Super::Crouch(bClientSimulation);

	GetCharacterMovement()->MaxWalkSpeed = 250.f;

	// Дополнительные действия, которые вы хотите выполнить при приседании
}



void AMyMyProject2Character::CrouchAction()
{
	if (CanCrouch()) // Проверяем, может ли персонаж присесть
	{
		if (GetCharacterMovement()->IsCrouching()) // Проверяем, находится ли персонаж в состоянии приседа
		{
			UnCrouch(); // Если да, то вызываем функцию UnCrouch() для вставания из приседа
		}
		else
		{
			Crouch(); // Если нет, то вызываем функцию Crouch() для начала приседания
		}
	}
}
