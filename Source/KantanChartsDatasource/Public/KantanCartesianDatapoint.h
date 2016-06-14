// Copyright (C) 2015 Cameron Angus. All Rights Reserved.

#pragma once

#include "KantanCartesianDataPoint.generated.h"


USTRUCT(BlueprintType)
struct FKantanCartesianDatapoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FVector2D Coords;
};


