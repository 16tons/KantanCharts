// Copyright (C) 2015-2018 Cameron Angus. All Rights Reserved.

#include "Charts/SKantanChartLegend.h"
#include "Charts/SKantanCartesianChart.h"

#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"


void SKantanChartLegend::Construct(const FArguments& InArgs)
{
	SetMargins(InArgs._Margins);
	SetSeriesPadding(InArgs._SeriesPadding);
	SetShowDataStyle(InArgs._bShowDataStyle);
	SetBackgroundOverride(InArgs._BackgroundOverride);
	SetFontSizeOverride(InArgs._FontSizeOverride);
}

TSharedRef< SWidget > SKantanChartLegend::AsWidget()
{
	return SharedThis(this);
}

void SKantanChartLegend::SetChart(const TSharedPtr< KantanCharts::ICartesianChart >& InChart)
{
	ChartPtr = InChart; //StaticCastSharedPtr< SKantanCartesianChart >(InChart);
}

void SKantanChartLegend::SetMargins(FMargin const& InMargins)
{
	Margins = InMargins;
}

void SKantanChartLegend::SetSeriesPadding(FMargin const& InPadding)
{
	Padding = InPadding;
}

void SKantanChartLegend::SetShowDataStyle(bool bShow)
{
	bShowDataStyle = bShow;
}

void SKantanChartLegend::SetBackgroundOverride(const FSlateBrush* Background)
{
	BackgroundOverride = Background;
}

void SKantanChartLegend::SetFontSizeOverride(int32 FontSize)
{
	FontSizeOverride = FontSize;
}

void SKantanChartLegend::SetShowSeriesMarker(bool bShow)
{
	bShowSeriesMarker = bShow;
}

void SKantanChartLegend::SetMarkerDataAsset(UChartEventMarkerDataAsset* InDA)
{
	MarkerDataAsset = InDA;
}

void SKantanChartLegend::SetRowCount(int InCount)
{
	RowCount = InCount;
}

FVector2D SKantanChartLegend::ComputeDesiredSize(float) const
{
	auto Chart = ChartPtr.Pin();
	if(!Chart.IsValid())
	{
		return FVector2D(150, 30);
	}

	auto FontInfo = GetLegendFont();
	auto FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		
	auto const& Snapshot = Chart->GetCurrentSnapshot();
	FVector2D ReqSize(0, 0);
	float BiggestXExtent = 0.0f;
	float CurrentRowYExtent = 0.0f;

	if (bShowSeriesMarker)
	{
		TArray<int32> VisibleMarkers;
		for (auto Entry : Snapshot.Elements)
		{
			auto Id = Entry.Id;
			if (!Chart->IsSeriesEnabled(Id))
			{
				continue;
			}
			for (auto Marker : Entry.Markers)
			{
				VisibleMarkers.AddUnique(Marker.MarkerId);
			}
		}

		if (VisibleMarkers.Num() > 0)
		{
			for (int i = 0; i < VisibleMarkers.Num(); i++)
			{
				auto Marker = VisibleMarkers[i];
				if (!MarkerDataAsset->ChartEventMarker.Contains(Marker))
				{
					Marker = -1;
				}
				FVector2D MarkerSize = MarkerDataAsset->ChartEventMarker[Marker].Brush.ImageSize;

				FText Label = MarkerDataAsset->ChartEventMarker[Marker].DisplayText;
				FVector2D LabelExtent = FontMeasureService->Measure(Label, FontInfo);
				LabelExtent = FVector2D(LabelExtent.X + MarkerSize.X, FMath::Max(LabelExtent.Y, MarkerSize.Y));
				LabelExtent += Padding.GetDesiredSize();

				if (RowCount > 0)
				{
					BiggestXExtent = FMath::Max(LabelExtent.X, BiggestXExtent);
					CurrentRowYExtent += LabelExtent.Y;
					if (i % RowCount == RowCount - 1 || i == Snapshot.Elements.Num() - 1)
					{
						ReqSize.X += BiggestXExtent;
						ReqSize.Y = FMath::Max(CurrentRowYExtent, ReqSize.Y);

						BiggestXExtent = 0.0f;
						CurrentRowYExtent = 0.0f;
					}
				}
				else
				{
					ReqSize.Y += LabelExtent.Y;
					ReqSize.X = FMath::Max(LabelExtent.X, ReqSize.X);
				}
			}
		}
	}
	else
	{
		for(int i = 0; i < Snapshot.Elements.Num(); i++)
		{
			auto Entry = Snapshot.Elements[i];
			FText Label = Entry.Name;
			FVector2D LabelExtent = FontMeasureService->Measure(Label, FontInfo);
			LabelExtent += Padding.GetDesiredSize();

			if (RowCount > 0)
			{
				BiggestXExtent = FMath::Max(LabelExtent.X, BiggestXExtent);
				CurrentRowYExtent += LabelExtent.Y;
				if (i % RowCount == RowCount - 1 || i  == Snapshot.Elements.Num() - 1)
				{
					ReqSize.X += BiggestXExtent;
					ReqSize.Y = FMath::Max(CurrentRowYExtent, ReqSize.Y);
					
					BiggestXExtent = 0.0f;
					CurrentRowYExtent = 0.0f;
				}
			}
			else
			{
				ReqSize.Y += LabelExtent.Y;
				ReqSize.X = FMath::Max(LabelExtent.X, ReqSize.X);
			}
		}
	}

	ReqSize += Margins.GetDesiredSize();
	return ReqSize;
}

int32 SKantanChartLegend::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// Pre-snap the clipping rect to try and reduce common jitter, since the padding is typically only a single pixel.
	FSlateRect SnappedClippingRect = FSlateRect(FMath::RoundToInt(MyClippingRect.Left), FMath::RoundToInt(MyClippingRect.Top), FMath::RoundToInt(MyClippingRect.Right), FMath::RoundToInt(MyClippingRect.Bottom));

	auto FontInfo = GetLegendFont();

	auto Chart = ChartPtr.Pin();
	if(!Chart.IsValid())
	{
		auto Label = FText::FromString(TEXT("[Legend]"));
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			Label,
			FontInfo,
			//SnappedClippingRect,
			ESlateDrawEffect::None
			);
		return LayerId;
	}

	auto ChartStyle = Chart->GetChartStyle();
	auto Brush = BackgroundOverride;
	if(Brush == nullptr)
	{
		Brush = &ChartStyle->Background;
	}

	FVector2D DesiredDrawSize = ComputeDesiredSize(0.0f);
	
	if (DesiredDrawSize > Margins.GetDesiredSize())
	{
		// Offset to center the Legend inside the alloted geometry
		FVector2D GeomOffset = (AllottedGeometry.GetLocalSize() - DesiredDrawSize) / 2.f;
		auto BackgroundGeom = AllottedGeometry.MakeChild(
			DesiredDrawSize,
			FSlateLayoutTransform(GeomOffset)
		);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			BackgroundGeom.ToPaintGeometry(),
			Brush,
			//SnappedClippingRect,
			ESlateDrawEffect::None,
			Brush->TintColor.GetColor(InWidgetStyle)
		);
		++LayerId;

		auto FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

		auto const& Snapshot = Chart->GetCurrentSnapshot();
		FVector2D Offset(Margins.Left, Margins.Top);
		float BiggestXExtent = 0.0f;

		if (bShowSeriesMarker)
		{
			TArray<int32> VisibleMarkers;
			for (auto Entry : Snapshot.Elements)
			{
				auto Id = Entry.Id;
				if (!Chart->IsSeriesEnabled(Id))
				{
					continue;
				}
				for (auto Marker : Entry.Markers)
				{
					VisibleMarkers.AddUnique(Marker.MarkerId);
				}
			}

			if (VisibleMarkers.Num() > 0)
			{
				for (int i = 0; i < VisibleMarkers.Num(); i++)
				{
					auto Marker = VisibleMarkers[i];
					if (!MarkerDataAsset->ChartEventMarker.Contains(Marker))
					{
						Marker = -1;
					}
					if (MarkerDataAsset->ChartEventMarker.Contains(Marker))
					{
						FVector2D MarkerSize = MarkerDataAsset->ChartEventMarker[Marker].Brush.ImageSize;

						FText Label = MarkerDataAsset->ChartEventMarker[Marker].DisplayText;
						FVector2D LabelExtent = FontMeasureService->Measure(Label, FontInfo);
						LabelExtent = FVector2D(LabelExtent.X, FMath::Max(LabelExtent.Y, MarkerSize.Y));

						auto IconGeom = AllottedGeometry.MakeChild(
							MarkerSize,
							FSlateLayoutTransform(GeomOffset + Offset + FVector2D(Padding.Left, Padding.Top))
						);

						FSlateDrawElement::MakeBox(
							OutDrawElements,
							LayerId,
							IconGeom.ToPaintGeometry(),
							&MarkerDataAsset->ChartEventMarker[Marker].Brush,
							ESlateDrawEffect::None,
							FLinearColor(1, 1, 1, 1)
						);

						auto TextGeom = AllottedGeometry.MakeChild(
							LabelExtent,
							FSlateLayoutTransform(GeomOffset + Offset + FVector2D(Padding.Left + MarkerSize.X, Padding.Top))
						);

						FSlateDrawElement::MakeText(
							OutDrawElements,
							LayerId,
							TextGeom.ToPaintGeometry(),
							Label,
							FontInfo,
							//SnappedClippingRect,
							ESlateDrawEffect::None,
							FLinearColor(0, 0, 0, 1)
						);

						BiggestXExtent = FMath::Max(LabelExtent.X + MarkerSize.X, BiggestXExtent);

						if (RowCount > 0 && i % RowCount == RowCount - 1)
						{
							Offset.X += BiggestXExtent + Padding.GetDesiredSize().X;
							BiggestXExtent = 0.0f;
							Offset.Y = Margins.Top;
						}
						else
						{
							Offset.Y += LabelExtent.Y + Padding.GetDesiredSize().Y;
						}
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < Snapshot.Elements.Num(); i++)
			{
				auto Entry = Snapshot.Elements[i];
				auto Id = Entry.Id;
				if (!Chart->IsSeriesEnabled(Id))
				{
					continue;
				}

				auto const& SeriesStyle = Chart->GetSeriesStyle(Id);

				FText Label = Entry.Name;
				FVector2D LabelExtent = FontMeasureService->Measure(Label, FontInfo);

				auto TextGeom = AllottedGeometry.MakeChild(
					LabelExtent,
					FSlateLayoutTransform(GeomOffset + Offset + FVector2D(Padding.Left, Padding.Top))
				);
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId,
					TextGeom.ToPaintGeometry(),
					Label,
					FontInfo,
					//SnappedClippingRect,
					ESlateDrawEffect::None,
					SeriesStyle.Color
				);

				BiggestXExtent = FMath::Max(LabelExtent.X, BiggestXExtent);

				if (RowCount > 0 && i % RowCount == RowCount - 1)
				{
					Offset.X += BiggestXExtent + Padding.GetDesiredSize().X;
					BiggestXExtent = 0.0f;
					Offset.Y = Margins.Top;
				}
				else
				{
					Offset.Y += LabelExtent.Y + Padding.GetDesiredSize().Y;
				}
			}
		}
	}

	return LayerId;
}

FSlateFontInfo SKantanChartLegend::GetLegendFont() const
{
	auto Chart = ChartPtr.Pin();
	if(!Chart.IsValid())
	{
		auto DefaultFont = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText").Font;
		DefaultFont.Size = 18;
		return DefaultFont;
	}

	// Default to chart font
	auto ChartStyle = Chart->GetChartStyle();
	auto FontInfo = SKantanChart::GetLabelFont(ChartStyle, EKantanChartLabelClass::AxisTitle);

	// But allow size override
	if(FontSizeOverride != 0)
	{
		FontInfo.Size = FontSizeOverride;
	}

	return FontInfo;
}

