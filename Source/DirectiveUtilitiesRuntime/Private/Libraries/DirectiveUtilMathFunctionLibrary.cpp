// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

#include "Components/SplineComponent.h"
#include "Math/RotationMatrix.h"

#include <limits>

namespace
{
	constexpr double DirectionDotTolerance = 8.0 * std::numeric_limits<double>::epsilon();
	constexpr double ProjectionTolerance = 32.0 * std::numeric_limits<double>::epsilon();

	bool IsSupportedGeneratedElementCount(const int64 Count)
	{
		return Count > 0 && Count <= UDirectiveUtilMathFunctionLibrary::MaximumGeneratedElementCount;
	}

	bool IsFiniteVector2D(const FVector2D& Value)
	{
		return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y);
	}

	bool IsFiniteVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y) && FMath::IsFinite(Value.Z);
	}

	bool TryGetNormalizedVector(const FVector& Value, FVector& Normalized, double* Length = nullptr)
	{
		Normalized = FVector::ZeroVector;
		if (Value.ContainsNaN())
		{
			return false;
		}

		const double MaximumComponent = Value.GetAbsMax();
		if (MaximumComponent == 0.0)
		{
			return false;
		}

		const FVector Scaled = Value / MaximumComponent;
		const double ScaledLength = Scaled.Size();
		if (!FMath::IsFinite(ScaledLength) || ScaledLength == 0.0)
		{
			return false;
		}

		Normalized = Scaled / ScaledLength;
		if (Length)
		{
			if (ScaledLength > TNumericLimits<double>::Max() / MaximumComponent)
			{
				Normalized = FVector::ZeroVector;
				return false;
			}
			*Length = MaximumComponent * ScaledLength;
		}
		return !Normalized.ContainsNaN();
	}

	bool TryGetProjectedDirection(const FVector& Value, const FVector& NormalizedAxis, FVector& Direction)
	{
		Direction = FVector::ZeroVector;
		if (Value.ContainsNaN())
		{
			return false;
		}

		const double MaximumComponent = Value.GetAbsMax();
		if (MaximumComponent == 0.0)
		{
			return false;
		}

		const FVector Scaled = Value / MaximumComponent;
		const FVector Projected = FVector::VectorPlaneProject(Scaled, NormalizedAxis);
		if (Projected.GetAbsMax() <= ProjectionTolerance)
		{
			return false;
		}
		return TryGetNormalizedVector(Projected, Direction);
	}

	bool IsNormalizedDirectionWithinCone(const FVector& NormalizedDirection, const FVector& ConeDirection,
		const float ConeHalfAngleDegrees)
	{
		FVector NormalizedConeDirection;
		if (!FMath::IsFinite(ConeHalfAngleDegrees)
			|| !TryGetNormalizedVector(ConeDirection, NormalizedConeDirection))
		{
			return false;
		}

		const double Dot = FMath::Clamp(
			FVector::DotProduct(NormalizedDirection, NormalizedConeDirection), -1.0, 1.0);
		const double ClampedHalfAngle = FMath::Clamp(static_cast<double>(ConeHalfAngleDegrees), 0.0, 180.0);
		return Dot + DirectionDotTolerance >= FMath::Cos(FMath::DegreesToRadians(ClampedHalfAngle));
	}

	bool TryGetRotationQuaternion(const FRotator& Rotation, FQuat& Quaternion)
	{
		Quaternion = FQuat::Identity;
		if (!FMath::IsFinite(Rotation.Pitch) || !FMath::IsFinite(Rotation.Yaw) || !FMath::IsFinite(Rotation.Roll))
		{
			return false;
		}

		Quaternion = Rotation.Quaternion();
		return !Quaternion.ContainsNaN();
	}

	bool TryGetRotatedAxes(const FRotator& Rotation, FVector& AxisX, FVector& AxisY, FVector& AxisZ)
	{
		AxisX = FVector::ZeroVector;
		AxisY = FVector::ZeroVector;
		AxisZ = FVector::ZeroVector;
		FQuat Quaternion;
		if (!TryGetRotationQuaternion(Rotation, Quaternion))
		{
			return false;
		}

		AxisX = Quaternion.GetAxisX();
		AxisY = Quaternion.GetAxisY();
		AxisZ = Quaternion.GetAxisZ();
		return !AxisX.ContainsNaN() && !AxisY.ContainsNaN() && !AxisZ.ContainsNaN();
	}

	struct FHexLayout
	{
		FVector Origin = FVector::ZeroVector;
		FVector AxisX = FVector::ZeroVector;
		FVector AxisY = FVector::ZeroVector;
		FVector StepQ = FVector::ZeroVector;
		FVector StepR = FVector::ZeroVector;
		double LayoutRadius = 0.0;
		EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop;

		FVector GetLocation(const int64 Q, const int64 R) const
		{
			return Origin + StepQ * static_cast<double>(Q) + StepR * static_cast<double>(R);
		}
	};

	bool TryMakeHexLayout(const FVector& Origin, const FRotator& Rotation, const double CellRadius,
		const double Gap, const EDirectiveUtilHexOrientation Orientation, FHexLayout& Layout)
	{
		FVector AxisX;
		FVector AxisY;
		FVector AxisZ;
		if (Origin.ContainsNaN() || !FMath::IsFinite(CellRadius) || CellRadius <= 0.0
			|| !FMath::IsFinite(Gap) || !TryGetRotatedAxes(Rotation, AxisX, AxisY, AxisZ))
		{
			return false;
		}

		const double LayoutRadius = CellRadius + Gap / UE_DOUBLE_SQRT_3;
		if (!FMath::IsFinite(LayoutRadius) || LayoutRadius <= 0.0)
		{
			return false;
		}

		Layout.Origin = Origin;
		Layout.AxisX = AxisX;
		Layout.AxisY = AxisY;
		Layout.LayoutRadius = LayoutRadius;
		Layout.Orientation = Orientation;
		switch (Orientation)
		{
		case EDirectiveUtilHexOrientation::PointyTop:
			Layout.StepQ = AxisX * (UE_DOUBLE_SQRT_3 * LayoutRadius);
			Layout.StepR = (AxisX * (UE_DOUBLE_SQRT_3 * 0.5) + AxisY * 1.5) * LayoutRadius;
			break;
		case EDirectiveUtilHexOrientation::FlatTop:
			Layout.StepQ = (AxisX * 1.5 + AxisY * (UE_DOUBLE_SQRT_3 * 0.5)) * LayoutRadius;
			Layout.StepR = AxisY * (UE_DOUBLE_SQRT_3 * LayoutRadius);
			break;
		default:
			return false;
		}
		return !Layout.StepQ.ContainsNaN() && !Layout.StepR.ContainsNaN();
	}

	constexpr int32 HexDirections[6][2] = {
		{ 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, 0 }, { -1, 1 }, { 0, 1 }
	};

	void GetHexOffsetCoordinate(const EDirectiveUtilHexOrientation Orientation, const int32 Column,
		const int32 Row, int64& Q, int64& R)
	{
		if (Orientation == EDirectiveUtilHexOrientation::PointyTop)
		{
			Q = static_cast<int64>(Column) - (Row - (Row & 1)) / 2;
			R = Row;
		}
		else
		{
			Q = Column;
			R = static_cast<int64>(Row) - (Column - (Column & 1)) / 2;
		}
	}

	bool TryMakeHexCoordinate(const int64 Q, const int64 R, FIntPoint& Coordinate)
	{
		if (Q < MIN_int32 || Q > MAX_int32 || R < MIN_int32 || R > MAX_int32)
		{
			return false;
		}
		Coordinate = FIntPoint(static_cast<int32>(Q), static_cast<int32>(R));
		return true;
	}

	bool TryValidateHexLayoutSpan(const FHexLayout& Layout, const int64 MaxAbsQ, const int64 MaxAbsR)
	{
		if (MaxAbsQ < 0 || MaxAbsR < 0)
		{
			return false;
		}
		const FVector Bound = Layout.Origin.GetAbs()
			+ Layout.StepQ.GetAbs() * static_cast<double>(MaxAbsQ)
			+ Layout.StepR.GetAbs() * static_cast<double>(MaxAbsR);
		return IsFiniteVector(Bound);
	}

	void WriteHexLocationUnchecked(const FHexLayout& Layout, const int64 Q, const int64 R, FVector& Destination)
	{
		Destination = Layout.GetLocation(Q, R);
	}

	bool TryApplyRectangularHexCentering(FHexLayout& Layout, const FIntPoint Dimensions, const bool bCentered)
	{
		if (!bCentered)
		{
			return true;
		}

		const double NeighborSpacing = UE_DOUBLE_SQRT_3 * Layout.LayoutRadius;
		double CenterX;
		double CenterY;
		if (Layout.Orientation == EDirectiveUtilHexOrientation::PointyTop)
		{
			CenterX = NeighborSpacing * (Dimensions.X - 1 + (Dimensions.Y > 1 ? 0.5 : 0.0)) * 0.5;
			CenterY = 1.5 * Layout.LayoutRadius * (Dimensions.Y - 1) * 0.5;
		}
		else
		{
			CenterX = 1.5 * Layout.LayoutRadius * (Dimensions.X - 1) * 0.5;
			CenterY = NeighborSpacing * (Dimensions.Y - 1 + (Dimensions.X > 1 ? 0.5 : 0.0)) * 0.5;
		}
		Layout.Origin -= Layout.AxisX * CenterX + Layout.AxisY * CenterY;
		return !Layout.Origin.ContainsNaN();
	}

	int64 GetRectangularHexSpan(const FIntPoint Dimensions)
	{
		// Offset coordinates stay within |Q|,|R| ≤ Columns+Rows for both orientations.
		return static_cast<int64>(Dimensions.X) + Dimensions.Y;
	}

	template <typename VisitorType>
	bool VisitHexagonalCoordinates(const int64 Radius, VisitorType Visitor)
	{
		for (int64 R = -Radius; R <= Radius; ++R)
		{
			const int64 MinimumQ = FMath::Max(-Radius, -R - Radius);
			const int64 MaximumQ = FMath::Min(Radius, -R + Radius);
			for (int64 Q = MinimumQ; Q <= MaximumQ; ++Q)
			{
				if (!Visitor(Q, R))
				{
					return false;
				}
			}
		}
		return true;
	}

	template <typename VisitorType>
	bool VisitRectangularHexCoordinates(const FIntPoint Dimensions,
		const EDirectiveUtilHexOrientation Orientation, VisitorType Visitor)
	{
		for (int32 Row = 0; Row < Dimensions.Y; ++Row)
		{
			for (int32 Column = 0; Column < Dimensions.X; ++Column)
			{
				int64 Q;
				int64 R;
				GetHexOffsetCoordinate(Orientation, Column, Row, Q, R);
				if (!Visitor(Q, R))
				{
					return false;
				}
			}
		}
		return true;
	}

	bool TryRoundHexCoordinate(const double FractionalQ, const double FractionalR, FIntPoint& Coordinate)
	{
		const double FractionalS = -FractionalQ - FractionalR;
		if (!FMath::IsFinite(FractionalQ) || !FMath::IsFinite(FractionalR) || !FMath::IsFinite(FractionalS))
		{
			return false;
		}

		double Q = FMath::RoundHalfFromZero(FractionalQ);
		double R = FMath::RoundHalfFromZero(FractionalR);
		double S = FMath::RoundHalfFromZero(FractionalS);
		const double QDifference = FMath::Abs(Q - FractionalQ);
		const double RDifference = FMath::Abs(R - FractionalR);
		const double SDifference = FMath::Abs(S - FractionalS);
		if (QDifference > RDifference && QDifference > SDifference)
		{
			Q = -R - S;
		}
		else if (RDifference > SDifference)
		{
			R = -Q - S;
		}

		if (Q < MIN_int32 || Q > MAX_int32 || R < MIN_int32 || R > MAX_int32)
		{
			return false;
		}
		Coordinate = FIntPoint(static_cast<int32>(Q), static_cast<int32>(R));
		return true;
	}

	bool TryGetHexCoordinate(const FHexLayout& Layout, const FVector& Location, FIntPoint& Coordinate)
	{
		if (Location.ContainsNaN())
		{
			return false;
		}

		const FVector Offset = Location - Layout.Origin;
		const double X = FVector::DotProduct(Offset, Layout.AxisX) / Layout.LayoutRadius;
		const double Y = FVector::DotProduct(Offset, Layout.AxisY) / Layout.LayoutRadius;
		double Q;
		double R;
		switch (Layout.Orientation)
		{
		case EDirectiveUtilHexOrientation::PointyTop:
			Q = UE_DOUBLE_SQRT_3 / 3.0 * X - Y / 3.0;
			R = 2.0 / 3.0 * Y;
			break;
		case EDirectiveUtilHexOrientation::FlatTop:
			Q = 2.0 / 3.0 * X;
			R = -X / 3.0 + UE_DOUBLE_SQRT_3 / 3.0 * Y;
			break;
		default:
			return false;
		}
		return TryRoundHexCoordinate(Q, R, Coordinate);
	}

	bool TryGetHexagonalPointCount(const int32 GridRadius, int32& PointCount)
	{
		PointCount = 0;
		if (GridRadius < 0)
		{
			return false;
		}

		const int64 Radius = GridRadius;
		const int64 Multiplier = 3 * (Radius + 1);
		if (Radius > (MAX_int32 - 1) / Multiplier)
		{
			return false;
		}
		const int64 Count = 1 + Radius * Multiplier;
		if (!IsSupportedGeneratedElementCount(Count))
		{
			return false;
		}
		PointCount = static_cast<int32>(Count);
		return true;
	}

	bool TryGetGridPointCount(const FIntVector& Dimensions, int32& PointCount)
	{
		PointCount = 0;
		if (Dimensions.X <= 0 || Dimensions.Y <= 0 || Dimensions.Z <= 0)
		{
			return false;
		}

		constexpr int64 MaximumPointCount = UDirectiveUtilMathFunctionLibrary::MaximumGeneratedElementCount;
		int64 Count = Dimensions.X;
		if (Count > MaximumPointCount / Dimensions.Y)
		{
			return false;
		}
		Count *= Dimensions.Y;
		if (Count > MaximumPointCount / Dimensions.Z)
		{
			return false;
		}
		PointCount = static_cast<int32>(Count * Dimensions.Z);
		return true;
	}

	struct FGridLattice
	{
		int32 DimX = 0;
		int32 DimY = 0;
		int32 DimZ = 0;
		int32 PointCount = 0;
		FVector FirstPoint = FVector::ZeroVector;
		FVector StepX = FVector::ZeroVector;
		FVector StepY = FVector::ZeroVector;
		FVector StepZ = FVector::ZeroVector;
	};

	bool TryMakeGridLattice(const FVector& Origin, const FRotator& Rotation,
		const FIntVector& Dimensions, const FVector& Spacing, const bool bCentered, FGridLattice& Lattice)
	{
		Lattice = {};
		FVector AxisX;
		FVector AxisY;
		FVector AxisZ;
		if (!IsFiniteVector(Origin) || !IsFiniteVector(Spacing)
			|| !TryGetGridPointCount(Dimensions, Lattice.PointCount)
			|| !TryGetRotatedAxes(Rotation, AxisX, AxisY, AxisZ))
		{
			return false;
		}

		Lattice.DimX = Dimensions.X;
		Lattice.DimY = Dimensions.Y;
		Lattice.DimZ = Dimensions.Z;
		Lattice.StepX = AxisX * Spacing.X;
		Lattice.StepY = AxisY * Spacing.Y;
		Lattice.StepZ = AxisZ * Spacing.Z;
		Lattice.FirstPoint = Origin;
		if (bCentered)
		{
			Lattice.FirstPoint -= (Lattice.StepX * (Lattice.DimX - 1)
				+ Lattice.StepY * (Lattice.DimY - 1)
				+ Lattice.StepZ * (Lattice.DimZ - 1)) * 0.5;
		}

		// Reject if any lattice point can overflow: |p| <= |First| + Σ |Step|*(Dim-1).
		const FVector CoordinateBound = Lattice.FirstPoint.GetAbs()
			+ Lattice.StepX.GetAbs() * (Lattice.DimX - 1)
			+ Lattice.StepY.GetAbs() * (Lattice.DimY - 1)
			+ Lattice.StepZ.GetAbs() * (Lattice.DimZ - 1);
		return IsFiniteVector(Lattice.FirstPoint)
			&& IsFiniteVector(Lattice.StepX)
			&& IsFiniteVector(Lattice.StepY)
			&& IsFiniteVector(Lattice.StepZ)
			&& IsFiniteVector(CoordinateBound);
	}

	TArray<FVector> GenerateGridPoints(const FVector& Origin, const FRotator& Rotation,
		const FIntVector& Dimensions, const FVector& Spacing, const bool bCentered)
	{
		FGridLattice Lattice;
		if (!TryMakeGridLattice(Origin, Rotation, Dimensions, Spacing, bCentered, Lattice))
		{
			return {};
		}

		TArray<FVector> Points;
		Points.SetNumUninitialized(Lattice.PointCount);
		FVector* RESTRICT Dest = Points.GetData();
		for (int32 Z = 0; Z < Lattice.DimZ; ++Z)
		{
			const FVector LayerStart = Lattice.FirstPoint + Lattice.StepZ * Z;
			for (int32 Y = 0; Y < Lattice.DimY; ++Y)
			{
				const FVector RowStart = LayerStart + Lattice.StepY * Y;
				for (int32 X = 0; X < Lattice.DimX; ++X)
				{
					*Dest++ = RowStart + Lattice.StepX * X;
				}
			}
		}
		return Points;
	}

	TArray<FTransform> GenerateGridTransforms(const FVector& Origin, const FRotator& Rotation,
		const FIntVector& Dimensions, const FVector& Spacing, const bool bCentered,
		const FRotator& InstanceRotation, const FVector& Scale)
	{
		FGridLattice Lattice;
		FQuat InstanceQuaternion;
		if (!IsFiniteVector(Scale)
			|| !TryGetRotationQuaternion(InstanceRotation, InstanceQuaternion)
			|| !TryMakeGridLattice(Origin, Rotation, Dimensions, Spacing, bCentered, Lattice))
		{
			return {};
		}

		TArray<FTransform> Transforms;
		Transforms.SetNumUninitialized(Lattice.PointCount);
		FTransform* RESTRICT Dest = Transforms.GetData();
		for (int32 Z = 0; Z < Lattice.DimZ; ++Z)
		{
			const FVector LayerStart = Lattice.FirstPoint + Lattice.StepZ * Z;
			for (int32 Y = 0; Y < Lattice.DimY; ++Y)
			{
				const FVector RowStart = LayerStart + Lattice.StepY * Y;
				for (int32 X = 0; X < Lattice.DimX; ++X)
				{
					*Dest++ = FTransform(InstanceQuaternion, RowStart + Lattice.StepX * X, Scale);
				}
			}
		}
		return Transforms;
	}

	TArray<FVector> GenerateLinearPoints(const FVector& Origin, const FVector& Step,
		const int32 Count, const double FirstStep)
	{
		if (!IsSupportedGeneratedElementCount(Count)
			|| !FMath::IsFinite(FirstStep) || Origin.ContainsNaN() || Step.ContainsNaN())
		{
			return {};
		}

		TArray<FVector> Points;
		Points.SetNumUninitialized(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const FVector Point = Origin + Step * (FirstStep + Index);
			if (Point.ContainsNaN())
			{
				return {};
			}
			Points[Index] = Point;
		}
		return Points;
	}

	TArray<FVector> MakeSinglePoint(const FVector& Point)
	{
		return Point.ContainsNaN() ? TArray<FVector>() : TArray<FVector>({ Point });
	}

	struct FAngleStepper
	{
		explicit FAngleStepper(const double InStartAngle, const double InStepAngle)
			: StartAngle(InStartAngle), StepAngle(InStepAngle)
		{
			FMath::SinCos(&Sine, &Cosine, StartAngle);
			FMath::SinCos(&StepSine, &StepCosine, StepAngle);
		}

		void Advance()
		{
			++Index;
			if ((Index & 255) == 0)
			{
				FMath::SinCos(&Sine, &Cosine, StartAngle + StepAngle * Index);
				return;
			}

			const double NextSine = Sine * StepCosine + Cosine * StepSine;
			Cosine = Cosine * StepCosine - Sine * StepSine;
			Sine = NextSine;
		}

		double Sine = 0.0;
		double Cosine = 1.0;

	private:
		double StartAngle;
		double StepAngle;
		double StepSine = 0.0;
		double StepCosine = 1.0;
		int32 Index = 0;
	};

	template <typename RadiusFunction>
	TArray<FVector> GeneratePlanarRadialPoints(const FVector& Center, const FRotator& Rotation,
		const int32 Count, const double StartAngle, const double StepAngle, RadiusFunction&& GetRadius)
	{
		FVector AxisX;
		FVector AxisY;
		FVector AxisZ;
		if (!IsSupportedGeneratedElementCount(Count)
			|| Center.ContainsNaN() || !FMath::IsFinite(StartAngle) || !FMath::IsFinite(StepAngle)
			|| !TryGetRotatedAxes(Rotation, AxisX, AxisY, AxisZ))
		{
			return {};
		}

		TArray<FVector> Points;
		Points.SetNumUninitialized(Count);
		FAngleStepper Angle(StartAngle, StepAngle);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const double Radius = GetRadius(Index);
			const FVector Point = Center + AxisX * (Angle.Cosine * Radius) + AxisY * (Angle.Sine * Radius);
			if (!FMath::IsFinite(Radius) || Point.ContainsNaN())
			{
				return {};
			}
			Points[Index] = Point;
			Angle.Advance();
		}
		return Points;
	}

	bool TryMakeFacingRotationFromNormalized(const FVector& Forward, const FVector& UpDirection,
		const FQuat& RotationOffset, FQuat& Rotation)
	{
		// Forward and RotationOffset are already unit/validated; MakeFromXZ yields a unit quat.
		Rotation = FRotationMatrix::MakeFromXZ(Forward, UpDirection).ToQuat() * RotationOffset;
		return !Rotation.ContainsNaN();
	}

	bool IsValidRadialOrientation(const EDirectiveUtilRadialOrientation Orientation)
	{
		switch (Orientation)
		{
		case EDirectiveUtilRadialOrientation::Fixed:
		case EDirectiveUtilRadialOrientation::FaceCenter:
		case EDirectiveUtilRadialOrientation::FaceAwayFromCenter:
		case EDirectiveUtilRadialOrientation::FollowPath:
		case EDirectiveUtilRadialOrientation::FaceAgainstPath:
			return true;
		default:
			return false;
		}
	}

	bool TryGetRadialRotation(const EDirectiveUtilRadialOrientation Orientation,
		const FVector& AxisX, const FVector& AxisY, const FVector& AxisZ, const FQuat& FixedRotation,
		const FQuat& RotationOffset, const double Sine, const double Cosine, const double PathDirection,
		FQuat& Rotation)
	{
		switch (Orientation)
		{
		case EDirectiveUtilRadialOrientation::Fixed:
			Rotation = FixedRotation;
			return true;
		case EDirectiveUtilRadialOrientation::FaceCenter:
			return TryMakeFacingRotationFromNormalized(
				-AxisX * Cosine - AxisY * Sine, AxisZ, RotationOffset, Rotation);
		case EDirectiveUtilRadialOrientation::FaceAwayFromCenter:
			return TryMakeFacingRotationFromNormalized(
				AxisX * Cosine + AxisY * Sine, AxisZ, RotationOffset, Rotation);
		case EDirectiveUtilRadialOrientation::FollowPath:
			return TryMakeFacingRotationFromNormalized(
				(AxisX * -Sine + AxisY * Cosine) * PathDirection, AxisZ, RotationOffset, Rotation);
		case EDirectiveUtilRadialOrientation::FaceAgainstPath:
			return TryMakeFacingRotationFromNormalized(
				(AxisX * Sine - AxisY * Cosine) * PathDirection, AxisZ, RotationOffset, Rotation);
		default:
			return false;
		}
	}

	TArray<FTransform> GeneratePlanarRadialTransforms(const FVector& Center, const FRotator& PlaneRotator,
		const double Radius, const int32 Count, const double StartAngle, const double StepAngle,
		const double PathDirection, const EDirectiveUtilRadialOrientation Orientation,
		const FRotator& RotationOffset, const FVector& Scale)
	{
		FQuat PlaneRotation;
		FQuat RotationOffsetQuaternion;
		if (!IsSupportedGeneratedElementCount(Count) || Center.ContainsNaN() || Scale.ContainsNaN()
			|| !FMath::IsFinite(Radius) || !FMath::IsFinite(StartAngle) || !FMath::IsFinite(StepAngle)
			|| !FMath::IsFinite(PathDirection) || !IsValidRadialOrientation(Orientation)
			|| !TryGetRotationQuaternion(PlaneRotator, PlaneRotation)
			|| !TryGetRotationQuaternion(RotationOffset, RotationOffsetQuaternion))
		{
			return {};
		}

		const FVector AxisX = PlaneRotation.GetAxisX();
		const FVector AxisY = PlaneRotation.GetAxisY();
		const FVector AxisZ = PlaneRotation.GetAxisZ();
		const double DirectionSign = PathDirection < 0.0 ? -1.0 : 1.0;
		const double AbsoluteRadius = FMath::Abs(Radius);
		FQuat FixedRotation = PlaneRotation * RotationOffsetQuaternion;
		FixedRotation.Normalize();
		TArray<FTransform> Transforms;
		Transforms.SetNumUninitialized(Count);
		FAngleStepper Angle(StartAngle, StepAngle);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const FVector Location = Center
				+ AxisX * (Angle.Cosine * AbsoluteRadius) + AxisY * (Angle.Sine * AbsoluteRadius);
			FQuat TransformRotation;
			if (!TryGetRadialRotation(Orientation, AxisX, AxisY, AxisZ, FixedRotation,
					RotationOffsetQuaternion, Angle.Sine, Angle.Cosine, DirectionSign, TransformRotation))
			{
				return {};
			}
			Transforms[Index] = FTransform(TransformRotation, Location, Scale);
			Angle.Advance();
		}
		return Transforms;
	}

	struct FSplineSamplePlan
	{
		double StartDistance = 0.0;
		double EndDistance = 0.0;
		double Spacing = 0.0;
		float SampleStart = 0.0f;
		float SampleSpacing = 0.0f;
		float SampleEnd = 0.0f;
		int32 RegularSampleCount = 0;
		bool bAppendEndpoint = false;

		int32 Num() const
		{
			return RegularSampleCount + (bAppendEndpoint ? 1 : 0);
		}

		void FinalizeSampleDistances()
		{
			SampleStart = static_cast<float>(StartDistance);
			SampleSpacing = static_cast<float>(Spacing);
			SampleEnd = static_cast<float>(EndDistance);
		}

		float GetDistance(const int32 Index) const
		{
			return bAppendEndpoint && Index == RegularSampleCount
				? SampleEnd
				: SampleStart + static_cast<float>(Index) * SampleSpacing;
		}
	};

	bool IsValidSplineCoordinateSpace(const ESplineCoordinateSpace::Type CoordinateSpace)
	{
		return CoordinateSpace == ESplineCoordinateSpace::Local
			|| CoordinateSpace == ESplineCoordinateSpace::World;
	}

	bool TryResolveSplineSampleRange(const USplineComponent* Spline, const double StartDistance,
		const double EndDistance, FSplineSamplePlan& Plan, bool& bOutFullClosedLoop)
	{
		Plan = {};
		bOutFullClosedLoop = false;
		if (!IsValid(Spline) || Spline->GetNumberOfSplinePoints() <= 0
			|| !FMath::IsFinite(StartDistance) || !FMath::IsFinite(EndDistance))
		{
			return false;
		}

		const double SplineLength = Spline->GetSplineLength();
		if (!FMath::IsFinite(SplineLength) || SplineLength < 0.0)
		{
			return false;
		}

		Plan.StartDistance = FMath::Clamp(StartDistance, 0.0, SplineLength);
		Plan.EndDistance = EndDistance < 0.0 ? SplineLength : FMath::Clamp(EndDistance, 0.0, SplineLength);
		if (Plan.EndDistance < Plan.StartDistance)
		{
			return false;
		}

		bOutFullClosedLoop = Spline->IsClosedLoop()
			&& Plan.StartDistance == 0.0 && Plan.EndDistance == SplineLength;
		return true;
	}

	bool TryMakeSplineSpacingPlan(const USplineComponent* Spline, const double Spacing,
		const EDirectiveUtilSplineSpacingMode SpacingMode, const bool bIncludeEndpoint,
		const double StartDistance, const double EndDistance, FSplineSamplePlan& Plan)
	{
		bool bFullClosedLoop = false;
		if (!FMath::IsFinite(Spacing) || Spacing <= 0.0
			|| (SpacingMode != EDirectiveUtilSplineSpacingMode::Fixed
				&& SpacingMode != EDirectiveUtilSplineSpacingMode::Even)
			|| !TryResolveSplineSampleRange(Spline, StartDistance, EndDistance, Plan, bFullClosedLoop))
		{
			return false;
		}

		const double RangeLength = Plan.EndDistance - Plan.StartDistance;
		if (RangeLength == 0.0)
		{
			Plan.RegularSampleCount = 1;
			Plan.FinalizeSampleDistances();
			return true;
		}

		const double SampleCountValue = FMath::CeilToDouble(RangeLength / Spacing);
		if (!FMath::IsFinite(SampleCountValue) || SampleCountValue < 1.0
			|| SampleCountValue > UDirectiveUtilMathFunctionLibrary::MaximumGeneratedElementCount)
		{
			return false;
		}

		Plan.RegularSampleCount = static_cast<int32>(SampleCountValue);
		Plan.Spacing = SpacingMode == EDirectiveUtilSplineSpacingMode::Even
			? RangeLength / SampleCountValue
			: Spacing;
		Plan.bAppendEndpoint = bIncludeEndpoint && !bFullClosedLoop;
		if (Plan.bAppendEndpoint)
		{
			// Drop a final regular sample that float noise placed within a hair of the endpoint.
			const double LastRegularOffset = static_cast<double>(Plan.RegularSampleCount - 1) * Plan.Spacing;
			if (RangeLength - LastRegularOffset <= FMath::Max(UE_DOUBLE_KINDA_SMALL_NUMBER, RangeLength * 1.e-9))
			{
				--Plan.RegularSampleCount;
			}
		}
		if (!IsSupportedGeneratedElementCount(Plan.Num()))
		{
			return false;
		}
		Plan.FinalizeSampleDistances();
		return true;
	}

	bool TryMakeSplineCountPlan(const USplineComponent* Spline, const int32 Count,
		const bool bIncludeEndpoints, const double StartDistance, const double EndDistance,
		FSplineSamplePlan& Plan)
	{
		bool bFullClosedLoop = false;
		if (!IsSupportedGeneratedElementCount(Count)
			|| !TryResolveSplineSampleRange(Spline, StartDistance, EndDistance, Plan, bFullClosedLoop))
		{
			return false;
		}

		Plan.RegularSampleCount = Count;
		const double RangeLength = Plan.EndDistance - Plan.StartDistance;
		if (RangeLength == 0.0 || (bFullClosedLoop && Count == 1))
		{
			Plan.FinalizeSampleDistances();
			return true;
		}
		if (bFullClosedLoop)
		{
			Plan.Spacing = RangeLength / static_cast<double>(Count);
		}
		else if (!bIncludeEndpoints)
		{
			Plan.Spacing = RangeLength / (static_cast<double>(Count) + 1.0);
			Plan.StartDistance += Plan.Spacing;
		}
		else if (Count == 1)
		{
			Plan.StartDistance += RangeLength * 0.5;
		}
		else
		{
			Plan.Spacing = RangeLength / static_cast<double>(Count - 1);
			Plan.RegularSampleCount = Count - 1;
			Plan.bAppendEndpoint = true;
		}
		Plan.FinalizeSampleDistances();
		return true;
	}

	TArray<FVector> SampleSplineLocations(const USplineComponent* Spline, const FSplineSamplePlan& Plan,
		const ESplineCoordinateSpace::Type CoordinateSpace)
	{
		TArray<FVector> Points;
		Points.SetNumUninitialized(Plan.Num());
		for (int32 Index = 0; Index < Points.Num(); ++Index)
		{
			const FVector Point = Spline->GetLocationAtDistanceAlongSpline(
				Plan.GetDistance(Index), CoordinateSpace);
			if (Point.ContainsNaN())
			{
				return {};
			}
			Points[Index] = Point;
		}
		return Points;
	}

	TArray<FTransform> SampleSplineTransforms(const USplineComponent* Spline, const FSplineSamplePlan& Plan,
		const ESplineCoordinateSpace::Type CoordinateSpace, const bool bUseSplineScale,
		const FQuat& RotationOffsetQuaternion, const FVector& ScaleMultiplier)
	{
		TArray<FTransform> Transforms;
		Transforms.SetNumUninitialized(Plan.Num());
		for (int32 Index = 0; Index < Transforms.Num(); ++Index)
		{
			FTransform Transform = Spline->GetTransformAtDistanceAlongSpline(
				Plan.GetDistance(Index), CoordinateSpace, bUseSplineScale);
			FQuat Rotation = Transform.GetRotation() * RotationOffsetQuaternion;
			Rotation.Normalize();
			Transform.SetRotation(Rotation);
			Transform.SetScale3D(Transform.GetScale3D() * ScaleMultiplier);
			if (Transform.ContainsNaN())
			{
				return {};
			}
			Transforms[Index] = Transform;
		}
		return Transforms;
	}

	FTransform BlendTransforms(const FTransform& A, const FTransform& B, const double Alpha)
	{
		FQuat Rotation = FQuat::Slerp(A.GetRotation(), B.GetRotation(), Alpha);
		Rotation.Normalize();
		return FTransform(Rotation,
			FMath::Lerp(A.GetLocation(), B.GetLocation(), Alpha),
			FMath::Lerp(A.GetScale3D(), B.GetScale3D(), Alpha));
	}

	template <typename PositionType>
	bool TryFindArraySample(const int32 Count, const bool bClosedLoop, const float Alpha,
		PositionType Position, int32& IndexA, int32& IndexB, double& SegmentAlpha)
	{
		IndexA = 0;
		IndexB = 0;
		SegmentAlpha = 0.0;
		if (Count <= 0 || !FMath::IsFinite(Alpha))
		{
			return false;
		}
		if (Count == 1)
		{
			return true;
		}

		const int32 SegmentCount = bClosedLoop ? Count : Count - 1;
		// Reuse capacity across samples on this thread; inline storage covers typical Blueprint paths.
		static thread_local TArray<double, TInlineAllocator<128>> SegmentLengths;
		SegmentLengths.SetNumUninitialized(SegmentCount, EAllowShrinking::No);
		double TotalLength = 0.0;
		for (int32 Index = 0; Index < SegmentCount; ++Index)
		{
			const int32 NextIndex = (!bClosedLoop || Index + 1 < Count) ? Index + 1 : 0;
			const double Length = FVector::Distance(Position(Index), Position(NextIndex));
			if (!FMath::IsFinite(Length))
			{
				return false;
			}
			SegmentLengths[Index] = Length;
			TotalLength += Length;
		}
		if (TotalLength <= 0.0)
		{
			return true;
		}

		const double TargetAlpha = bClosedLoop
			? static_cast<double>(Alpha) - FMath::FloorToDouble(Alpha)
			: FMath::Clamp(static_cast<double>(Alpha), 0.0, 1.0);
		double TargetDistance = TargetAlpha * TotalLength;
		for (int32 Index = 0; Index < SegmentCount; ++Index)
		{
			const double Length = SegmentLengths[Index];
			if (TargetDistance <= Length || Index == SegmentCount - 1)
			{
				IndexA = Index;
				IndexB = (!bClosedLoop || Index + 1 < Count) ? Index + 1 : 0;
				SegmentAlpha = Length > 0.0
					? FMath::Clamp(TargetDistance / Length, 0.0, 1.0)
					: 0.0;
				return true;
			}
			TargetDistance -= Length;
		}
		return true;
	}

	// Perlin noise is exactly zero at every integer lattice point, so bias samples off the lattice.
	constexpr double NoiseSampleBias = 0.6180339887498949;

	bool TryOffsetLocationByNoise(const FVector& Location, const FVector& NormalizedDirection,
		const double NoiseScale, const double Amplitude, const FVector& NoiseOffset, FVector& OffsetLocation)
	{
		if (Location.ContainsNaN())
		{
			return false;
		}
		const FVector SamplePosition = (Location + NoiseOffset) / NoiseScale + FVector(NoiseSampleBias);
		const double Noise = FMath::PerlinNoise3D(SamplePosition);
		OffsetLocation = Location + NormalizedDirection * (Noise * Amplitude);
		return !OffsetLocation.ContainsNaN();
	}

	bool TryGetNoiseOffsetInputs(const double NoiseScale, const double Amplitude, const FVector& Direction,
		const FVector& NoiseOffset, FVector& NormalizedDirection)
	{
		return FMath::IsFinite(NoiseScale) && NoiseScale > 0.0 && FMath::IsFinite(Amplitude)
			&& !NoiseOffset.ContainsNaN() && TryGetNormalizedVector(Direction, NormalizedDirection);
	}

	double WrapDegreesAsRadians(const double AngleDegrees)
	{
		return FMath::DegreesToRadians(FMath::Fmod(AngleDegrees, 360.0));
	}

	double FindWrappedDeltaDegrees(const double From, const double To)
	{
		double Delta = FMath::Fmod(To - From, 360.0);
		if (Delta > 180.0)
		{
			Delta -= 360.0;
		}
		else if (Delta < -180.0)
		{
			Delta += 360.0;
		}
		return Delta;
	}

}

float UDirectiveUtilMathFunctionLibrary::PerlinNoise2D(const FVector2D Position)
{
	return FMath::PerlinNoise2D(Position);
}

float UDirectiveUtilMathFunctionLibrary::PerlinNoise3D(const FVector& Position)
{
	return FMath::PerlinNoise3D(Position);
}

float UDirectiveUtilMathFunctionLibrary::AngleBetweenVectors(const FVector& A, const FVector& B)
{
	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(A.GetSafeNormal(), B.GetSafeNormal()), -1.0, 1.0)));
}

float UDirectiveUtilMathFunctionLibrary::SignedAngleBetweenVectors(const FVector& From, const FVector& To, const FVector& Axis)
{
	FVector NormalizedAxis;
	if (!TryGetNormalizedVector(Axis, NormalizedAxis))
	{
		return 0.0f;
	}

	FVector ProjectedFrom;
	FVector ProjectedTo;
	if (!TryGetProjectedDirection(From, NormalizedAxis, ProjectedFrom)
		|| !TryGetProjectedDirection(To, NormalizedAxis, ProjectedTo))
	{
		return 0.0f;
	}

	const double Sine = FVector::DotProduct(NormalizedAxis, FVector::CrossProduct(ProjectedFrom, ProjectedTo));
	const double Cosine = FMath::Clamp(FVector::DotProduct(ProjectedFrom, ProjectedTo), -1.0, 1.0);
	return static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(Sine, Cosine)));
}

float UDirectiveUtilMathFunctionLibrary::DeltaAngle(const float From, const float To)
{
	if (!FMath::IsFinite(From) || !FMath::IsFinite(To))
	{
		return 0.0f;
	}

	return static_cast<float>(FindWrappedDeltaDegrees(From, To));
}

float UDirectiveUtilMathFunctionLibrary::LerpAngle(const float A, const float B, const float Alpha)
{
	if (!FMath::IsFinite(A) || !FMath::IsFinite(B) || !FMath::IsFinite(Alpha))
	{
		return 0.0f;
	}

	const double Result = static_cast<double>(A)
		+ FindWrappedDeltaDegrees(A, B) * static_cast<double>(Alpha);
	return static_cast<float>(FMath::Wrap(Result, -180.0, 180.0));
}

float UDirectiveUtilMathFunctionLibrary::PingPong(const float Value, const float Minimum, const float Maximum)
{
	if (!FMath::IsFinite(Value) || !FMath::IsFinite(Minimum) || !FMath::IsFinite(Maximum))
	{
		return 0.0f;
	}

	const double LowerBound = FMath::Min(static_cast<double>(Minimum), static_cast<double>(Maximum));
	const double UpperBound = FMath::Max(static_cast<double>(Minimum), static_cast<double>(Maximum));
	const double Range = UpperBound - LowerBound;
	if (Range == 0.0)
	{
		return static_cast<float>(LowerBound);
	}

	const double Period = Range * 2.0;
	double Offset = FMath::Fmod(static_cast<double>(Value) - LowerBound, Period);
	if (Offset < 0.0)
	{
		Offset += Period;
	}

	const double DistanceFromLowerBound = Offset <= Range ? Offset : Period - Offset;
	return static_cast<float>(LowerBound + DistanceFromLowerBound);
}

float UDirectiveUtilMathFunctionLibrary::SmoothStep(const float Value, const float Minimum, const float Maximum)
{
	if (!FMath::IsFinite(Value) || !FMath::IsFinite(Minimum) || !FMath::IsFinite(Maximum))
	{
		return 0.0f;
	}

	const double LowerBound = FMath::Min(static_cast<double>(Minimum), static_cast<double>(Maximum));
	const double UpperBound = FMath::Max(static_cast<double>(Minimum), static_cast<double>(Maximum));
	if (LowerBound == UpperBound)
	{
		return Value < LowerBound ? 0.0f : 1.0f;
	}

	const double Alpha = FMath::Clamp((static_cast<double>(Value) - LowerBound) / (UpperBound - LowerBound), 0.0, 1.0);
	return static_cast<float>(Alpha * Alpha * (3.0 - 2.0 * Alpha));
}

float UDirectiveUtilMathFunctionLibrary::SmootherStep(const float Value, const float Minimum, const float Maximum)
{
	if (!FMath::IsFinite(Value) || !FMath::IsFinite(Minimum) || !FMath::IsFinite(Maximum))
	{
		return 0.0f;
	}

	const double LowerBound = FMath::Min(static_cast<double>(Minimum), static_cast<double>(Maximum));
	const double UpperBound = FMath::Max(static_cast<double>(Minimum), static_cast<double>(Maximum));
	if (LowerBound == UpperBound)
	{
		return Value < LowerBound ? 0.0f : 1.0f;
	}

	const double Alpha = FMath::Clamp((static_cast<double>(Value) - LowerBound) / (UpperBound - LowerBound), 0.0, 1.0);
	return static_cast<float>(Alpha * Alpha * Alpha * (Alpha * (Alpha * 6.0 - 15.0) + 10.0));
}

float UDirectiveUtilMathFunctionLibrary::RangeFalloff(const float Distance, const float InnerRadius,
	const float OuterRadius, const float FalloffExponent)
{
	if (!FMath::IsFinite(Distance) || !FMath::IsFinite(InnerRadius)
		|| !FMath::IsFinite(OuterRadius) || !FMath::IsFinite(FalloffExponent))
	{
		return 0.0f;
	}

	const double FirstRadius = FMath::Max(0.0, static_cast<double>(InnerRadius));
	const double SecondRadius = FMath::Max(0.0, static_cast<double>(OuterRadius));
	const double Inner = FMath::Min(FirstRadius, SecondRadius);
	const double Outer = FMath::Max(FirstRadius, SecondRadius);
	const double ClampedDistance = FMath::Max(0.0, static_cast<double>(Distance));
	if (ClampedDistance >= Outer)
	{
		return 0.0f;
	}
	if (ClampedDistance <= Inner || FalloffExponent <= 0.0f)
	{
		return 1.0f;
	}

	const double Alpha = 1.0 - (ClampedDistance - Inner) / (Outer - Inner);
	if (FalloffExponent == 1.0f)
	{
		return static_cast<float>(Alpha);
	}
	if (FalloffExponent == 2.0f)
	{
		return static_cast<float>(Alpha * Alpha);
	}
	return static_cast<float>(FMath::Pow(Alpha, static_cast<double>(FalloffExponent)));
}

bool UDirectiveUtilMathFunctionLibrary::IsDirectionWithinCone(const FVector& Direction, const FVector& ConeDirection, const float ConeHalfAngleDegrees)
{
	FVector NormalizedDirection;
	if (!TryGetNormalizedVector(Direction, NormalizedDirection))
	{
		return false;
	}
	return IsNormalizedDirectionWithinCone(NormalizedDirection, ConeDirection, ConeHalfAngleDegrees);
}

bool UDirectiveUtilMathFunctionLibrary::GetDirectionAndDistance(const FVector& From, const FVector& To,
	FVector& Direction, double& Distance)
{
	Direction = FVector::ZeroVector;
	Distance = 0.0;
	if (From.ContainsNaN() || To.ContainsNaN())
	{
		return false;
	}

	const FVector Delta = To - From;
	if (Delta.ContainsNaN())
	{
		return false;
	}

	if (!TryGetNormalizedVector(Delta, Direction, &Distance))
	{
		Distance = 0.0;
		return false;
	}
	return true;
}

FVector2D UDirectiveUtilMathFunctionLibrary::RotatePointAroundPivot2D(const FVector2D& Point,
	const FVector2D& Pivot, const float AngleDegrees)
{
	if (!IsFiniteVector2D(Point) || !IsFiniteVector2D(Pivot) || !FMath::IsFinite(AngleDegrees))
	{
		return FVector2D::ZeroVector;
	}

	const FVector2D Offset = Point - Pivot;
	const double AngleRadians = FMath::DegreesToRadians(static_cast<double>(AngleDegrees));
	const double Sine = FMath::Sin(AngleRadians);
	const double Cosine = FMath::Cos(AngleRadians);
	const FVector2D RotatedPoint = Pivot + FVector2D(
		Offset.X * Cosine - Offset.Y * Sine,
		Offset.X * Sine + Offset.Y * Cosine);
	return IsFiniteVector2D(RotatedPoint) ? RotatedPoint : FVector2D::ZeroVector;
}

double UDirectiveUtilMathFunctionLibrary::SignedDistanceToPlane(const FVector& Point,
	const FVector& PlanePoint, const FVector& PlaneNormal)
{
	FVector NormalizedPlaneNormal;
	if (Point.ContainsNaN() || PlanePoint.ContainsNaN()
		|| !TryGetNormalizedVector(PlaneNormal, NormalizedPlaneNormal))
	{
		return 0.0;
	}

	const FVector Offset = Point - PlanePoint;
	if (Offset.ContainsNaN())
	{
		return 0.0;
	}

	const double MaximumComponent = Offset.GetAbsMax();
	if (MaximumComponent == 0.0)
	{
		return 0.0;
	}

	const double ScaledDistance = FVector::DotProduct(Offset / MaximumComponent, NormalizedPlaneNormal);
	if (!FMath::IsFinite(ScaledDistance)
		|| FMath::Abs(ScaledDistance) > TNumericLimits<double>::Max() / MaximumComponent)
	{
		return 0.0;
	}
	return ScaledDistance * MaximumComponent;
}

bool UDirectiveUtilMathFunctionLibrary::IsPointWithinCone(const FVector& Point, const FVector& ConeOrigin,
	const FVector& ConeDirection, const float ConeHalfAngleDegrees, const double MaximumDistance)
{
	if (Point.ContainsNaN() || ConeOrigin.ContainsNaN() || ConeDirection.ContainsNaN()
		|| !FMath::IsFinite(ConeHalfAngleDegrees) || !FMath::IsFinite(MaximumDistance))
	{
		return false;
	}

	const FVector PointDirection = Point - ConeOrigin;
	FVector NormalizedPointDirection;
	double Distance = 0.0;
	if (PointDirection.ContainsNaN()
		|| !TryGetNormalizedVector(PointDirection, NormalizedPointDirection,
			MaximumDistance > 0.0 ? &Distance : nullptr))
	{
		return false;
	}

	if (MaximumDistance > 0.0 && Distance > MaximumDistance)
	{
		return false;
	}

	return IsNormalizedDirectionWithinCone(NormalizedPointDirection, ConeDirection, ConeHalfAngleDegrees);
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(const TArray<FVector>& Locations,
	const FRotator Rotation, const FVector Scale)
{
	if (Locations.IsEmpty())
	{
		return {};
	}

	FQuat RotationQuaternion;
	if (Scale.ContainsNaN() || !TryGetRotationQuaternion(Rotation, RotationQuaternion))
	{
		return {};
	}

	TArray<FTransform> Transforms;
	Transforms.Reserve(Locations.Num());
	for (const FVector& Location : Locations)
	{
		if (Location.ContainsNaN())
		{
			return {};
		}
		Transforms.Emplace(RotationQuaternion, Location, Scale);
	}
	return Transforms;
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::LocationsToFacingTransforms(
	const TArray<FVector>& Locations, const FVector Target, const FVector UpDirection,
	const FRotator RotationOffset, const FVector Scale, const bool bFaceAway)
{
	if (Locations.IsEmpty())
	{
		return {};
	}

	FVector NormalizedUpDirection;
	FQuat RotationOffsetQuaternion;
	if (Target.ContainsNaN() || Scale.ContainsNaN()
		|| !TryGetNormalizedVector(UpDirection, NormalizedUpDirection)
		|| !TryGetRotationQuaternion(RotationOffset, RotationOffsetQuaternion))
	{
		return {};
	}

	TArray<FTransform> Transforms;
	Transforms.SetNumUninitialized(Locations.Num());
	for (int32 Index = 0; Index < Locations.Num(); ++Index)
	{
		const FVector& Location = Locations[Index];
		if (Location.ContainsNaN())
		{
			return {};
		}

		const FVector Direction = bFaceAway ? Location - Target : Target - Location;
		FQuat Rotation = RotationOffsetQuaternion;
		if (Direction.SizeSquared() > UE_DOUBLE_SMALL_NUMBER)
		{
			FVector Forward;
			if (!TryGetNormalizedVector(Direction, Forward)
				|| !TryMakeFacingRotationFromNormalized(
					Forward, NormalizedUpDirection, RotationOffsetQuaternion, Rotation))
			{
				return {};
			}
		}
		Transforms[Index] = FTransform(Rotation, Location, Scale);
	}
	return Transforms;
}

bool UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(const TArray<FVector>& Locations,
	const TArray<FRotator>& Rotations, const TArray<FVector>& Scales, TArray<FTransform>& Transforms)
{
	Transforms.Reset();
	const int32 TransformCount = Locations.Num();
	if ((Rotations.Num() != 0 && Rotations.Num() != 1 && Rotations.Num() != TransformCount)
		|| (Scales.Num() != 0 && Scales.Num() != 1 && Scales.Num() != TransformCount))
	{
		return false;
	}
	if (TransformCount == 0)
	{
		return true;
	}

	const bool bUsePerTransformRotations = Rotations.Num() == TransformCount && TransformCount > 1;
	const bool bUsePerTransformScales = Scales.Num() == TransformCount && TransformCount > 1;
	FQuat SharedRotation = FQuat::Identity;
	if (!bUsePerTransformRotations && !Rotations.IsEmpty()
		&& !TryGetRotationQuaternion(Rotations[0], SharedRotation))
	{
		return false;
	}
	const FVector SharedScale = Scales.IsEmpty() ? FVector::OneVector : Scales[0];
	if (!bUsePerTransformScales && SharedScale.ContainsNaN())
	{
		return false;
	}

	Transforms.Reserve(TransformCount);
	for (int32 Index = 0; Index < TransformCount; ++Index)
	{
		const FVector& Location = Locations[Index];
		if (Location.ContainsNaN())
		{
			Transforms.Reset();
			return false;
		}

		FQuat ItemRotation;
		const FQuat* RotationQuaternion = &SharedRotation;
		if (bUsePerTransformRotations)
		{
			if (!TryGetRotationQuaternion(Rotations[Index], ItemRotation))
			{
				Transforms.Reset();
				return false;
			}
			RotationQuaternion = &ItemRotation;
		}

		const FVector& Scale = bUsePerTransformScales ? Scales[Index] : SharedScale;
		if (bUsePerTransformScales && Scale.ContainsNaN())
		{
			Transforms.Reset();
			return false;
		}
		Transforms.Emplace(*RotationQuaternion, Location, Scale);
	}
	return true;
}

FVector UDirectiveUtilMathFunctionLibrary::SampleLocationArray(const TArray<FVector>& Locations,
	const float Alpha, const bool bClosedLoop)
{
	int32 IndexA;
	int32 IndexB;
	double SegmentAlpha;
	if (!TryFindArraySample(Locations.Num(), bClosedLoop, Alpha,
		[&Locations](const int32 Index) { return Locations[Index]; }, IndexA, IndexB, SegmentAlpha))
	{
		return FVector::ZeroVector;
	}

	const FVector Result = FMath::Lerp(Locations[IndexA], Locations[IndexB], SegmentAlpha);
	return Result.ContainsNaN() ? FVector::ZeroVector : Result;
}

FTransform UDirectiveUtilMathFunctionLibrary::SampleTransformArray(const TArray<FTransform>& Transforms,
	const float Alpha, const bool bClosedLoop)
{
	int32 IndexA;
	int32 IndexB;
	double SegmentAlpha;
	if (!TryFindArraySample(Transforms.Num(), bClosedLoop, Alpha,
		[&Transforms](const int32 Index) { return Transforms[Index].GetLocation(); },
		IndexA, IndexB, SegmentAlpha))
	{
		return FTransform::Identity;
	}

	const FTransform Result = BlendTransforms(Transforms[IndexA], Transforms[IndexB], SegmentAlpha);
	return Result.ContainsNaN() ? FTransform::Identity : Result;
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(const FVector& Origin,
	const FRotator& Rotation, const FIntPoint Dimensions, const FVector2D& Spacing, const bool bCentered)
{
	if (!IsFiniteVector2D(Spacing))
	{
		return {};
	}
	return GenerateGridPoints(Origin, Rotation, FIntVector(Dimensions.X, Dimensions.Y, 1),
		FVector(Spacing.X, Spacing.Y, 0.0), bCentered);
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GenerateGridPoints3D(const FVector& Origin,
	const FRotator& Rotation, const FIntVector Dimensions, const FVector& Spacing, const bool bCentered)
{
	return GenerateGridPoints(Origin, Rotation, Dimensions, Spacing, bCentered);
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::GenerateGridTransforms2D(const FVector& Origin,
	const FRotator& Rotation, const FIntPoint Dimensions, const FVector2D& Spacing, const bool bCentered,
	const FRotator InstanceRotation, const FVector Scale)
{
	if (!IsFiniteVector2D(Spacing))
	{
		return {};
	}
	return GenerateGridTransforms(Origin, Rotation, FIntVector(Dimensions.X, Dimensions.Y, 1),
		FVector(Spacing.X, Spacing.Y, 0.0), bCentered, InstanceRotation, Scale);
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::GenerateGridTransforms3D(const FVector& Origin,
	const FRotator& Rotation, const FIntVector Dimensions, const FVector& Spacing, const bool bCentered,
	const FRotator InstanceRotation, const FVector Scale)
{
	return GenerateGridTransforms(Origin, Rotation, Dimensions, Spacing, bCentered, InstanceRotation, Scale);
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGrid(const FVector& Origin,
	const FRotator& Rotation, const FIntPoint Dimensions, const double CellRadius,
	const EDirectiveUtilHexOrientation Orientation, const double Gap, const bool bCentered)
{
	int32 PointCount;
	FHexLayout Layout;
	if (!TryGetGridPointCount(FIntVector(Dimensions.X, Dimensions.Y, 1), PointCount)
		|| !TryMakeHexLayout(Origin, Rotation, CellRadius, Gap, Orientation, Layout)
		|| !TryApplyRectangularHexCentering(Layout, Dimensions, bCentered)
		|| !TryValidateHexLayoutSpan(Layout, GetRectangularHexSpan(Dimensions), GetRectangularHexSpan(Dimensions)))
	{
		return {};
	}

	TArray<FVector> Points;
	Points.SetNumUninitialized(PointCount);
	int32 PointIndex = 0;
	if (!VisitRectangularHexCoordinates(Dimensions, Orientation,
		[&Layout, &Points, &PointIndex](const int64 Q, const int64 R)
		{
			WriteHexLocationUnchecked(Layout, Q, R, Points[PointIndex++]);
			return true;
		}))
	{
		return {};
	}
	return Points;
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGridTransforms(const FVector& Origin,
	const FRotator& Rotation, const FIntPoint Dimensions, const double CellRadius,
	const EDirectiveUtilHexOrientation Orientation, const double Gap, const bool bCentered,
	const FRotator InstanceRotation, const FVector Scale)
{
	int32 PointCount;
	FHexLayout Layout;
	FQuat InstanceQuaternion;
	if (!IsFiniteVector(Scale)
		|| !TryGetRotationQuaternion(InstanceRotation, InstanceQuaternion)
		|| !TryGetGridPointCount(FIntVector(Dimensions.X, Dimensions.Y, 1), PointCount)
		|| !TryMakeHexLayout(Origin, Rotation, CellRadius, Gap, Orientation, Layout)
		|| !TryApplyRectangularHexCentering(Layout, Dimensions, bCentered)
		|| !TryValidateHexLayoutSpan(Layout, GetRectangularHexSpan(Dimensions), GetRectangularHexSpan(Dimensions)))
	{
		return {};
	}

	TArray<FTransform> Transforms;
	Transforms.SetNumUninitialized(PointCount);
	int32 PointIndex = 0;
	if (!VisitRectangularHexCoordinates(Dimensions, Orientation,
		[&Layout, &Transforms, &PointIndex, &InstanceQuaternion, &Scale](const int64 Q, const int64 R)
		{
			FVector Location;
			WriteHexLocationUnchecked(Layout, Q, R, Location);
			Transforms[PointIndex++] = FTransform(InstanceQuaternion, Location, Scale);
			return true;
		}))
	{
		return {};
	}
	return Transforms;
}

TArray<FIntPoint> UDirectiveUtilMathFunctionLibrary::GetRectangularHexGridCoordinates(const FIntPoint Dimensions,
	const EDirectiveUtilHexOrientation Orientation)
{
	int32 PointCount;
	if ((Orientation != EDirectiveUtilHexOrientation::PointyTop
			&& Orientation != EDirectiveUtilHexOrientation::FlatTop)
		|| !TryGetGridPointCount(FIntVector(Dimensions.X, Dimensions.Y, 1), PointCount))
	{
		return {};
	}

	TArray<FIntPoint> Coordinates;
	Coordinates.SetNumUninitialized(PointCount);
	int32 PointIndex = 0;
	if (!VisitRectangularHexCoordinates(Dimensions, Orientation,
		[&Coordinates, &PointIndex](const int64 Q, const int64 R)
		{
			return TryMakeHexCoordinate(Q, R, Coordinates[PointIndex++]);
		}))
	{
		return {};
	}
	return Coordinates;
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGrid(const FVector& Origin,
	const FRotator& Rotation, const int32 GridRadius, const double CellRadius,
	const EDirectiveUtilHexOrientation Orientation, const double Gap)
{
	int32 PointCount;
	FHexLayout Layout;
	if (!TryGetHexagonalPointCount(GridRadius, PointCount)
		|| !TryMakeHexLayout(Origin, Rotation, CellRadius, Gap, Orientation, Layout)
		|| !TryValidateHexLayoutSpan(Layout, GridRadius, GridRadius))
	{
		return {};
	}

	TArray<FVector> Points;
	Points.SetNumUninitialized(PointCount);
	int32 PointIndex = 0;
	if (!VisitHexagonalCoordinates(GridRadius,
		[&Layout, &Points, &PointIndex](const int64 Q, const int64 R)
		{
			WriteHexLocationUnchecked(Layout, Q, R, Points[PointIndex++]);
			return true;
		}))
	{
		return {};
	}
	return Points;
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGridTransforms(const FVector& Origin,
	const FRotator& Rotation, const int32 GridRadius, const double CellRadius,
	const EDirectiveUtilHexOrientation Orientation, const double Gap,
	const FRotator InstanceRotation, const FVector Scale)
{
	int32 PointCount;
	FHexLayout Layout;
	FQuat InstanceQuaternion;
	if (!IsFiniteVector(Scale)
		|| !TryGetRotationQuaternion(InstanceRotation, InstanceQuaternion)
		|| !TryGetHexagonalPointCount(GridRadius, PointCount)
		|| !TryMakeHexLayout(Origin, Rotation, CellRadius, Gap, Orientation, Layout)
		|| !TryValidateHexLayoutSpan(Layout, GridRadius, GridRadius))
	{
		return {};
	}

	TArray<FTransform> Transforms;
	Transforms.SetNumUninitialized(PointCount);
	int32 PointIndex = 0;
	if (!VisitHexagonalCoordinates(GridRadius,
		[&Layout, &Transforms, &PointIndex, &InstanceQuaternion, &Scale](const int64 Q, const int64 R)
		{
			FVector Location;
			WriteHexLocationUnchecked(Layout, Q, R, Location);
			Transforms[PointIndex++] = FTransform(InstanceQuaternion, Location, Scale);
			return true;
		}))
	{
		return {};
	}
	return Transforms;
}

FVector UDirectiveUtilMathFunctionLibrary::HexCoordinateToLocation(const FIntPoint Coordinate,
	const FVector& Origin, const FRotator& Rotation, const double CellRadius,
	const EDirectiveUtilHexOrientation Orientation, const double Gap)
{
	FHexLayout Layout;
	if (!TryMakeHexLayout(Origin, Rotation, CellRadius, Gap, Orientation, Layout))
	{
		return FVector::ZeroVector;
	}

	const FVector Location = Layout.GetLocation(Coordinate.X, Coordinate.Y);
	return Location.ContainsNaN() ? FVector::ZeroVector : Location;
}

FIntPoint UDirectiveUtilMathFunctionLibrary::LocationToHexCoordinate(const FVector& Location,
	const FVector& Origin, const FRotator& Rotation, const double CellRadius,
	const EDirectiveUtilHexOrientation Orientation, const double Gap)
{
	FHexLayout Layout;
	FIntPoint Coordinate = FIntPoint::ZeroValue;
	if (!TryMakeHexLayout(Origin, Rotation, CellRadius, Gap, Orientation, Layout)
		|| !TryGetHexCoordinate(Layout, Location, Coordinate))
	{
		return FIntPoint::ZeroValue;
	}
	return Coordinate;
}

TArray<FIntPoint> UDirectiveUtilMathFunctionLibrary::GetHexNeighbors(const FIntPoint Coordinate)
{
	TArray<FIntPoint> Neighbors;
	Neighbors.SetNumUninitialized(6);
	for (int32 Index = 0; Index < 6; ++Index)
	{
		const int64 Q = static_cast<int64>(Coordinate.X) + HexDirections[Index][0];
		const int64 R = static_cast<int64>(Coordinate.Y) + HexDirections[Index][1];
		if (!TryMakeHexCoordinate(Q, R, Neighbors[Index]))
		{
			return {};
		}
	}
	return Neighbors;
}

int64 UDirectiveUtilMathFunctionLibrary::GetHexDistance(const FIntPoint A, const FIntPoint B)
{
	const int64 DeltaQ = static_cast<int64>(A.X) - B.X;
	const int64 DeltaR = static_cast<int64>(A.Y) - B.Y;
	return FMath::Max3(FMath::Abs(DeltaQ), FMath::Abs(DeltaR), FMath::Abs(DeltaQ + DeltaR));
}

TArray<FIntPoint> UDirectiveUtilMathFunctionLibrary::GetHexesInRange(const FIntPoint Center, const int32 Range)
{
	int32 PointCount;
	if (!TryGetHexagonalPointCount(Range, PointCount))
	{
		return {};
	}

	TArray<FIntPoint> Hexes;
	Hexes.SetNumUninitialized(PointCount);
	int32 PointIndex = 0;
	if (!VisitHexagonalCoordinates(Range,
		[&Center, &Hexes, &PointIndex](const int64 DeltaQ, const int64 DeltaR)
		{
			return TryMakeHexCoordinate(Center.X + DeltaQ, Center.Y + DeltaR, Hexes[PointIndex++]);
		}))
	{
		return {};
	}
	return Hexes;
}

TArray<FIntPoint> UDirectiveUtilMathFunctionLibrary::GetHexRing(const FIntPoint Center, const int32 Radius)
{
	const int64 PointCount = static_cast<int64>(Radius) * 6;
	if (Radius < 0 || (Radius > 0 && !IsSupportedGeneratedElementCount(PointCount)))
	{
		return {};
	}
	if (Radius == 0)
	{
		return { Center };
	}

	TArray<FIntPoint> Ring;
	Ring.SetNumUninitialized(static_cast<int32>(PointCount));
	int32 PointIndex = 0;
	int64 Q = static_cast<int64>(Center.X) + static_cast<int64>(HexDirections[4][0]) * Radius;
	int64 R = static_cast<int64>(Center.Y) + static_cast<int64>(HexDirections[4][1]) * Radius;
	for (int32 Side = 0; Side < 6; ++Side)
	{
		for (int32 Step = 0; Step < Radius; ++Step)
		{
			if (!TryMakeHexCoordinate(Q, R, Ring[PointIndex++]))
			{
				return {};
			}
			Q += HexDirections[Side][0];
			R += HexDirections[Side][1];
		}
	}
	return Ring;
}

TArray<FIntPoint> UDirectiveUtilMathFunctionLibrary::GetHexLine(const FIntPoint Start, const FIntPoint End)
{
	const int64 Distance = GetHexDistance(Start, End);
	if (!IsSupportedGeneratedElementCount(Distance + 1))
	{
		return {};
	}

	TArray<FIntPoint> Line;
	Line.SetNumUninitialized(static_cast<int32>(Distance) + 1);
	Line[0] = Start;
	if (Distance == 0)
	{
		return Line;
	}
	Line.Last() = End;

	// Nudging both endpoints off the cell edges makes ties round to a consistent side.
	constexpr double Nudge = 1.e-6;
	const double StartQ = static_cast<double>(Start.X) + Nudge;
	const double StartR = static_cast<double>(Start.Y) + Nudge;
	const double EndQ = static_cast<double>(End.X) + Nudge;
	const double EndR = static_cast<double>(End.Y) + Nudge;
	for (int64 Step = 1; Step < Distance; ++Step)
	{
		const double Alpha = static_cast<double>(Step) / static_cast<double>(Distance);
		FIntPoint Coordinate;
		if (!TryRoundHexCoordinate(
			FMath::Lerp(StartQ, EndQ, Alpha), FMath::Lerp(StartR, EndR, Alpha), Coordinate))
		{
			return {};
		}
		Line[static_cast<int32>(Step)] = Coordinate;
	}
	return Line;
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GetHexCellCorners(const FIntPoint Coordinate,
	const FVector& Origin, const FRotator& Rotation, const double CellRadius,
	const EDirectiveUtilHexOrientation Orientation, const double Gap)
{
	FHexLayout Layout;
	if (!TryMakeHexLayout(Origin, Rotation, CellRadius, Gap, Orientation, Layout))
	{
		return {};
	}

	const FVector Center = Layout.GetLocation(Coordinate.X, Coordinate.Y);
	if (Center.ContainsNaN())
	{
		return {};
	}

	const double StartAngle = Orientation == EDirectiveUtilHexOrientation::PointyTop
		? UE_DOUBLE_PI / 6.0
		: 0.0;
	TArray<FVector> Corners;
	Corners.SetNumUninitialized(6);
	for (int32 Index = 0; Index < 6; ++Index)
	{
		const double Angle = StartAngle + Index * (UE_DOUBLE_PI / 3.0);
		const FVector Corner = Center
			+ (Layout.AxisX * FMath::Cos(Angle) + Layout.AxisY * FMath::Sin(Angle)) * CellRadius;
		if (Corner.ContainsNaN())
		{
			return {};
		}
		Corners[Index] = Corner;
	}
	return Corners;
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(const FVector& Origin,
	const FVector& Direction, const int32 Count, const double Spacing, const bool bCentered)
{
	FVector NormalizedDirection;
	if (Count <= 0 || !FMath::IsFinite(Spacing) || Origin.ContainsNaN()
		|| !TryGetNormalizedVector(Direction, NormalizedDirection))
	{
		return {};
	}

	const FVector Step = NormalizedDirection * Spacing;
	const double FirstStep = bCentered ? -0.5 * static_cast<double>(Count - 1) : 0.0;
	return GenerateLinearPoints(Origin, Step, Count, FirstStep);
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(const FVector& Start,
	const FVector& End, const int32 Count, const bool bIncludeEndpoints)
{
	if (Count <= 0 || Start.ContainsNaN() || End.ContainsNaN())
	{
		return {};
	}
	if (Count == 1)
	{
		return MakeSinglePoint(Start * 0.5 + End * 0.5);
	}

	const FVector Delta = End - Start;
	if (Delta.ContainsNaN())
	{
		return {};
	}
	const double Divisor = bIncludeEndpoints ? static_cast<double>(Count - 1) : static_cast<double>(Count) + 1.0;
	TArray<FVector> Points = GenerateLinearPoints(Start, Delta / Divisor, Count, bIncludeEndpoints ? 0.0 : 1.0);
	if (bIncludeEndpoints && Points.Num() == Count)
	{
		Points[0] = Start;
		Points.Last() = End;
	}
	return Points;
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(const USplineComponent* Spline,
	const double Spacing, const bool bIncludeEndpoint, const EDirectiveUtilSplineSpacingMode SpacingMode,
	const ESplineCoordinateSpace::Type CoordinateSpace, const double StartDistance, const double EndDistance)
{
	FSplineSamplePlan Plan;
	if (!IsValidSplineCoordinateSpace(CoordinateSpace)
		|| !TryMakeSplineSpacingPlan(Spline, Spacing, SpacingMode, bIncludeEndpoint,
			StartDistance, EndDistance, Plan))
	{
		return {};
	}
	return SampleSplineLocations(Spline, Plan, CoordinateSpace);
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSplineByCount(
	const USplineComponent* Spline, const int32 Count, const bool bIncludeEndpoints,
	const ESplineCoordinateSpace::Type CoordinateSpace, const double StartDistance, const double EndDistance)
{
	FSplineSamplePlan Plan;
	if (!IsValidSplineCoordinateSpace(CoordinateSpace)
		|| !TryMakeSplineCountPlan(Spline, Count, bIncludeEndpoints, StartDistance, EndDistance, Plan))
	{
		return {};
	}
	return SampleSplineLocations(Spline, Plan, CoordinateSpace);
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSpline(
	const USplineComponent* Spline, const double Spacing, const bool bIncludeEndpoint,
	const EDirectiveUtilSplineSpacingMode SpacingMode, const ESplineCoordinateSpace::Type CoordinateSpace,
	const bool bUseSplineScale, const FRotator RotationOffset, const FVector ScaleMultiplier,
	const double StartDistance, const double EndDistance)
{
	FSplineSamplePlan Plan;
	FQuat RotationOffsetQuaternion;
	if (ScaleMultiplier.ContainsNaN()
		|| !IsValidSplineCoordinateSpace(CoordinateSpace)
		|| !TryGetRotationQuaternion(RotationOffset, RotationOffsetQuaternion)
		|| !TryMakeSplineSpacingPlan(Spline, Spacing, SpacingMode, bIncludeEndpoint,
			StartDistance, EndDistance, Plan))
	{
		return {};
	}
	return SampleSplineTransforms(Spline, Plan, CoordinateSpace, bUseSplineScale,
		RotationOffsetQuaternion, ScaleMultiplier);
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSplineByCount(
	const USplineComponent* Spline, const int32 Count, const bool bIncludeEndpoints,
	const ESplineCoordinateSpace::Type CoordinateSpace, const bool bUseSplineScale,
	const FRotator RotationOffset, const FVector ScaleMultiplier,
	const double StartDistance, const double EndDistance)
{
	FSplineSamplePlan Plan;
	FQuat RotationOffsetQuaternion;
	if (ScaleMultiplier.ContainsNaN()
		|| !IsValidSplineCoordinateSpace(CoordinateSpace)
		|| !TryGetRotationQuaternion(RotationOffset, RotationOffsetQuaternion)
		|| !TryMakeSplineCountPlan(Spline, Count, bIncludeEndpoints, StartDistance, EndDistance, Plan))
	{
		return {};
	}
	return SampleSplineTransforms(Spline, Plan, CoordinateSpace, bUseSplineScale,
		RotationOffsetQuaternion, ScaleMultiplier);
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(const FVector& Center,
	const FRotator& Rotation, const double Radius, const int32 Count, const double StartAngleDegrees)
{
	if (Count <= 0 || !FMath::IsFinite(Radius) || !FMath::IsFinite(StartAngleDegrees))
	{
		return {};
	}
	const double AbsoluteRadius = FMath::Abs(Radius);
	return GeneratePlanarRadialPoints(Center, Rotation, Count, WrapDegreesAsRadians(StartAngleDegrees),
		UE_DOUBLE_TWO_PI / Count, [AbsoluteRadius](const int32) { return AbsoluteRadius; });
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnCircle(const FVector& Center,
	const FRotator& Rotation, const double Radius, const int32 Count, const double StartAngleDegrees,
	const EDirectiveUtilRadialOrientation Orientation, const FRotator RotationOffset, const FVector Scale)
{
	if (Count <= 0 || !FMath::IsFinite(StartAngleDegrees))
	{
		return {};
	}
	return GeneratePlanarRadialTransforms(Center, Rotation, Radius, Count,
		WrapDegreesAsRadians(StartAngleDegrees), UE_DOUBLE_TWO_PI / Count, 1.0,
		Orientation, RotationOffset, Scale);
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GeneratePointsOnArc(const FVector& Center,
	const FRotator& Rotation, const double Radius, const int32 Count, const double StartAngleDegrees,
	const double ArcAngleDegrees, const bool bIncludeEndpoint)
{
	if (Count <= 0 || !FMath::IsFinite(Radius) || !FMath::IsFinite(StartAngleDegrees)
		|| !FMath::IsFinite(ArcAngleDegrees))
	{
		return {};
	}

	const double Divisor = bIncludeEndpoint && Count > 1 ? Count - 1.0 : static_cast<double>(Count);
	const double StepAngle = Count == 1 ? 0.0 : WrapDegreesAsRadians(ArcAngleDegrees / Divisor);
	const double AbsoluteRadius = FMath::Abs(Radius);
	return GeneratePlanarRadialPoints(Center, Rotation, Count, WrapDegreesAsRadians(StartAngleDegrees),
		StepAngle, [AbsoluteRadius](const int32) { return AbsoluteRadius; });
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnArc(const FVector& Center,
	const FRotator& Rotation, const double Radius, const int32 Count, const double StartAngleDegrees,
	const double ArcAngleDegrees, const bool bIncludeEndpoint,
	const EDirectiveUtilRadialOrientation Orientation, const FRotator RotationOffset, const FVector Scale)
{
	if (Count <= 0 || !FMath::IsFinite(StartAngleDegrees) || !FMath::IsFinite(ArcAngleDegrees))
	{
		return {};
	}

	const double Divisor = bIncludeEndpoint && Count > 1 ? Count - 1.0 : static_cast<double>(Count);
	const double StepAngle = Count == 1 ? 0.0 : WrapDegreesAsRadians(ArcAngleDegrees / Divisor);
	return GeneratePlanarRadialTransforms(Center, Rotation, Radius, Count,
		WrapDegreesAsRadians(StartAngleDegrees), StepAngle, ArcAngleDegrees,
		Orientation, RotationOffset, Scale);
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GeneratePointsOnDisc(const FVector& Center,
	const FRotator& Rotation, const double Radius, const int32 Count, const double AngleOffsetDegrees)
{
	if (Count <= 0 || !FMath::IsFinite(Radius) || !FMath::IsFinite(AngleOffsetDegrees))
	{
		return {};
	}
	if (Count == 1)
	{
		return MakeSinglePoint(Center);
	}

	const double AbsoluteRadius = FMath::Abs(Radius);
	const double InverseCount = 1.0 / Count;
	const double GoldenAngle = UE_DOUBLE_PI * (3.0 - FMath::Sqrt(5.0));
	return GeneratePlanarRadialPoints(Center, Rotation, Count, WrapDegreesAsRadians(AngleOffsetDegrees),
		GoldenAngle, [AbsoluteRadius, InverseCount](const int32 Index)
		{
			return AbsoluteRadius * FMath::Sqrt((Index + 0.5) * InverseCount);
		});
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(const FVector& Center,
	const FRotator& Rotation, const double Radius, const int32 Count, const double AngleOffsetDegrees)
{
	if (!IsSupportedGeneratedElementCount(Count)
		|| Center.ContainsNaN() || !FMath::IsFinite(Radius) || !FMath::IsFinite(AngleOffsetDegrees))
	{
		return {};
	}

	FVector AxisX;
	FVector AxisY;
	FVector AxisZ;
	if (!TryGetRotatedAxes(Rotation, AxisX, AxisY, AxisZ))
	{
		return {};
	}

	const double AbsoluteRadius = FMath::Abs(Radius);
	if (Count == 1)
	{
		return MakeSinglePoint(Center + AxisZ * AbsoluteRadius);
	}

	TArray<FVector> Points;
	Points.SetNumUninitialized(Count);
	const double InverseCount = 1.0 / Count;
	const double GoldenAngle = UE_DOUBLE_PI * (3.0 - FMath::Sqrt(5.0));
	FAngleStepper Angle(WrapDegreesAsRadians(AngleOffsetDegrees), GoldenAngle);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const double Z = 1.0 - 2.0 * (Index + 0.5) * InverseCount;
		const double RadialScale = FMath::Sqrt(FMath::Max(0.0, 1.0 - Z * Z));
		const FVector Point = Center + (AxisX * (Angle.Cosine * RadialScale)
			+ AxisY * (Angle.Sine * RadialScale) + AxisZ * Z) * AbsoluteRadius;
		if (Point.ContainsNaN())
		{
			return {};
		}
		Points[Index] = Point;
		Angle.Advance();
	}
	return Points;
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::OffsetLocationsByNoise(const TArray<FVector>& Locations,
	const double NoiseScale, const double Amplitude, const FVector Direction, const FVector NoiseOffset)
{
	FVector NormalizedDirection;
	if (!TryGetNoiseOffsetInputs(NoiseScale, Amplitude, Direction, NoiseOffset, NormalizedDirection))
	{
		return {};
	}

	TArray<FVector> Result;
	Result.SetNumUninitialized(Locations.Num());
	for (int32 Index = 0; Index < Locations.Num(); ++Index)
	{
		if (!TryOffsetLocationByNoise(Locations[Index], NormalizedDirection, NoiseScale, Amplitude,
			NoiseOffset, Result[Index]))
		{
			return {};
		}
	}
	return Result;
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::OffsetTransformsByNoise(
	const TArray<FTransform>& Transforms, const double NoiseScale, const double Amplitude,
	const FVector Direction, const FVector NoiseOffset)
{
	FVector NormalizedDirection;
	if (!TryGetNoiseOffsetInputs(NoiseScale, Amplitude, Direction, NoiseOffset, NormalizedDirection))
	{
		return {};
	}

	TArray<FTransform> Result;
	Result.SetNumUninitialized(Transforms.Num());
	for (int32 Index = 0; Index < Transforms.Num(); ++Index)
	{
		FVector OffsetLocation;
		if (!TryOffsetLocationByNoise(Transforms[Index].GetLocation(), NormalizedDirection, NoiseScale,
				Amplitude, NoiseOffset, OffsetLocation))
		{
			return {};
		}
		Result[Index] = Transforms[Index];
		Result[Index].SetLocation(OffsetLocation);
	}
	return Result;
}
