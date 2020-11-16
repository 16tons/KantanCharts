// Copyright (C) 2015-2018 Cameron Angus. All Rights Reserved.

#include "Charts/SKantanTimeSeriesPlot.h"
#include "FloatRoundingLevel.h"

#include <algorithm>
#include <cmath>
#include <limits>


void SKantanTimeSeriesPlot::Construct(const FArguments& InArgs)
{
	SKantanCartesianChart::Construct(
		SKantanCartesianChart::FArguments()
		.Style(InArgs._Style)
		.Datasource(InArgs._Datasource)
		.UpdateTickRate(InArgs._UpdateTickRate)
		);

	SetFixedTimeRange(30.0f);
	//SetRoundTimeRange(false);
	SetDisplayInMinutes(true);
	SetDrawInReversedOrder(false);
	LowerValueBound.SetFitToDataRounded();
	UpperValueBound.SetFitToDataRounded();
	RightLowerValueBound.SetFitToDataRounded();
	RightUpperValueBound.SetFitToDataRounded();
	LowerTimeBound.SetFitToDataRounded();
	UpperTimeBound.SetFitToDataRounded();
	SetOnUpdatePlotScale(FOnUpdatePlotScale::CreateSP(this, &SKantanTimeSeriesPlot::DeterminePlotScale));
}

void SKantanTimeSeriesPlot::SetFixedTimeRange(TOptional< float > InRange)
{
	FixedTimeRange = InRange;
	if(FixedTimeRange.IsSet())
	{
		const float MinimumFixedTimeRange = 1.0e-6f;	// @TODO: don't hard code
		FixedTimeRange = FMath::Max(FixedTimeRange.GetValue(), MinimumFixedTimeRange);
	}
}

void SKantanTimeSeriesPlot::SetDisplayInMinutes(bool bDisplayMinutes)
{
	bDisplayInMinutes = bDisplayMinutes;
}

void SKantanTimeSeriesPlot::SetDrawInReversedOrder(bool bReversed)
{
	bDrawInReversedOrder = bReversed;
}

/*
void SKantanTimeSeriesPlot::SetRoundTimeRange(bool bInRound)
{
	bRoundTimeRange = bInRound;
}
*/
void SKantanTimeSeriesPlot::SetLowerValueBound(FCartesianRangeBound const& InBound)
{
	LowerValueBound = InBound;
}

void SKantanTimeSeriesPlot::SetUpperValueBound(FCartesianRangeBound const& InBound)
{
	UpperValueBound = InBound;
}

void SKantanTimeSeriesPlot::SetRightLowerValueBound(FCartesianRangeBound const& InBound)
{
	RightLowerValueBound = InBound;
}

void SKantanTimeSeriesPlot::SetRightUpperValueBound(FCartesianRangeBound const& InBound)
{
	RightUpperValueBound = InBound;
}

void SKantanTimeSeriesPlot::SetLowerTimeBound(FCartesianRangeBound const& InBound)
{
	LowerTimeBound = InBound;
}

void SKantanTimeSeriesPlot::SetUpperTimeBound(FCartesianRangeBound const& InBound)
{
	UpperTimeBound = InBound;
}

void SKantanTimeSeriesPlot::SetExtendValueRangeToZero(bool bExtendToZero)
{
	bExtendValueRangeToZero = bExtendToZero;
}

int32 SKantanTimeSeriesPlot::DrawChartArea(
	EChartContentArea::Type Area,
	const FPaintArgs& Args,
	const FGeometry& Geometry,
	const FGeometry& PlotSpaceGeometry,
	const FSlateRect& MyClippingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled
) const
{
	// Used to track the layer ID we will return.
	int32 RetLayerId = LayerId;

	bool bEnabled = ShouldBeEnabled(bParentEnabled);
	const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	// Pre-snap the clipping rect to try and reduce common jitter, since the padding is typically only a single pixel.
	FSlateRect SnappedClippingRect = FSlateRect(FMath::RoundToInt(MyClippingRect.Left), FMath::RoundToInt(MyClippingRect.Top), FMath::RoundToInt(MyClippingRect.Right), FMath::RoundToInt(MyClippingRect.Bottom));

	if (PlotSpaceGeometry.GetLocalSize().X == 0 || PlotSpaceGeometry.GetLocalSize().Y == 0)
	{
		// @TODO: Bit of a cheap way out, avoiding some division by zero issues
		return RetLayerId;
	}

	switch (Area)
	{
	case EChartContentArea::XAxisBottomTitle:
		if (XAxisCfg.LeftBottomAxis.bEnabled && XAxisCfg.LeftBottomAxis.bShowTitle)
		{
			DrawXAxisTitle(Geometry, SnappedClippingRect, OutDrawElements, RetLayerId, XAxisCfg, GetCachedMarkerData(EAxis::X, PlotSpaceGeometry, bDisplayInMinutes));
		}
		break;
	case EChartContentArea::XAxisTopTitle:
		if (XAxisCfg.RightTopAxis.bEnabled && XAxisCfg.RightTopAxis.bShowTitle)
		{
			DrawXAxisTitle(Geometry, SnappedClippingRect, OutDrawElements, RetLayerId, XAxisCfg, GetCachedMarkerData(EAxis::X, PlotSpaceGeometry, bDisplayInMinutes));
		}
		break;
	case EChartContentArea::YAxisLeftTitle:
		if (YAxisCfg.LeftBottomAxis.bEnabled && YAxisCfg.LeftBottomAxis.bShowTitle)
		{
			DrawYAxisTitle(Geometry, SnappedClippingRect, OutDrawElements, RetLayerId, YAxisCfg, GetCachedMarkerData(EAxis::Y, PlotSpaceGeometry));
		}
		break;
	case EChartContentArea::YAxisRightTitle:
		if (YAxisCfg.RightTopAxis.bEnabled && YAxisCfg.RightTopAxis.bShowTitle)
		{
			DrawYAxisTitle(Geometry, SnappedClippingRect, OutDrawElements, RetLayerId, RightYAxisCfg, GetCachedMarkerData(EAxis::Y, PlotSpaceGeometry));
		}
		break;

	case EChartContentArea::XAxisBottom:
		if (XAxisCfg.LeftBottomAxis.bEnabled)
		{
			DrawFixedAxis(
				Geometry,
				SnappedClippingRect,
				OutDrawElements,
				RetLayerId,
				EAxis::X,
				AxisUtil::FAxisTransform::FromTransform2D(CartesianToPlotTransform(PlotSpaceGeometry), 0 /* X axis */),
				EChartAxisPosition::LeftBottom,
				GetCachedMarkerData(EAxis::X, PlotSpaceGeometry, bDisplayInMinutes),
				XAxisCfg.LeftBottomAxis.bShowMarkers,
				XAxisCfg.LeftBottomAxis.bShowLabels,
				ChartConstants::AxisMarkerLength,
				ChartConstants::AxisMarkerLabelGap,
				bDisplayInMinutes
			);
		}
		break;
	case EChartContentArea::XAxisTop:
		if (XAxisCfg.RightTopAxis.bEnabled)
		{
			DrawFixedAxis(
				Geometry,
				SnappedClippingRect,
				OutDrawElements,
				RetLayerId,
				EAxis::X,
				AxisUtil::FAxisTransform::FromTransform2D(CartesianToPlotTransform(PlotSpaceGeometry), 0 /* X axis */),
				EChartAxisPosition::RightTop,
				GetCachedMarkerData(EAxis::X, PlotSpaceGeometry, bDisplayInMinutes),
				XAxisCfg.RightTopAxis.bShowMarkers,
				XAxisCfg.RightTopAxis.bShowLabels,
				ChartConstants::AxisMarkerLength,
				ChartConstants::AxisMarkerLabelGap,
				bDisplayInMinutes
			);
		}
		break;
	case EChartContentArea::YAxisLeft:
		if (YAxisCfg.LeftBottomAxis.bEnabled)
		{
			DrawFixedAxis(
				Geometry,
				SnappedClippingRect,
				OutDrawElements,
				RetLayerId,
				EAxis::Y,
				AxisUtil::FAxisTransform::FromTransform2D(CartesianToPlotTransform(PlotSpaceGeometry), 1 /* Y axis */),
				EChartAxisPosition::LeftBottom,
				GetCachedMarkerData(EAxis::Y, PlotSpaceGeometry),
				YAxisCfg.LeftBottomAxis.bShowMarkers,
				YAxisCfg.LeftBottomAxis.bShowLabels,
				ChartConstants::AxisMarkerLength,
				ChartConstants::AxisMarkerLabelGap
			);
		}
		break;
	case EChartContentArea::YAxisRight:
		if (YAxisCfg.RightTopAxis.bEnabled)
		{
			DrawFixedAxis(
				Geometry,
				SnappedClippingRect,
				OutDrawElements,
				RetLayerId,
				EAxis::Z,
				AxisUtil::FAxisTransform::FromTransform2D(CartesianToPlotTransform(PlotSpaceGeometry, true), 1 /* Y axis */),
				EChartAxisPosition::RightTop,
				GetCachedMarkerData(EAxis::Z, PlotSpaceGeometry),
				YAxisCfg.RightTopAxis.bShowMarkers,
				YAxisCfg.RightTopAxis.bShowLabels,
				ChartConstants::AxisMarkerLength,
				ChartConstants::AxisMarkerLabelGap
			);
		}
		break;

	case EChartContentArea::Plot:
	{
		// Add 1 unit to right and bottom of clip rect for purposes of drawing axes
		const FSlateRect AxisClipRect = SnappedClippingRect.ExtendBy(FMargin(0, 0, 1, 1));
		//Geometry.GetRenderBoundingRect(FMargin(0, 0, 1, 1));
		OutDrawElements.PushClip(FSlateClippingZone(AxisClipRect));

		auto AxisLayer = RetLayerId;
		FPlotMarkerData PlotMarkerData;
		PlotMarkerData.XAxis = GetCachedMarkerData(EAxis::X, PlotSpaceGeometry, bDisplayInMinutes);
		PlotMarkerData.YAxis = GetCachedMarkerData(EAxis::Y, PlotSpaceGeometry);
		RetLayerId = DrawAxes(PlotSpaceGeometry, AxisClipRect, OutDrawElements, AxisLayer, AxisLayer + 2, PlotMarkerData);

		OutDrawElements.PopClip();

		// Inflate slightly to avoid clipping plot lines lying exactly along the edges of the plot area.
		// @NOTE: Bit random, but apparently 1.0 on the vertical is not sufficient to stop this.
		const FSlateRect DataClipRect = SnappedClippingRect.ExtendBy(FMargin(0.5f, 2.0f));
		//PlotSpaceGeometry.GetRenderBoundingRect(FMargin(0.5f, 2.0f));
		OutDrawElements.PushClip(FSlateClippingZone(DataClipRect));

		auto ChartStyle = GetChartStyle();
		auto NumSeries = GetNumSeries();

		if (bDrawInReversedOrder)
		{
			for (int32 Idx = NumSeries - 1; Idx >= 0; --Idx)
			{
				DrawSeriesAtIdx(Idx, PlotSpaceGeometry, DataClipRect, OutDrawElements, AxisLayer + 1);
			}
		}
		else
		{
			for (int32 Idx = 0; Idx < NumSeries; ++Idx)
			{
				DrawSeriesAtIdx(Idx, PlotSpaceGeometry, DataClipRect, OutDrawElements, AxisLayer + 1);
			}
		}

		OutDrawElements.PopClip();
	}
	break;
	}

	return RetLayerId;
}

int32 SKantanTimeSeriesPlot::DrawSeriesAtIdx(int32 Idx, const FGeometry& PlotSpaceGeometry, const FSlateRect& ClipRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	auto SeriesId = GetSeriesId(Idx);
	if (SeriesId.IsNone())
	{
		return LayerId;
	}

	auto const& Config = SeriesConfig[SeriesId];
	if (Config.bEnabled == false)
	{
		return LayerId;
	}

	// Don't render if no element is setup for this series
	if (SeriesElements.Contains(SeriesId) == false)
	{
		return LayerId;
	}

	const auto Points = GetSeriesDatapoints(Idx);
	const auto Markers = GetSeriesMarkers(Idx);
	auto const& SeriesStyle = GetSeriesStyle(SeriesId);

	// @TODO: Sort out layers, maybe need to separate out DrawAxes into DrawAxisLines and DrawAxisLabels
	return DrawSeries(PlotSpaceGeometry, ClipRect, OutDrawElements, LayerId, SeriesId, Points, SeriesStyle, Markers, Config.bUseRightYAxis);
}

void SKantanTimeSeriesPlot::OnActiveTick(double InCurrentTime, float InDeltaTime)
{
	SKantanCartesianChart::OnActiveTick(InCurrentTime, InDeltaTime);
}

void SKantanTimeSeriesPlot::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SKantanCartesianChart::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

FKantanCartesianPlotScale SKantanTimeSeriesPlot::DeterminePlotScale(const FCartesianDataSnapshot& Snapshot, const TArray< int32 >& Enabled) const
{
	// @TODO: for now just rounding to about 1 tenth of the range
	const float RoundingProportion = 0.1f;

	FKantanCartesianPlotScale NewPlotScale;
	NewPlotScale.Type = ECartesianScalingType::FixedRange;
	
	// Y (value) axis
	{
		TOptional< FCartesianAxisRange > DataYSpan;
		FCartesianAxisRange YSpan;

		switch(LowerValueBound.Type)
		{
			case ECartesianRangeBoundType::FixedValue:
			YSpan.Min = LowerValueBound.FixedBoundValue;
			break;

			case ECartesianRangeBoundType::FitToData:
			case ECartesianRangeBoundType::FitToDataRounded:
			{
				if(!DataYSpan.IsSet())
				{
					DataYSpan = Snapshot.GetDataRange(EAxis::Y, false, Enabled, bExtendValueRangeToZero);
				}
				YSpan.Min = DataYSpan->Min;
			}
			break;
		}

		switch(UpperValueBound.Type)
		{
			case ECartesianRangeBoundType::FixedValue:
			YSpan.Max = UpperValueBound.FixedBoundValue;
			break;

			case ECartesianRangeBoundType::FitToData:
			case ECartesianRangeBoundType::FitToDataRounded:
			{
				if(!DataYSpan.IsSet())
				{
					DataYSpan = Snapshot.GetDataRange(EAxis::Y, false, Enabled, bExtendValueRangeToZero);
				}
				YSpan.Max = DataYSpan->Max;
			}
			break;
		}

		// Need to validate before passing into rounding calcs, to avoid infinite looping.
		YSpan = ValidateAxisDisplayRange(YSpan);

		FCartesianAxisRange RightYSpan;

		switch (RightLowerValueBound.Type)
		{
			case ECartesianRangeBoundType::FixedValue:
				RightYSpan.Min = RightLowerValueBound.FixedBoundValue;
				break;
		}

		switch (RightUpperValueBound.Type)
		{
			case ECartesianRangeBoundType::FixedValue:
				RightYSpan.Max = RightUpperValueBound.FixedBoundValue;
				break;
		}

		// Need to validate before passing into rounding calcs, to avoid infinite looping.
		RightYSpan = ValidateAxisDisplayRange(RightYSpan);
		
		if(LowerValueBound.Type == ECartesianRangeBoundType::FitToDataRounded || UpperValueBound.Type == ECartesianRangeBoundType::FitToDataRounded)
		{
			auto RL = FFloatRoundingLevel::MakeFromMinimumStepSize(YSpan.Size() * RoundingProportion);

			if(LowerValueBound.Type == ECartesianRangeBoundType::FitToDataRounded)
			{
				YSpan.Min = RL.RoundDown(YSpan.Min).GetFloatValue();
			}

			if(UpperValueBound.Type == ECartesianRangeBoundType::FitToDataRounded)
			{
				YSpan.Max = RL.RoundUp(YSpan.Max).GetFloatValue();
			}

			// Extra validation just to be safe
			YSpan = ValidateAxisDisplayRange(YSpan);
		}

		NewPlotScale.RangeY = YSpan;
		NewPlotScale.RangeRightY = RightYSpan;
	}

	// X (time) axis
	{
		TOptional< FCartesianAxisRange > DataXSpan;
		FCartesianAxisRange XSpan;

		switch(UpperTimeBound.Type)
		{
			case ECartesianRangeBoundType::FixedValue:
			XSpan.Max = UpperTimeBound.FixedBoundValue;
			break;

			case ECartesianRangeBoundType::FitToData:
			case ECartesianRangeBoundType::FitToDataRounded:
			{
				if(!DataXSpan.IsSet())
				{
					DataXSpan = Snapshot.GetDataRange(EAxis::X, true, Enabled, false);
				}
				XSpan.Max = DataXSpan->Max;
			}
			break;
		}

		if(FixedTimeRange.IsSet())
		{
			if(!DataXSpan.IsSet())
			{
				DataXSpan = Snapshot.GetDataRange(EAxis::X, true, Enabled, false);
			}

			// For fixed time span, ensure the max is at least as big as the time span.
			XSpan.Max = FMath::Max(XSpan.Max, FixedTimeRange.GetValue());

			// Then set the lower bound from the upper one.
			XSpan.Min = XSpan.Max - FixedTimeRange.GetValue();
		}
		else
		{
			switch(LowerTimeBound.Type)
			{
				case ECartesianRangeBoundType::FixedValue:
				XSpan.Min = LowerTimeBound.FixedBoundValue;
				break;

				case ECartesianRangeBoundType::FitToData:
				case ECartesianRangeBoundType::FitToDataRounded:
				{
					if(!DataXSpan.IsSet())
					{
						DataXSpan = Snapshot.GetDataRange(EAxis::X, true, Enabled, false);
					}
					XSpan.Min = DataXSpan->Min;
				}
				break;
			}
		}

		XSpan = ValidateAxisDisplayRange(XSpan);
		
		if((!FixedTimeRange.IsSet() && LowerTimeBound.Type == ECartesianRangeBoundType::FitToDataRounded) || UpperTimeBound.Type == ECartesianRangeBoundType::FitToDataRounded)
		{
			auto RL = FFloatRoundingLevel::MakeFromMinimumStepSize(XSpan.Size() * RoundingProportion);

			if(UpperTimeBound.Type == ECartesianRangeBoundType::FitToDataRounded)
			{
				XSpan.Max = RL.RoundUp(XSpan.Max).GetFloatValue();

				if(FixedTimeRange.IsSet())
				{
					XSpan.Min = XSpan.Max - FixedTimeRange.GetValue();
				}
			}

			if(!FixedTimeRange.IsSet() && LowerTimeBound.Type == ECartesianRangeBoundType::FitToDataRounded)
			{
				XSpan.Min = RL.RoundDown(XSpan.Min).GetFloatValue();
			}

			XSpan = ValidateAxisDisplayRange(XSpan);
		}

		NewPlotScale.RangeX = XSpan;

		/*	if(bRoundTimeRange)
		{
		auto RL = FFloatRoundingLevel::MakeFromMinimumStepSize(NewPlotScale.RangeX.Size() * RoundingProportion);
		NewPlotScale.RangeX.Max = RL.RoundUp(NewPlotScale.RangeX.Max).GetFloatValue();
		if(DisplayTimeRange > 0.0f)
		{
		NewPlotScale.RangeX.Min = NewPlotScale.RangeX.Max - DisplayTimeRange;
		//RL.RoundDown(NewPlotScale.RangeX.Min).GetFloatValue();
		}
		}
		*/
	}

	return NewPlotScale;
}

void SKantanTimeSeriesPlot::GetLinePointsToDraw(
	TArray< FKantanCartesianDatapoint > const& InPoints,
	FCartesianAxisRange const& RangeX,
	FCartesianAxisRange const& RangeY,
	TArray< FVector2D >& OutPoints) const
{
	// Assume points are ordered by x value, and search for first and last points within the plot range
	struct ComparePred
	{
		inline bool operator() (FKantanCartesianDatapoint const& A, FKantanCartesianDatapoint const& B) const
		{
			return A.Coords.X < B.Coords.X;
		};

		inline bool operator() (FKantanCartesianDatapoint const& A, float B) const
		{
			return A.Coords.X < B;
		};

		inline bool operator() (float A, FKantanCartesianDatapoint const& B) const
		{
			return A < B.Coords.X;
		};
	};

	int32 Lower = 0;
	int32 Upper = 0;

	if (InPoints.Num())
	{
		Lower = std::lower_bound(&InPoints[0], &InPoints[0] + InPoints.Num(), RangeX.Min, ComparePred()) - &InPoints[0];
		Upper = (Lower == InPoints.Num() ? Lower : (std::upper_bound(&InPoints[Lower], &InPoints[0] + InPoints.Num(), RangeX.Max, ComparePred()) - &InPoints[0]));

		// Include point preceding the first in range, if exists
		/*if (Lower != 0 && Lower != InPoints.Num())
		{
			Lower -= 1;
		}*/

		// Include the last point
		if (Upper != InPoints.Num())
		{
			Upper += 1;
		}
	}

	auto const Count = Upper - Lower;
	OutPoints.SetNumUninitialized(Count);
	for (int32 Idx = 0; Idx < Count; ++Idx)
	{
		OutPoints[Idx] = InPoints[Lower + Idx].Coords;
	}
}

void SKantanTimeSeriesPlot::GetMarkerPointsToDraw(
	TArray< FKantanCartesianMarker > const& InMarkers,
	FCartesianAxisRange const& RangeX,
	FCartesianAxisRange const& RangeY,
	TArray< FVector2D >& OutPoints) const
{
	// Assume points are ordered by x value, and search for first and last points within the plot range
	struct ComparePred
	{
		inline bool operator() (FKantanCartesianMarker const& A, FKantanCartesianMarker const& B) const
		{
			return A.Time < B.Time;
		};

		inline bool operator() (FKantanCartesianMarker const& A, float B) const
		{
			return A.Time < B;
		};

		inline bool operator() (float A, FKantanCartesianMarker const& B) const
		{
			return A < B.Time;
		};
	};

	int32 Lower = 0;
	int32 Upper = 0;

	if (InMarkers.Num())
	{
		Lower = std::lower_bound(&InMarkers[0], &InMarkers[0] + InMarkers.Num(), RangeX.Min, ComparePred()) - &InMarkers[0];
		Upper = (Lower == InMarkers.Num() ? Lower : (std::upper_bound(&InMarkers[Lower], &InMarkers[0] + InMarkers.Num(), RangeX.Max, ComparePred()) - &InMarkers[0]));

		// Include the last point
		if (Upper != InMarkers.Num())
		{
			Upper += 1;
		}
	}

	auto const Count = Upper - Lower;
	OutPoints.SetNumUninitialized(Count * 2);
	for (int32 Idx = 0; Idx < Count; ++Idx)
	{
		//OutPoints[Idx] = InMarkers[Lower + Idx].Coords;
		OutPoints[Idx * 2] = FVector2D(InMarkers[Lower + Idx].Time, RangeY.Max);
		OutPoints[Idx * 2 + 1] = FVector2D(InMarkers[Lower + Idx].Time, RangeY.Min);
	}
}

