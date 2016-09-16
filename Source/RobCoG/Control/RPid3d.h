#pragma once
#include "EngineUtils.h"
#include "RPid3d.generated.h"

/*
3D PID controller implementation (for 3D vectors)

Error: where you want to be vs. where you are
This is certainly a big factor. If you are at point A and
your target is at point B, then the vector from A to B
tells you a lot about how you need to steer, but it isn't the only factor.

Derivative: how fast you are approaching
If you are approaching the target quickly and you are close to it,
you need to slow down. The derivative helps take that into consideration.

Integral: alignment error
Your robot may not actually do exactly what you tell it to do.
The integral helps determine how much you need to compensate for that.
*/
USTRUCT()
struct FRPid3d
{
	GENERATED_USTRUCT_BODY()

	// Constructor
	FRPid3d(float _P = 0.0f, float _I = 0.0f, float _D = 0.0f,
		float _OutMax = 0.0, float _OutMin = 0.0f) :
		P(_P), I(_I), D(_D), OutMax(_OutMax), OutMin(_OutMin)
	{
		IErr = FVector(0.0f);
		PrevErr = FVector(0.0f);
	};

	// Destructor
	~FRPid3d()
	{
	};

	// Initialise all PID values
	void Init(float _P = 0.0f, float _I = 0.0f, float _D = 0.0f,
		float _OutMax = 0.0, float _OutMin = 0.0f)
	{
		P = _P;
		I = _I;
		D = _D;
		OutMax = _OutMax;
		OutMin = _OutMin;

		FRPid3d::Reset();
	};

	// Update the PID loop
	// Error is the difference between the current state and the target
	// DeltaT is the change in time since the last update
	FVector Update(const FVector Error, const float DeltaT)

	{
		if (DeltaT == 0.0f)
		{
			return FVector(0.0f);
		}
		
		// Calculate proportional output
		const FVector POut = P * Error;

		// Calculate integral error / output
		IErr += DeltaT * Error;
		const FVector IOut = I * IErr;

		// Calculate the derivative error / output
		const FVector DErr = (Error - PrevErr) / DeltaT;
		const FVector DOut = D * DErr;

		// Set previous error
		PrevErr = Error;

		// Calculate Output
		const FVector Out = POut + IOut + DOut;

		// If Out>Outmax --> return Outmax, else if Out<Outmin --> return Outmin, else return Out
		const FVector ClampedOut = FVector(
			(Out.X > OutMax) ? (OutMax) : (Out.X < OutMin) ? (OutMin) : (Out.X),
			(Out.Y > OutMax) ? (OutMax) : (Out.X < OutMin) ? (OutMin) : (Out.Y),
			(Out.Z > OutMax) ? (OutMax) : (Out.X < OutMin) ? (OutMin) : (Out.Z)
		);

		return ClampedOut;
	};

	// Reset all values of the PID
	void Reset()
	{
		IErr = FVector(0.0f);
		PrevErr = FVector(0.0f);
	};

	// Proportional gain
	float P;

	// Integral gain
	float I;

	// Derivative gain
	float D;

	// Output maximul clamping value
	float OutMax;

	// Output minimum claming value
	float OutMin;

	// Previous step error value
	FVector PrevErr;

	// Integral error
	FVector IErr;
};