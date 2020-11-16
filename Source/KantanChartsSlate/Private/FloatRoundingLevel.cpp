// Copyright (C) 2015-2018 Cameron Angus. All Rights Reserved.

#include "FloatRoundingLevel.h"


FFloatRoundingLevel::FFloatRoundingLevel(int32 InPower, int32 InBase, int32 InDesiredRounding)
{
	Rounding.Base = InBase;
	Rounding.Power = InPower;

	if (InDesiredRounding == 0)
	{
		MultiplierValues.Add(1);
		MultiplierValues.Add(2);
		MultiplierValues.Add(5);
	}
	else
	{
		MultiplierValues.Add(InDesiredRounding);
	}

	SetMultiplierIndex(0);
}

FFloatRoundingLevel FFloatRoundingLevel::MakeFromMinimumStepSize(float InMinStep, int32 InBase)
{
	auto RL = FFloatRoundingLevel(0, InBase);

	while (RL.GetStepValue() > InMinStep)
	{
		RL.Decrease();
	}

	while (RL.GetStepValue() < InMinStep)
	{
		RL.Increase();
	}

	return RL;
}

void FFloatRoundingLevel::Increase()
{
	if (MultiplierIdx < MultiplierValues.Num() - 1)
	{
		SetMultiplierIndex(MultiplierIdx + 1);
	}
	else
	{
		SetMultiplierIndex(0);
		++Rounding.Power;
	}
}

void FFloatRoundingLevel::Decrease()
{
	if (MultiplierIdx > 0)
	{
		SetMultiplierIndex(MultiplierIdx - 1);
	}
	else
	{
		SetMultiplierIndex(MultiplierValues.Num() - 1);
		--Rounding.Power;
	}
}


