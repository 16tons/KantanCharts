// Copyright (C) 2015-2018 Cameron Angus. All Rights Reserved.

#include "KantanTimeSeriesPlotBase.h"

#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "KantanCharts"


UKantanTimeSeriesPlotBase::UKantanTimeSeriesPlotBase(FObjectInitializer const& OI) : Super(OI)
{
	bUseFixedTimeRange = true;
	DisplayTimeRange = 10.0f;
	bDisplayInMinutes = true;
	bDrawInReversedOrder = false;
	//bRoundTimeRange = false;
	LowerTimeBound.SetFitToData();
	UpperTimeBound.SetFitToData();
	LowerValueBound.SetFitToData();
	UpperValueBound.SetFitToData();
	RightLowerValueBound.SetFitToData();
	RightUpperValueBound.SetFitToData();
}

void UKantanTimeSeriesPlotBase::SetLowerTimeBound(FCartesianRangeBound InLowerBound)
{
	LowerTimeBound = InLowerBound;
	if (MyChart.IsValid())
	{
		GetTimeSeriesPlot()->SetLowerTimeBound(LowerTimeBound);
	}
}

void UKantanTimeSeriesPlotBase::SetUpperTimeBound(FCartesianRangeBound InUpperBound)
{
	UpperTimeBound = InUpperBound;
	if (MyChart.IsValid())
	{
		GetTimeSeriesPlot()->SetUpperTimeBound(UpperTimeBound);
	}
}

void UKantanTimeSeriesPlotBase::SetLowerValueBound(FCartesianRangeBound InLowerBound)
{
	LowerValueBound = InLowerBound;
	if (MyChart.IsValid())
	{
		GetTimeSeriesPlot()->SetLowerValueBound(LowerValueBound);
	}
}

void UKantanTimeSeriesPlotBase::SetUpperValueBound(FCartesianRangeBound InUpperBound)
{
	UpperValueBound = InUpperBound;
	if (MyChart.IsValid())
	{
		GetTimeSeriesPlot()->SetUpperValueBound(UpperValueBound);
	}
}

void UKantanTimeSeriesPlotBase::SetRightLowerValueBound(FCartesianRangeBound InLowerBound)
{
	RightLowerValueBound = InLowerBound;
	if (MyChart.IsValid())
	{
		GetTimeSeriesPlot()->SetRightLowerValueBound(RightLowerValueBound);
	}
}

void UKantanTimeSeriesPlotBase::SetRightUpperValueBound(FCartesianRangeBound InUpperBound)
{
	RightUpperValueBound = InUpperBound;
	if (MyChart.IsValid())
	{
		GetTimeSeriesPlot()->SetRightUpperValueBound(RightUpperValueBound);
	}
}

void UKantanTimeSeriesPlotBase::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	auto TimeSeriesPlot = GetTimeSeriesPlot();

	TimeSeriesPlot->SetFixedTimeRange(bUseFixedTimeRange ? TOptional< float >(DisplayTimeRange) : TOptional< float >());
	TimeSeriesPlot->SetDisplayInMinutes(bDisplayInMinutes);
	TimeSeriesPlot->SetDrawInReversedOrder(bDrawInReversedOrder);
	//TimeSeriesPlot->SetRoundTimeRange(bRoundTimeRange);
	TimeSeriesPlot->SetLowerTimeBound(LowerTimeBound);
	TimeSeriesPlot->SetUpperTimeBound(UpperTimeBound);
	TimeSeriesPlot->SetLowerValueBound(LowerValueBound);
	TimeSeriesPlot->SetUpperValueBound(UpperValueBound);
	TimeSeriesPlot->SetRightLowerValueBound(RightLowerValueBound);
	TimeSeriesPlot->SetRightUpperValueBound(RightUpperValueBound);
	TimeSeriesPlot->SetExtendValueRangeToZero(bExtendValueRangeToZero);
}

TSharedRef< SWidget > UKantanTimeSeriesPlotBase::RebuildWidget()
{
	MyChart = SNew(SKantanTimeSeriesPlot)
		.Style(&WidgetStyle);
	return MyChart.ToSharedRef();
}


#if WITH_EDITOR

UObject* UKantanTimeSeriesPlotBase::InternalGetPreviewDatasource() const
{
	return FindObject< UObject >(
		::GetTransientPackage(),
		TEXT("PreviewTimeSeriesPlotDatasource")
		);
}

void UKantanTimeSeriesPlotBase::InitPreview()
{
	auto TimeSeriesPlot = GetTimeSeriesPlot();
	auto NumSeries = TimeSeriesPlot->GetNumSeries();
	for (int32 Idx = 0; Idx < NumSeries; ++Idx)
	{
		auto Id = TimeSeriesPlot->GetSeriesId(Idx);
		TimeSeriesPlot->EnableSeries(Id, true);
		TimeSeriesPlot->ConfigureSeries(Id, false, true, false, false);
	}
}

#endif


#undef LOCTEXT_NAMESPACE


