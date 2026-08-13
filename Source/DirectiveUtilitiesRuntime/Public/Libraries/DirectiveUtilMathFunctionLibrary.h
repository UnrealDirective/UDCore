// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilMathTypes.h"
#include "DirectiveUtilMathFunctionLibrary.generated.h"

/**
 * UDirectiveUtilMathFunctionLibrary
 *
 * Contains math functions for the Directive Utilities plugin.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilMathFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr int32 MaximumGeneratedElementCount = 1000000;

	/**
	* Returns a perlin noise value between -1 and 1 at the given position.
	* @note This exposes the built-in PerlinNoise2D function to blueprints.
	* @param Position - The position to get the noise value for.
	* @returns The noise value at the given position.
	*/
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Random")
	static float PerlinNoise2D(FVector2D Position);

	/**
	* Returns a perlin noise value between -1 and 1 at the given position.
	* @note This exposes the built-in PerlinNoise3D function to blueprints.
	* @param Position - The position to get the noise value for.
	* @returns The noise value at the given position.
	*/
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Random")
	static float PerlinNoise3D(const FVector& Position);

	/**
	 * Returns the angle in degrees between two vectors.
	 * @param A - The first vector.
	 * @param B - The second vector.
	 * @returns The angle between the two vectors in degrees, or 0 if either
	 *          vector is zero or non-finite.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Vector")
	static float AngleBetweenVectors(const FVector& A, const FVector& B);

	/**
	 * Returns the signed angle in degrees from one vector to another around an axis.
	 * The vectors are projected onto the plane perpendicular to the axis before measuring.
	 * @param From - The starting direction.
	 * @param To - The target direction.
	 * @param Axis - The axis that defines the rotation plane and positive direction.
	 * @returns The signed angle in the [-180, 180] range, or 0 if an input cannot define a direction.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Signed Angle Between Vectors", BlueprintThreadSafe), Category = "Directive Utilities|Math|Vector")
	static float SignedAngleBetweenVectors(const FVector& From, const FVector& To, const FVector& Axis);

	/**
	 * Returns the shortest signed difference in degrees from one angle to another.
	 * Exactly opposite angles always return +180, regardless of how the inputs are spelled.
	 * @param From - The starting angle in degrees.
	 * @param To - The target angle in degrees.
	 * @returns The signed difference in the (-180, 180] range, or 0 for non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Delta Angle (Degrees)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static float DeltaAngle(float From, float To);

	/**
	 * Interpolates between two angles along the shortest path.
	 * Alpha 0 returns A. Values outside [0, 1] extrapolate along that same
	 * shortest-path direction without wrapping, so a timeline past the end
	 * does not jump the seam.
	 * @param A - The starting angle in degrees.
	 * @param B - The target angle in degrees.
	 * @param Alpha - The interpolation alpha. Values outside [0, 1] extrapolate.
	 * @returns A plus the shortest signed delta to B, scaled by Alpha, or 0 for non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Lerp Angle (Degrees)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static float LerpAngle(float A, float B, float Alpha);

	/**
	 * Repeats a value between two bounds, reversing direction at each bound.
	 * @param Value - The value to repeat.
	 * @param Minimum - One range bound.
	 * @param Maximum - The other range bound.
	 * @returns The ping-ponged value, the shared bound for a zero-sized range, or 0 for non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ping Pong (Float)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static float PingPong(float Value, float Minimum = 0.0f, float Maximum = 1.0f);

	/**
	 * Applies cubic smoothing to a value between two bounds.
	 * @returns A value in the [0, 1] range, or 0 for non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Smooth Step", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static float SmoothStep(float Value, float Minimum = 0.0f, float Maximum = 1.0f);

	/**
	 * Applies quintic smoothing to a value between two bounds.
	 * @returns A value in the [0, 1] range, or 0 for non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Smoother Step", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static float SmootherStep(float Value, float Minimum = 0.0f, float Maximum = 1.0f);

	/**
	 * Returns a normalized falloff between an inner and outer radius.
	 * @returns 1 at or inside the inner radius, 0 beyond the outer radius, or 0 for non-finite input.
	 *          Equal radii are a step: 1 at or inside the shared radius, 0 outside.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Range Falloff", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static float RangeFalloff(float Distance, float InnerRadius, float OuterRadius, float FalloffExponent = 1.0f);

	/**
	 * Tests whether a direction lies within a cone centered on another direction.
	 * @param Direction - The direction to test.
	 * @param ConeDirection - The center direction of the cone.
	 * @param ConeHalfAngleDegrees - The angle from the cone center to its edge. Clamped to [0, 180].
	 * @returns True when the direction lies inside or on the cone, or false for an invalid direction or angle.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Direction Within Cone", BlueprintThreadSafe), Category = "Directive Utilities|Math|Vector")
	static bool IsDirectionWithinCone(const FVector& Direction, const FVector& ConeDirection, float ConeHalfAngleDegrees);

	/**
	 * Calculates the normalized direction and distance from one point to another.
	 * @returns False when the points are equal or an input is non-finite.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Direction And Distance", BlueprintThreadSafe), Category = "Directive Utilities|Math|Vector")
	static bool GetDirectionAndDistance(const FVector& From, const FVector& To, FVector& Direction, double& Distance);

	/**
	 * Rotates a 2D point around a pivot in degrees.
	 * @returns The rotated point, or zero for non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Rotate Point Around Pivot 2D", BlueprintThreadSafe), Category = "Directive Utilities|Math|Vector")
	static FVector2D RotatePointAroundPivot2D(const FVector2D& Point, const FVector2D& Pivot, float AngleDegrees);

	/**
	 * Calculates the signed distance from a point to a plane.
	 * @returns The signed distance, or 0 when the plane normal is zero or an input is non-finite.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Signed Distance To Plane", BlueprintThreadSafe), Category = "Directive Utilities|Math|Vector")
	static double SignedDistanceToPlane(const FVector& Point, const FVector& PlanePoint, const FVector& PlaneNormal);

	/**
	 * Tests whether a point lies within a cone and optional maximum distance.
	 * @returns True when the point lies inside or on the cone and within the distance limit.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Point Within Cone", BlueprintThreadSafe), Category = "Directive Utilities|Math|Vector")
	static bool IsPointWithinCone(const FVector& Point, const FVector& ConeOrigin, const FVector& ConeDirection,
		float ConeHalfAngleDegrees, double MaximumDistance = 0.0);

	/**
	 * Samples a location along the polyline through an array, with Alpha 0 at the first point and 1 at the last.
	 * Progress is distance-weighted, so equal alpha steps cover equal distance.
	 * A closed loop adds the segment from the last point back to the first and wraps Alpha instead of clamping it.
	 * @returns The sampled location, or the zero vector for an empty array or non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Sample Location Array", BlueprintThreadSafe), Category = "Directive Utilities|Math|Vector")
	static FVector SampleLocationArray(const TArray<FVector>& Locations, float Alpha, bool bClosedLoop = false);

	/** Creates one transform per location using a shared rotation and scale. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Locations To Transforms", BlueprintThreadSafe), Category = "Directive Utilities|Math|Transform")
	static TArray<FTransform> LocationsToTransforms(const TArray<FVector>& Locations,
		FRotator Rotation = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));

	/**
	 * Creates one transform per location with its local X axis facing toward or away from a target.
	 * A location equal to Target uses Rotation Offset without a facing rotation.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Locations To Facing Transforms", BlueprintThreadSafe, AdvancedDisplay = "UpDirection,RotationOffset,Scale,bFaceAway"), Category = "Directive Utilities|Math|Transform")
	static TArray<FTransform> LocationsToFacingTransforms(const TArray<FVector>& Locations,
		FVector Target, FVector UpDirection = FVector(0.0, 0.0, 1.0),
		FRotator RotationOffset = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0),
		bool bFaceAway = false);

	/**
	 * Creates transforms from location, rotation, and scale arrays.
	 * Rotation and scale arrays may be empty, contain one value to broadcast, or match the location count.
	 * @returns True when the attribute-array lengths and values are valid.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Make Transforms From Arrays", AutoCreateRefTerm = "Rotations,Scales", BlueprintThreadSafe), Category = "Directive Utilities|Math|Transform")
	static bool MakeTransformsFromArrays(const TArray<FVector>& Locations, const TArray<FRotator>& Rotations,
		const TArray<FVector>& Scales, TArray<FTransform>& Transforms);

	/**
	 * Samples a transform along the path through an array, with Alpha 0 at the first transform and 1 at the last.
	 * Progress is distance-weighted by location. Rotation takes the shortest path and scale interpolates linearly.
	 * A closed loop adds the segment from the last transform back to the first and wraps Alpha instead of clamping it.
	 * @returns The sampled transform, or the identity for an empty array or non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Sample Transform Array", BlueprintThreadSafe), Category = "Directive Utilities|Math|Transform")
	static FTransform SampleTransformArray(const TArray<FTransform>& Transforms, float Alpha, bool bClosedLoop = false);

	/**
	 * Generates a rectangular grid on the local XY plane.
	 * @param Origin - The first point, or the grid center when Centered is true.
	 * @param Rotation - The grid plane rotation.
	 * @param Dimensions - The number of points along the local X and Y axes.
	 * @param Spacing - The signed center-to-center spacing along the local X and Y axes.
	 * @param bCentered - Whether to center the grid on Origin.
	 * @returns Points ordered by X, then Y, or an empty array for invalid input or an unsupported point count.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Grid Points 2D"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GenerateGridPoints2D(const FVector& Origin, const FRotator& Rotation,
		FIntPoint Dimensions, const FVector2D& Spacing, bool bCentered = true);

	/**
	 * Generates a rectangular 3D grid.
	 * @param Origin - The first point, or the grid center when Centered is true.
	 * @param Rotation - The grid rotation.
	 * @param Dimensions - The number of points along the local X, Y, and Z axes.
	 * @param Spacing - The signed center-to-center spacing along the local X, Y, and Z axes.
	 * @param bCentered - Whether to center the grid on Origin.
	 * @returns Points ordered by X, then Y, then Z, or an empty array for invalid input or an unsupported point count.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Grid Points 3D"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GenerateGridPoints3D(const FVector& Origin, const FRotator& Rotation,
		FIntVector Dimensions, const FVector& Spacing, bool bCentered = true);

	/**
	 * Generates transforms on a rectangular grid on the local XY plane.
	 * @param Origin - The first location, or the grid center when Centered is true.
	 * @param Rotation - The grid plane rotation.
	 * @param Dimensions - The number of points along the local X and Y axes.
	 * @param Spacing - The signed center-to-center spacing along the local X and Y axes.
	 * @param bCentered - Whether to center the grid on Origin.
	 * @param InstanceRotation - Shared rotation applied to every transform.
	 * @param Scale - Shared scale applied to every transform.
	 * @returns Transforms ordered by X, then Y, or an empty array for invalid input or an unsupported count.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Grid Transforms 2D", AdvancedDisplay = "InstanceRotation,Scale"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FTransform> GenerateGridTransforms2D(const FVector& Origin, const FRotator& Rotation,
		FIntPoint Dimensions, const FVector2D& Spacing, bool bCentered = true,
		FRotator InstanceRotation = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));

	/**
	 * Generates transforms on a rectangular 3D grid.
	 * @param Origin - The first location, or the grid center when Centered is true.
	 * @param Rotation - The grid rotation.
	 * @param Dimensions - The number of points along the local X, Y, and Z axes.
	 * @param Spacing - The signed center-to-center spacing along the local X, Y, and Z axes.
	 * @param bCentered - Whether to center the grid on Origin.
	 * @param InstanceRotation - Shared rotation applied to every transform.
	 * @param Scale - Shared scale applied to every transform.
	 * @returns Transforms ordered by X, then Y, then Z, or an empty array for invalid input or an unsupported count.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Grid Transforms 3D", AdvancedDisplay = "InstanceRotation,Scale"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FTransform> GenerateGridTransforms3D(const FVector& Origin, const FRotator& Rotation,
		FIntVector Dimensions, const FVector& Spacing, bool bCentered = true,
		FRotator InstanceRotation = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));

	/**
	 * Generates a rectangular hex grid on the rotated local XY plane.
	 * @param Origin - The first cell center, or the grid bounds center when Centered is true.
	 * @param Rotation - The grid plane rotation.
	 * @param Dimensions - The number of columns and rows.
	 * @param CellRadius - The distance from a cell center to a corner. Must be positive.
	 * @param Orientation - Whether the hex cells have pointy or flat tops.
	 * @param Gap - The signed edge-to-edge gap between adjacent cells. Negative values overlap cells.
	 * @param bCentered - Whether to center the grid bounds on Origin.
	 * @returns Points ordered by row, then column, or an empty array for invalid input or an unsupported point count.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Rectangular Hex Grid"), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FVector> GenerateRectangularHexGrid(const FVector& Origin, const FRotator& Rotation,
		FIntPoint Dimensions, double CellRadius,
		EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
		double Gap = 0.0, bool bCentered = true);

	/**
	 * Generates transforms for a rectangular hex grid with a shared instance rotation and scale.
	 * Cell order matches Generate Rectangular Hex Grid.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Rectangular Hex Grid Transforms", AdvancedDisplay = "InstanceRotation,Scale"), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FTransform> GenerateRectangularHexGridTransforms(const FVector& Origin, const FRotator& Rotation,
		FIntPoint Dimensions, double CellRadius,
		EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
		double Gap = 0.0, bool bCentered = true,
		FRotator InstanceRotation = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));

	/**
	 * Returns the axial coordinate of every cell of a rectangular hex grid, in the same cell order as
	 * Generate Rectangular Hex Grid.
	 * @returns The coordinates, or an empty array for invalid input or an unsupported count.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Rectangular Hex Grid Coordinates"), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FIntPoint> GetRectangularHexGridCoordinates(FIntPoint Dimensions,
		EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop);

	/**
	 * Generates a hexagon-shaped grid on the rotated local XY plane.
	 * @param Origin - The center cell location.
	 * @param Rotation - The grid plane rotation.
	 * @param GridRadius - The number of cell rings around the center cell.
	 * @param CellRadius - The distance from a cell center to a corner. Must be positive.
	 * @param Orientation - Whether the hex cells have pointy or flat tops.
	 * @param Gap - The signed edge-to-edge gap between adjacent cells. Negative values overlap cells.
	 * @returns Points ordered by axial R, then Q, or an empty array for invalid input or an unsupported point count.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Hexagonal Hex Grid"), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FVector> GenerateHexagonalHexGrid(const FVector& Origin, const FRotator& Rotation,
		int32 GridRadius, double CellRadius,
		EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
		double Gap = 0.0);

	/**
	 * Generates transforms for a hexagon-shaped grid with a shared instance rotation and scale.
	 * Cell order matches Generate Hexagonal Hex Grid.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Hexagonal Hex Grid Transforms", AdvancedDisplay = "InstanceRotation,Scale"), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FTransform> GenerateHexagonalHexGridTransforms(const FVector& Origin, const FRotator& Rotation,
		int32 GridRadius, double CellRadius,
		EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
		double Gap = 0.0,
		FRotator InstanceRotation = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));

	/**
	 * Converts an axial hex coordinate to a location on the rotated local XY plane.
	 * @returns The cell center, or the zero vector for invalid layout input or coordinate overflow.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Hex Coordinate To Location", BlueprintThreadSafe), Category = "Directive Utilities|Math|Hex Grid")
	static FVector HexCoordinateToLocation(FIntPoint Coordinate, const FVector& Origin, const FRotator& Rotation,
		double CellRadius, EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
		double Gap = 0.0);

	/**
	 * Finds the axial coordinate of the nearest hex after projecting a location onto the rotated local XY plane.
	 * @returns The nearest axial coordinate, or (0, 0) for invalid layout input or an unrepresentable coordinate.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Location To Hex Coordinate", BlueprintThreadSafe), Category = "Directive Utilities|Math|Hex Grid")
	static FIntPoint LocationToHexCoordinate(const FVector& Location, const FVector& Origin,
		const FRotator& Rotation, double CellRadius,
		EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
		double Gap = 0.0);

	/**
	 * Returns the six adjacent axial coordinates in a stable direction order.
	 * @returns Six neighbors, or an empty array when a neighbor would exceed the FIntPoint range.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Hex Neighbors", BlueprintThreadSafe), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FIntPoint> GetHexNeighbors(FIntPoint Coordinate);

	/** Returns the number of hex-grid steps between two axial coordinates. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Hex Distance", BlueprintThreadSafe), Category = "Directive Utilities|Math|Hex Grid")
	static int64 GetHexDistance(FIntPoint A, FIntPoint B);

	/**
	 * Returns every axial coordinate within a number of steps of a center cell, ordered by axial R, then Q.
	 * With a zero center the order matches the cells of Generate Hexagonal Hex Grid.
	 * @returns The coordinates, or an empty array for a negative range, coordinate overflow, or an unsupported count.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Hexes In Range"), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FIntPoint> GetHexesInRange(FIntPoint Center, int32 Range);

	/**
	 * Returns the axial coordinates exactly Radius steps from a center cell.
	 * Consecutive entries are adjacent and trace the ring once. A radius of zero returns the center.
	 * @returns The ring coordinates, or an empty array for a negative radius or coordinate overflow.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Hex Ring"), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FIntPoint> GetHexRing(FIntPoint Center, int32 Radius);

	/**
	 * Returns the axial coordinates along the straight line between two cells, including both endpoints.
	 * @returns The line coordinates, or an empty array for an unsupported length.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Hex Line"), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FIntPoint> GetHexLine(FIntPoint Start, FIntPoint End);

	/**
	 * Returns the six corner locations of a hex cell on the rotated local XY plane, ordered counter-clockwise.
	 * Corners lie at Cell Radius from the cell center; Gap only moves the center.
	 * @returns The corner locations, or an empty array for invalid layout input or coordinate overflow.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Hex Cell Corners", BlueprintThreadSafe), Category = "Directive Utilities|Math|Hex Grid")
	static TArray<FVector> GetHexCellCorners(FIntPoint Coordinate, const FVector& Origin, const FRotator& Rotation,
		double CellRadius, EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
		double Gap = 0.0);

	/**
	 * Generates points at a fixed spacing along a direction.
	 * @param Origin - The first point, or the formation center when Centered is true.
	 * @param Direction - The direction of travel. Its magnitude is ignored.
	 * @param Count - The number of points to generate.
	 * @param Spacing - The signed center-to-center distance between points.
	 * @param bCentered - Whether to center the formation on Origin.
	 * @returns The generated points, or an empty array for invalid input.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Points Along Direction"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GeneratePointsAlongDirection(const FVector& Origin, const FVector& Direction,
		int32 Count, double Spacing, bool bCentered = false);

	/**
	 * Generates evenly spaced points between two locations.
	 * @param Start - The start of the segment.
	 * @param End - The end of the segment.
	 * @param Count - The number of points to generate.
	 * @param bIncludeEndpoints - Whether the generated points include Start and End.
	 * @returns The generated points, or an empty array for invalid input.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Points Between Locations"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GeneratePointsBetweenLocations(const FVector& Start, const FVector& End,
		int32 Count, bool bIncludeEndpoints = true);

	/**
	 * Generates points at fixed distances along a spline.
	 * @param Spline - The spline to sample.
	 * @param Spacing - The distance between regular samples. Must be positive.
	 * @param bIncludeEndpoint - Whether to append the exact end of an open sampling range.
	 * @param SpacingMode - Fixed samples every Spacing units. Even shrinks the spacing so the samples divide the range evenly.
	 * @param CoordinateSpace - The space of the returned points.
	 * @param StartDistance - The distance where sampling starts. Clamped to the spline length.
	 * @param EndDistance - The distance where sampling ends. Negative means the end of the spline.
	 * @returns The generated points, or an empty array for invalid input or an unsupported point count.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Points Along Spline", AdvancedDisplay = "SpacingMode,CoordinateSpace,StartDistance,EndDistance"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GeneratePointsAlongSpline(const USplineComponent* Spline, double Spacing,
		bool bIncludeEndpoint = true,
		EDirectiveUtilSplineSpacingMode SpacingMode = EDirectiveUtilSplineSpacingMode::Fixed,
		ESplineCoordinateSpace::Type CoordinateSpace = ESplineCoordinateSpace::World,
		double StartDistance = 0.0, double EndDistance = -1.0);

	/**
	 * Generates a fixed number of evenly spaced points along a spline.
	 * A closed loop spreads the points around the loop; a count of one on an open range returns its midpoint.
	 * @param Count - The number of points to generate.
	 * @param bIncludeEndpoints - Whether the points include both ends of an open sampling range.
	 * @param StartDistance - The distance where sampling starts. Clamped to the spline length.
	 * @param EndDistance - The distance where sampling ends. Negative means the end of the spline.
	 * @returns The generated points, or an empty array for invalid input.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Points Along Spline by Count", AdvancedDisplay = "StartDistance,EndDistance"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GeneratePointsAlongSplineByCount(const USplineComponent* Spline, int32 Count,
		bool bIncludeEndpoints = true,
		ESplineCoordinateSpace::Type CoordinateSpace = ESplineCoordinateSpace::World,
		double StartDistance = 0.0, double EndDistance = -1.0);

	/**
	 * Generates transforms at fixed distances along a spline.
	 * Rotation follows the spline tangent and roll. Scale can include the spline scale before applying Scale Multiplier.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Transforms Along Spline", AdvancedDisplay = "SpacingMode,CoordinateSpace,bUseSplineScale,RotationOffset,ScaleMultiplier,StartDistance,EndDistance"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FTransform> GenerateTransformsAlongSpline(const USplineComponent* Spline, double Spacing,
		bool bIncludeEndpoint = true,
		EDirectiveUtilSplineSpacingMode SpacingMode = EDirectiveUtilSplineSpacingMode::Fixed,
		ESplineCoordinateSpace::Type CoordinateSpace = ESplineCoordinateSpace::World,
		bool bUseSplineScale = true,
		FRotator RotationOffset = FRotator(0.0, 0.0, 0.0), FVector ScaleMultiplier = FVector(1.0, 1.0, 1.0),
		double StartDistance = 0.0, double EndDistance = -1.0);

	/**
	 * Generates a fixed number of evenly spaced transforms along a spline.
	 * Rotation follows the spline tangent and roll. Scale can include the spline scale before applying Scale Multiplier.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Transforms Along Spline by Count", AdvancedDisplay = "bUseSplineScale,RotationOffset,ScaleMultiplier,StartDistance,EndDistance"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FTransform> GenerateTransformsAlongSplineByCount(const USplineComponent* Spline, int32 Count,
		bool bIncludeEndpoints = true,
		ESplineCoordinateSpace::Type CoordinateSpace = ESplineCoordinateSpace::World,
		bool bUseSplineScale = true,
		FRotator RotationOffset = FRotator(0.0, 0.0, 0.0), FVector ScaleMultiplier = FVector(1.0, 1.0, 1.0),
		double StartDistance = 0.0, double EndDistance = -1.0);

	/**
	 * Generates evenly spaced points around a circle on the rotated local XY plane.
	 * @returns The generated points without repeating the first point, or an empty array for invalid input.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Points On Circle"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GeneratePointsOnCircle(const FVector& Center, const FRotator& Rotation,
		double Radius, int32 Count, double StartAngleDegrees = 0.0);

	/** Generates transforms around a circle with fixed, radial, or path-relative orientation. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Transforms On Circle", AdvancedDisplay = "RotationOffset,Scale"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FTransform> GenerateTransformsOnCircle(const FVector& Center, const FRotator& Rotation,
		double Radius, int32 Count, double StartAngleDegrees = 0.0,
		EDirectiveUtilRadialOrientation Orientation = EDirectiveUtilRadialOrientation::FaceCenter,
		FRotator RotationOffset = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));

	/**
	 * Generates evenly spaced points along an arc on the rotated local XY plane.
	 * @param bIncludeEndpoint - Whether the final point lies at Start Angle plus Arc Angle.
	 * @returns The generated points, or an empty array for invalid input.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Points On Arc"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GeneratePointsOnArc(const FVector& Center, const FRotator& Rotation,
		double Radius, int32 Count, double StartAngleDegrees = 0.0, double ArcAngleDegrees = 90.0,
		bool bIncludeEndpoint = true);

	/** Generates transforms along an arc with fixed, radial, or path-relative orientation. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Transforms On Arc", AdvancedDisplay = "RotationOffset,Scale"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FTransform> GenerateTransformsOnArc(const FVector& Center, const FRotator& Rotation,
		double Radius, int32 Count, double StartAngleDegrees = 0.0, double ArcAngleDegrees = 90.0,
		bool bIncludeEndpoint = true,
		EDirectiveUtilRadialOrientation Orientation = EDirectiveUtilRadialOrientation::FaceCenter,
		FRotator RotationOffset = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));

	/**
	 * Generates a deterministic sunflower distribution across a disc on the rotated local XY plane.
	 * @returns Approximately even area coverage, or an empty array for invalid input.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Points On Disc"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GeneratePointsOnDisc(const FVector& Center, const FRotator& Rotation,
		double Radius, int32 Count, double AngleOffsetDegrees = 0.0);

	/**
	 * Generates a deterministic Fibonacci distribution across a sphere surface.
	 * @returns Approximately even surface coverage, or an empty array for invalid input.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Points On Sphere"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> GeneratePointsOnSphere(const FVector& Center, const FRotator& Rotation,
		double Radius, int32 Count, double AngleOffsetDegrees = 0.0);

	/**
	 * Offsets each location along a direction by Perlin noise sampled at that location.
	 * The offset varies smoothly between -Amplitude and Amplitude across the noise field.
	 * @param Locations - The locations to offset.
	 * @param NoiseScale - The world-space size of the noise features. Must be positive.
	 * @param Amplitude - The maximum offset distance along the direction.
	 * @param Direction - The offset direction. Its magnitude is ignored.
	 * @param NoiseOffset - World-space shift of the noise field, for varying the pattern between layers.
	 * @returns The offset locations, or an empty array for invalid input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Offset Locations By Noise", BlueprintThreadSafe, AdvancedDisplay = "Direction,NoiseOffset"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FVector> OffsetLocationsByNoise(const TArray<FVector>& Locations, double NoiseScale,
		double Amplitude, FVector Direction = FVector(0.0, 0.0, 1.0),
		FVector NoiseOffset = FVector(0.0, 0.0, 0.0));

	/**
	 * Offsets each transform location along a direction by Perlin noise sampled at that location.
	 * Rotation and scale are unchanged. Behaves like Offset Locations By Noise.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Offset Transforms By Noise", BlueprintThreadSafe, AdvancedDisplay = "Direction,NoiseOffset"), Category = "Directive Utilities|Math|Point Generation")
	static TArray<FTransform> OffsetTransformsByNoise(const TArray<FTransform>& Transforms, double NoiseScale,
		double Amplitude, FVector Direction = FVector(0.0, 0.0, 1.0),
		FVector NoiseOffset = FVector(0.0, 0.0, 0.0));

	/**
	 * Applies a Back/Elastic/Bounce easing curve to a normalized alpha.
	 * @note These are the Penner easing curves the engine's built-in "Ease" node (EEasingFunc) does not provide.
	 * For Sinusoidal/Exponential/Circular/power easings, use the engine's "Ease" node instead.
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased alpha. Endpoints are exact. Back and Elastic curves intentionally overshoot the [0, 1] range between the endpoints.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease Alpha", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static float EaseAlpha(float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases a float from A to B using a Back/Elastic/Bounce easing curve.
	 * @param A - The start value (returned at Alpha 0).
	 * @param B - The target value (returned at Alpha 1).
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased value between A and B.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Float)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static float EaseFloat(float A, float B, float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases a vector from A to B using a Back/Elastic/Bounce easing curve.
	 * @param A - The start vector (returned at Alpha 0).
	 * @param B - The target vector (returned at Alpha 1).
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased vector between A and B.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Vector)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static FVector EaseVector(const FVector& A, const FVector& B, float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases a rotator from A to B using a Back/Elastic/Bounce easing curve (shortest-path interpolation).
	 * @param A - The start rotator (returned at Alpha 0).
	 * @param B - The target rotator (returned at Alpha 1).
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased rotator between A and B.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Rotator)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static FRotator EaseRotator(const FRotator& A, const FRotator& B, float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases a color from A to B using a Back/Elastic/Bounce easing curve.
	 * @param A - The start color (returned at Alpha 0).
	 * @param B - The target color (returned at Alpha 1).
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased color between A and B.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Color)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static FLinearColor EaseColor(const FLinearColor& A, const FLinearColor& B, float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases a transform from A to B. Rotation takes the shortest path; location and scale interpolate linearly
	 * before the eased alpha is applied.
	 * @param A - The start transform (returned at Alpha 0).
	 * @param B - The target transform (returned at Alpha 1).
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased transform between A and B.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Transform)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static FTransform EaseTransform(const FTransform& A, const FTransform& B, float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases each location in From toward the same index in To. Use with two generated layouts to blend formations.
	 * @param Alpha - The shared input alpha. Clamped to the [0, 1] range.
	 * @param PerElementAlphas - When non-empty, one alpha per element replaces Alpha for staggered blends.
	 * @returns The eased locations, or an empty array for mismatched lengths or non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease Location Arrays", AutoCreateRefTerm = "PerElementAlphas", BlueprintThreadSafe, AdvancedDisplay = "PerElementAlphas"), Category = "Directive Utilities|Math|Easing")
	static TArray<FVector> EaseLocationArrays(const TArray<FVector>& From, const TArray<FVector>& To,
		float Alpha, EDirectiveUtilEaseType EaseType, const TArray<float>& PerElementAlphas);

	/**
	 * Eases each transform in From toward the same index in To. Use with two generated layouts to blend formations.
	 * Rotation takes the shortest path; location and scale interpolate linearly before the eased alpha is applied.
	 * @param Alpha - The shared input alpha. Clamped to the [0, 1] range.
	 * @param PerElementAlphas - When non-empty, one alpha per element replaces Alpha for staggered blends.
	 * @returns The eased transforms, or an empty array for mismatched lengths or non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease Transform Arrays", AutoCreateRefTerm = "PerElementAlphas", BlueprintThreadSafe, AdvancedDisplay = "PerElementAlphas"), Category = "Directive Utilities|Math|Easing")
	static TArray<FTransform> EaseTransformArrays(const TArray<FTransform>& From, const TArray<FTransform>& To,
		float Alpha, EDirectiveUtilEaseType EaseType, const TArray<float>& PerElementAlphas);

	/**
	 * Rounds a float to a given number of decimal places. Rounds half away from zero,
	 * matching "Round To Decimals (Text)".
	 * @note Due to floating-point representation the returned value may not display exactly;
	 * use "Round To Decimals (Text)" for clean display.
	 * @param Value - The value to round.
	 * @param Decimals - The number of decimal places to round to. Clamped to the [0, 10] range.
	 * @returns The rounded value.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Round To Decimals", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static float RoundToDecimals(float Value, int32 Decimals);

	/**
	 * Rounds a float to a given number of decimal places and returns it as display text.
	 * Rounds half away from zero, matching "Round To Decimals".
	 * @param Value - The value to round.
	 * @param Decimals - The maximum number of decimal places to display. Clamped to the [0, 10] range.
	 * @returns The rounded value as text, formatted with the current locale.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Round To Decimals (Text)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static FText RoundToDecimalsAsText(float Value, int32 Decimals);

	/**
	 * Formats a byte count as a human-readable size using binary units (1024): B, KB, MB, GB, TB, PB.
	 * Decimals are applied only from KB up ("532 B", "1.4 MB"). Negative input formats the absolute
	 * value with a leading minus sign. Output is English-only.
	 * @param Bytes - The byte count to format.
	 * @param Decimals - The number of decimal places to show from KB up. Clamped to the [0, 3] range.
	 * @returns The formatted size text.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Math|Formatting")
	static FText FormatBytes(int64 Bytes, int32 Decimals = 1);

	/**
	 * Formats a duration in seconds as d/h/m/s units from the largest nonzero unit down, with
	 * two-digit padding after the first ("1h 03m 05s", "2d 04h", "45s"). With bIncludeSeconds
	 * false the seconds unit is dropped and sub-minute durations return "0m". Negative input gets
	 * a leading minus sign when a nonzero unit remains; non-finite input returns "0s". Output is English-only.
	 * @param Seconds - The duration in seconds.
	 * @param bIncludeSeconds - Whether to include the seconds unit.
	 * @returns The formatted duration text.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Math|Formatting")
	static FText FormatDuration(float Seconds, bool bIncludeSeconds = true);

	/**
	 * Formats a timestamp relative to the current local time: "just now" (under a minute),
	 * "N minute(s)/hour(s)/day(s) ago", or "in N ..." for future timestamps. Uses local time,
	 * pairing with Get Save Slot Timestamp. Output is English-only.
	 * @note Not pure: reads the current clock each call.
	 * @param Timestamp - The local timestamp to describe.
	 * @returns The formatted relative time text.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Math|Formatting")
	static FText FormatRelativeTime(const FDateTime& Timestamp);

	/**
	 * Returns the sum of an integer array as a 64-bit integer, so large arrays cannot overflow int32.
	 * @param Values - The values to sum.
	 * @returns The sum of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static int64 GetIntArraySum(const TArray<int32>& Values);

	/**
	 * Returns the arithmetic mean of an integer array.
	 * @param Values - The values to average.
	 * @returns The average of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetIntArrayAverage(const TArray<int32>& Values);

	/**
	 * Returns the median of an integer array (computed on a sorted copy; the input is not modified).
	 * For an even count, returns the average of the two middle values.
	 * @param Values - The values to take the median of.
	 * @returns The median of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetIntArrayMedian(const TArray<int32>& Values);

	/**
	 * Returns the population standard deviation of an integer array (divides by N, not N-1).
	 * @param Values - The values to measure.
	 * @returns The population standard deviation, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetIntArrayStandardDeviation(const TArray<int32>& Values);

	/**
	 * Returns the sum of a float array. Accumulates in double internally for precision.
	 * @param Values - The values to sum.
	 * @returns The sum of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetFloatArraySum(const TArray<float>& Values);

	/**
	 * Returns the arithmetic mean of a float array. Accumulates in double internally for precision.
	 * @param Values - The values to average.
	 * @returns The average of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetFloatArrayAverage(const TArray<float>& Values);

	/**
	 * Returns the median of a float array (computed on a sorted copy; the input is not modified).
	 * For an even count, returns the average of the two middle values.
	 * @param Values - The values to take the median of.
	 * @returns The median of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetFloatArrayMedian(const TArray<float>& Values);

	/**
	 * Returns the population standard deviation of a float array (divides by N, not N-1).
	 * Accumulates in double internally for precision.
	 * @param Values - The values to measure.
	 * @returns The population standard deviation, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetFloatArrayStandardDeviation(const TArray<float>& Values);

	/**
	 * Calculates the circular mean of an angle array in degrees.
	 * @returns False for an empty array, non-finite input, or an undefined or numerically indeterminate circular mean.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Angle Array Average", BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static bool GetAngleArrayAverage(const TArray<float>& Angles, float& AverageAngle, float& ResultantStrength);

	/**
	 * Calculates the weighted average of a float array.
	 * @returns False when the arrays differ in size, contain invalid values, or have no positive weight.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Weighted Float Array Average", BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static bool GetWeightedFloatArrayAverage(const TArray<float>& Values, const TArray<float>& Weights, float& Average);

	/**
	 * Calculates the weighted average of a vector array.
	 * @returns False when the arrays differ in size, contain invalid values, or have no positive weight.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Weighted Vector Array Average", BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static bool GetWeightedVectorArrayAverage(const TArray<FVector>& Values, const TArray<float>& Weights, FVector& Average);

	/**
	 * Normalizes a float array to an output range.
	 * @returns False for an empty array or non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Normalize Float Array To Range", BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static bool NormalizeFloatArrayToRange(const TArray<float>& Values, float OutputMinimum, float OutputMaximum,
		TArray<float>& NormalizedValues);

	/**
	 * Normalizes positive weights so their sum is one. Negative and non-finite weights are treated as zero.
	 * @returns False for an empty array or when no positive weight remains.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Normalize Weights", BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static bool NormalizeWeights(const TArray<float>& Weights, TArray<float>& NormalizedWeights);

	/**
	 * Calculates a percentile using the Type 7 linear method without modifying the input array.
	 * @returns False for an empty array or non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Float Array Percentile", BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static bool GetFloatArrayPercentile(const TArray<float>& Values, float Percentile, float& Value);

	/**
	 * Calculates the root mean square of a float array.
	 * @returns False for an empty array or non-finite input.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Float Array Root Mean Square", BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static bool GetFloatArrayRootMeanSquare(const TArray<float>& Values, float& RootMeanSquare);

	/**
	 * Returns a random index into the Weights array, where each index's probability is proportional to its weight.
	 * Useful for loot tables and weighted spawning. Negative and non-finite weights are treated as zero.
	 * @param Weights - The per-index weights.
	 * @returns The selected index, or INDEX_NONE (-1) if the array is empty or all weights are zero.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Random Index From Weights"), Category = "Directive Utilities|Math|Random")
	static int32 GetRandomIndexFromWeights(const TArray<float>& Weights);

	/**
	 * Deterministic version of Get Random Index From Weights that draws from (and advances) the provided random stream.
	 * @param Stream - The random stream to draw from.
	 * @param Weights - The per-index weights. Negative and non-finite weights are treated as zero.
	 * @returns The selected index, or INDEX_NONE (-1) if the array is empty or all weights are zero.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Random Index From Weights (Stream)"), Category = "Directive Utilities|Math|Random")
	static int32 GetRandomIndexFromWeightsFromStream(UPARAM(ref) FRandomStream& Stream, const TArray<float>& Weights);

	/** Returns a uniformly distributed random point inside a circle. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Random Point In Circle"), Category = "Directive Utilities|Math|Random")
	static FVector2D RandomPointInCircle(float Radius);

	/** Returns a deterministic uniformly distributed random point inside a circle. Invalid or zero radii do not advance the stream. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Random Point In Circle (Stream)"), Category = "Directive Utilities|Math|Random")
	static FVector2D RandomPointInCircleFromStream(UPARAM(ref) FRandomStream& Stream, float Radius);

	/** Returns a uniformly distributed random point inside a 2D annulus. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Random Point In Annulus"), Category = "Directive Utilities|Math|Random")
	static FVector2D RandomPointInAnnulus(float InnerRadius, float OuterRadius);

	/** Returns a deterministic uniformly distributed random point inside a 2D annulus. Invalid or zero radii do not advance the stream. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Random Point In Annulus (Stream)"), Category = "Directive Utilities|Math|Random")
	static FVector2D RandomPointInAnnulusFromStream(UPARAM(ref) FRandomStream& Stream, float InnerRadius, float OuterRadius);

	/** Returns a uniformly distributed random point inside a sphere. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Random Point In Sphere"), Category = "Directive Utilities|Math|Random")
	static FVector RandomPointInSphere(float Radius);

	/** Returns a deterministic uniformly distributed random point inside a sphere. Invalid or zero radii do not advance the stream. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Random Point In Sphere (Stream)"), Category = "Directive Utilities|Math|Random")
	static FVector RandomPointInSphereFromStream(UPARAM(ref) FRandomStream& Stream, float Radius);
};
