// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMatineeCameraShake.h"

UMyMatineeCameraShake::UMyMatineeCameraShake()
{
    OscillationDuration = 2.0f; 
    OscillationBlendInTime = 0.2f; 
    OscillationBlendOutTime = 0.3f; 

    RotOscillation.Pitch.Amplitude = FMath::RandRange(5.0f, 10.0f); 
    RotOscillation.Pitch.Frequency = FMath::RandRange(10.0f, 20.0f); 

    RotOscillation.Yaw.Amplitude = FMath::RandRange(5.0f, 10.0f); 
    RotOscillation.Yaw.Frequency = FMath::RandRange(10.0f, 20.0f); 

    LocOscillation.Z.Amplitude = FMath::RandRange(5.0f, 10.0f); 
    LocOscillation.Z.Frequency = FMath::RandRange(10.0f, 20.0f); 

}