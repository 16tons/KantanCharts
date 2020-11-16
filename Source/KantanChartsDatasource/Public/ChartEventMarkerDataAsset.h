// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KantanCartesianDatapoint.h"
#include "Styling/SlateBrush.h"
#include "ChartEventMarkerDataAsset.generated.h"


USTRUCT(BlueprintType)
struct KANTANCHARTSDATASOURCE_API FChartEventMarkerItemDefinition
{
	GENERATED_BODY()

	/** Specifies which brush to use for this marker*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSlateBrush Brush;

	/** Specifies which text to display for this marker*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayText;
};

UCLASS(BlueprintType)
class KANTANCHARTSDATASOURCE_API UChartEventMarkerDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	TMap<int32, FChartEventMarkerItemDefinition> ChartEventMarker;
	
};
