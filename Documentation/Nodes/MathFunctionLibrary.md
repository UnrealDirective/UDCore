# Math Function Library

> Contains math functions for the Directive Utilities plugin.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilMathFunctionLibrary.h`

---

## Perlin Noise 2D
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static float PerlinNoise2D(FVector2D Position);
```

Returns a perlin noise value between -1 and 1 at the given position. Exposes the engine's built-in PerlinNoise2D function to Blueprints.

| Parameter | Type | Description |
|-----------|------|-------------|
| Position | `FVector2D` | The position to get the noise value for. |

**Returns:** The noise value at the given position.

## Perlin Noise 3D
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static float PerlinNoise3D(const FVector& Position);
```

Returns a perlin noise value between -1 and 1 at the given position. Exposes the engine's built-in PerlinNoise3D function to Blueprints.

| Parameter | Type | Description |
|-----------|------|-------------|
| Position | `const FVector&` | The position to get the noise value for. |

**Returns:** The noise value at the given position.

## Angle Between Vectors
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Vector`

```cpp
static float AngleBetweenVectors(const FVector& A, const FVector& B);
```

Returns the angle in degrees between two vectors. Zero or non-finite inputs return 0 instead of a silent 90.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FVector&` | The first vector. |
| B | `const FVector&` | The second vector. |

**Returns:** The angle between the two vectors in degrees, or 0 if either vector is zero or non-finite.

## Signed Angle Between Vectors
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Vector`

```cpp
static float SignedAngleBetweenVectors(const FVector& From, const FVector& To, const FVector& Axis);
```

Returns the signed angle from one direction to another around an axis. Both directions are projected onto the plane perpendicular to the axis before the angle is measured. Forward to Right around Up returns 90 degrees.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `const FVector&` | The starting direction. |
| To | `const FVector&` | The target direction. |
| Axis | `const FVector&` | The axis that defines the rotation plane and positive direction. |

**Returns:** The signed angle in the [-180, 180] range, or 0 if the axis or either projected direction is zero or non-finite.

## Delta Angle (Degrees)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static float DeltaAngle(float From, float To);
```

Returns the shortest signed difference from one angle to another. For example, the delta from 350 degrees to 10 degrees is 20 degrees. Exactly opposite angles always return +180.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `float` | The starting angle in degrees. |
| To | `float` | The target angle in degrees. |

**Returns:** The signed difference in the (-180, 180] range, or 0 if either input is non-finite.

## Lerp Angle (Degrees)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static float LerpAngle(float A, float B, float Alpha);
```

Interpolates between two angles along the shortest path. Alpha 0 returns A. Alpha is not clamped, so values outside the [0, 1] range extrapolate along that same shortest-path direction without wrapping.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `float` | The starting angle in degrees. |
| B | `float` | The target angle in degrees. |
| Alpha | `float` | The interpolation alpha. |

**Returns:** A plus the shortest signed delta to B, scaled by Alpha, or 0 if any input is non-finite.

## Ping Pong (Float)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static float PingPong(float Value, float Minimum = 0.0f, float Maximum = 1.0f);
```

Repeats a value between two bounds and reverses direction at each bound. Reversed bounds are accepted.

| Parameter | Type | Description |
|-----------|------|-------------|
| Value | `float` | The value to repeat. |
| Minimum | `float` | One range bound. |
| Maximum | `float` | The other range bound. |

**Returns:** The ping-ponged value, the shared bound for a zero-sized range, or 0 if any input is non-finite.

## Smooth Step
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static float SmoothStep(float Value, float Minimum = 0.0f, float Maximum = 1.0f);
```

Maps a value to a cubic S-curve between two bounds. Values outside the range are clamped, reversed bounds are accepted, and equal bounds act as a step.

| Parameter | Type | Description |
|-----------|------|-------------|
| Value | `float` | The value to evaluate. |
| Minimum | `float` | One range bound. |
| Maximum | `float` | The other range bound. |

**Returns:** A value in the [0, 1] range, or 0 if any input is non-finite.

## Smoother Step
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static float SmootherStep(float Value, float Minimum = 0.0f, float Maximum = 1.0f);
```

Maps a value to a quintic S-curve with zero first and second derivatives at both bounds. Values outside the range are clamped, reversed bounds are accepted, and equal bounds act as a step.

| Parameter | Type | Description |
|-----------|------|-------------|
| Value | `float` | The value to evaluate. |
| Minimum | `float` | One range bound. |
| Maximum | `float` | The other range bound. |

**Returns:** A value in the [0, 1] range, or 0 if any input is non-finite.

## Range Falloff
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static float RangeFalloff(float Distance, float InnerRadius, float OuterRadius, float FalloffExponent = 1.0f);
```

Returns full strength at or inside the inner radius and attenuates to zero at the outer radius. Reversed radii are accepted. Equal radii are a step: 1 at or inside the shared radius, 0 outside, including the default 0/0 case at the origin. A non-positive exponent keeps full strength until the outer boundary.

| Parameter | Type | Description |
|-----------|------|-------------|
| Distance | `float` | The distance from the source. Negative values are treated as zero. |
| InnerRadius | `float` | The full-strength radius. Negative values are treated as zero. |
| OuterRadius | `float` | The zero-strength radius. Negative values are treated as zero. |
| FalloffExponent | `float` | The falloff curve exponent. |

**Returns:** The normalized falloff, or 0 if any input is non-finite.

## Is Direction Within Cone
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Vector`

```cpp
static bool IsDirectionWithinCone(const FVector& Direction, const FVector& ConeDirection, float ConeHalfAngleDegrees);
```

Tests whether a direction lies inside or on a cone centered on another direction. The half-angle is clamped to the [0, 180] range. Boundary comparisons account for double-precision rounding.

| Parameter | Type | Description |
|-----------|------|-------------|
| Direction | `const FVector&` | The direction to test. |
| ConeDirection | `const FVector&` | The center direction of the cone. |
| ConeHalfAngleDegrees | `float` | The angle from the cone center to its edge. |

**Returns:** True when the direction lies inside or on the cone. Returns false if either direction is zero or non-finite, or if the angle is non-finite.

## Get Direction And Distance
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Vector`

```cpp
static bool GetDirectionAndDistance(const FVector& From, const FVector& To, FVector& Direction, double& Distance);
```

Calculates the normalized direction and distance between two points.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `const FVector&` | The starting point. |
| To | `const FVector&` | The destination point. |
| Direction | `FVector&` | Receives the normalized direction. |
| Distance | `double&` | Receives the distance between the points. |

**Returns:** True for distinct finite points. Equal or non-finite points return false and reset both outputs.

## Rotate Point Around Pivot 2D
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Vector`

```cpp
static FVector2D RotatePointAroundPivot2D(const FVector2D& Point, const FVector2D& Pivot, float AngleDegrees);
```

Rotates a 2D point around a pivot.

| Parameter | Type | Description |
|-----------|------|-------------|
| Point | `const FVector2D&` | The point to rotate. |
| Pivot | `const FVector2D&` | The center of rotation. |
| AngleDegrees | `float` | The rotation in degrees. |

**Returns:** The rotated point, or zero if any input is non-finite.

## Signed Distance To Plane
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Vector`

```cpp
static double SignedDistanceToPlane(const FVector& Point, const FVector& PlanePoint, const FVector& PlaneNormal);
```

Measures the perpendicular distance from a point to a plane. The sign follows the plane normal.

| Parameter | Type | Description |
|-----------|------|-------------|
| Point | `const FVector&` | The point to measure. |
| PlanePoint | `const FVector&` | Any point on the plane. |
| PlaneNormal | `const FVector&` | The plane normal. It does not need to be normalized. |

**Returns:** The signed distance, or 0 for a zero or non-finite normal or non-finite point.

## Is Point Within Cone
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Vector`

```cpp
static bool IsPointWithinCone(const FVector& Point, const FVector& ConeOrigin, const FVector& ConeDirection,
    float ConeHalfAngleDegrees, double MaximumDistance = 0.0);
```

Tests a world-space point against a cone and an optional distance limit.

| Parameter | Type | Description |
|-----------|------|-------------|
| Point | `const FVector&` | The point to test. |
| ConeOrigin | `const FVector&` | The cone origin. |
| ConeDirection | `const FVector&` | The center direction of the cone. |
| ConeHalfAngleDegrees | `float` | The angle from the cone center to its edge. Clamped to [0, 180]. |
| MaximumDistance | `double` | The distance limit. Zero or negative values disable the limit. |

**Returns:** True when the point lies inside or on the cone and within the enabled distance limit.

## Sample Location Array
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Vector`

```cpp
static FVector SampleLocationArray(const TArray<FVector>& Locations, float Alpha, bool bClosedLoop = false);
```

Samples a location along the polyline through an array, with Alpha 0 at the first point and 1 at the last. Progress is distance-weighted, so equal alpha steps cover equal world distance. Feed it the output of any generator to move something along that shape without a spline component.

| Parameter | Type | Description |
|-----------|------|-------------|
| Locations | `const TArray<FVector>&` | The polyline points, in order. |
| Alpha | `float` | Progress along the polyline. Clamped for open paths, wrapped for closed loops. |
| ClosedLoop | `bool` | Adds the segment from the last point back to the first and wraps Alpha. |

**Returns:** The sampled location. A single point returns itself; an empty array or non-finite input returns the zero vector.

## Locations To Transforms
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Transform`

```cpp
static TArray<FTransform> LocationsToTransforms(const TArray<FVector>& Locations,
    FRotator Rotation = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));
```

Creates one transform for each location using a shared rotation and scale. The location order is preserved. The output is allocated once, and the shared rotation is converted to a quaternion once.

| Parameter | Type | Description |
|-----------|------|-------------|
| Locations | `const TArray<FVector>&` | Transform locations in output order. |
| Rotation | `FRotator` | Rotation applied to every transform. |
| Scale | `FVector` | Scale applied to every transform. |

**Returns:** One transform per location. An empty location array or any non-finite input returns an empty array.

## Locations To Facing Transforms
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Transform`

```cpp
static TArray<FTransform> LocationsToFacingTransforms(const TArray<FVector>& Locations,
    FVector Target, FVector UpDirection = FVector(0.0, 0.0, 1.0),
    FRotator RotationOffset = FRotator(0.0, 0.0, 0.0),
    FVector Scale = FVector(1.0, 1.0, 1.0), bool bFaceAway = false);
```

Creates one transform per location with its local X axis facing toward or away from a target. The output order matches Locations. A location equal to Target uses Rotation Offset without an additional facing rotation.

| Parameter | Type | Description |
|-----------|------|-------------|
| Locations | `const TArray<FVector>&` | World-space transform locations in output order. |
| Target | `FVector` | World-space location to face toward or away from. |
| UpDirection | `FVector` | Preferred world-space local Z direction. Must be non-zero. |
| RotationOffset | `FRotator` | Local rotation applied after the facing rotation. |
| Scale | `FVector` | Scale applied to every transform. |
| FaceAway | `bool` | Faces away from Target when true. |

**Returns:** One transform per location. An empty location array or any non-finite input returns an empty array.

## Make Transforms From Arrays
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Transform`

```cpp
static bool MakeTransformsFromArrays(const TArray<FVector>& Locations,
    const TArray<FRotator>& Rotations, const TArray<FVector>& Scales,
    TArray<FTransform>& Transforms);
```

Creates transforms from aligned attribute arrays. Locations determine the output count. Rotations and Scales may be empty to use identity values, contain one value to broadcast, or contain one value per location.

| Parameter | Type | Description |
|-----------|------|-------------|
| Locations | `const TArray<FVector>&` | Transform locations and the required output count. |
| Rotations | `const TArray<FRotator>&` | Zero, one, or one rotation per location. |
| Scales | `const TArray<FVector>&` | Zero, one, or one scale per location. |
| Transforms | `TArray<FTransform>&` | The generated transforms. Cleared when validation fails. |

**Returns:** True when the arrays and values are valid. A rotation or scale count other than zero, one, or the location count returns false. Non-finite input also returns false.

## Sample Transform Array
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Transform`

```cpp
static FTransform SampleTransformArray(const TArray<FTransform>& Transforms, float Alpha, bool bClosedLoop = false);
```

Samples a transform along the path through an array, with Alpha 0 at the first transform and 1 at the last. Progress is distance-weighted by location. Rotation takes the shortest path and scale interpolates linearly between the two neighboring entries.

| Parameter | Type | Description |
|-----------|------|-------------|
| Transforms | `const TArray<FTransform>&` | The path transforms, in order. |
| Alpha | `float` | Progress along the path. Clamped for open paths, wrapped for closed loops. |
| ClosedLoop | `bool` | Adds the segment from the last transform back to the first and wraps Alpha. |

**Returns:** The sampled transform. A single transform returns itself; an empty array or non-finite input returns the identity.

Collection generators use execution pins so Blueprint controls when allocation happens. They return an empty array when the result would exceed 1,000,000 elements.

## Generate Grid Points 2D
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GenerateGridPoints2D(const FVector& Origin, const FRotator& Rotation,
    FIntPoint Dimensions, const FVector2D& Spacing, bool bCentered = true);
```

Generates a rectangular grid on the rotated local XY plane. Points are ordered by X, then Y.

| Parameter | Type | Description |
|-----------|------|-------------|
| Origin | `const FVector&` | The first point, or the grid center when Centered is true. |
| Rotation | `const FRotator&` | The grid plane rotation. |
| Dimensions | `FIntPoint` | The number of points along the local X and Y axes. |
| Spacing | `const FVector2D&` | Signed center-to-center spacing along the local X and Y axes. |
| Centered | `bool` | Centers the grid on Origin when true. |

**Returns:** The generated points. Non-positive dimensions, non-finite input, coordinate overflow, or a point count above the array limit returns an empty array.

## Generate Grid Points 3D
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GenerateGridPoints3D(const FVector& Origin, const FRotator& Rotation,
    FIntVector Dimensions, const FVector& Spacing, bool bCentered = true);
```

Generates a rectangular 3D grid. Points are ordered by X, then Y, then Z.

| Parameter | Type | Description |
|-----------|------|-------------|
| Origin | `const FVector&` | The first point, or the grid center when Centered is true. |
| Rotation | `const FRotator&` | The grid rotation. |
| Dimensions | `FIntVector` | The number of points along the local X, Y, and Z axes. |
| Spacing | `const FVector&` | Signed center-to-center spacing along the local X, Y, and Z axes. |
| Centered | `bool` | Centers the grid on Origin when true. |

**Returns:** The generated points. Non-positive dimensions, non-finite input, coordinate overflow, or a point count above the array limit returns an empty array.

## Generate Grid Transforms 2D
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FTransform> GenerateGridTransforms2D(const FVector& Origin, const FRotator& Rotation,
    FIntPoint Dimensions, const FVector2D& Spacing, bool bCentered = true,
    FRotator InstanceRotation = FRotator(0.0, 0.0, 0.0),
    FVector Scale = FVector(1.0, 1.0, 1.0));
```

Generates transforms on a rectangular grid on the rotated local XY plane. Locations match Generate Grid Points 2D. Instance Rotation and Scale are shared across every transform. Transforms are ordered by X, then Y.

| Parameter | Type | Description |
|-----------|------|-------------|
| Origin | `const FVector&` | The first location, or the grid center when Centered is true. |
| Rotation | `const FRotator&` | The grid plane rotation. |
| Dimensions | `FIntPoint` | The number of points along the local X and Y axes. |
| Spacing | `const FVector2D&` | Signed center-to-center spacing along the local X and Y axes. |
| Centered | `bool` | Centers the grid on Origin when true. |
| InstanceRotation | `FRotator` | Shared rotation applied to every transform. |
| Scale | `FVector` | Shared scale applied to every transform. |

**Returns:** The generated transforms. Non-positive dimensions, non-finite input, coordinate overflow, or a count above the array limit returns an empty array.

## Generate Grid Transforms 3D
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FTransform> GenerateGridTransforms3D(const FVector& Origin, const FRotator& Rotation,
    FIntVector Dimensions, const FVector& Spacing, bool bCentered = true,
    FRotator InstanceRotation = FRotator(0.0, 0.0, 0.0),
    FVector Scale = FVector(1.0, 1.0, 1.0));
```

Generates transforms on a rectangular 3D grid. Locations match Generate Grid Points 3D. Instance Rotation and Scale are shared across every transform. Transforms are ordered by X, then Y, then Z.

| Parameter | Type | Description |
|-----------|------|-------------|
| Origin | `const FVector&` | The first location, or the grid center when Centered is true. |
| Rotation | `const FRotator&` | The grid rotation. |
| Dimensions | `FIntVector` | The number of points along the local X, Y, and Z axes. |
| Spacing | `const FVector&` | Signed center-to-center spacing along the local X, Y, and Z axes. |
| Centered | `bool` | Centers the grid on Origin when true. |
| InstanceRotation | `FRotator` | Shared rotation applied to every transform. |
| Scale | `FVector` | Shared scale applied to every transform. |

**Returns:** The generated transforms. Non-positive dimensions, non-finite input, coordinate overflow, or a count above the array limit returns an empty array.

## Generate Rectangular Hex Grid
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FVector> GenerateRectangularHexGrid(const FVector& Origin, const FRotator& Rotation,
    FIntPoint Dimensions, double CellRadius,
    EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
    double Gap = 0.0, bool bCentered = true);
```

Generates a rectangular set of hex cell centers on the rotated local XY plane. Pointy-top grids use odd-row offset placement. Flat-top grids use odd-column offset placement. Points are ordered by row, then column.

| Parameter | Type | Description |
|-----------|------|-------------|
| Origin | `const FVector&` | The first cell center, or the grid bounds center when Centered is true. |
| Rotation | `const FRotator&` | The grid plane rotation. |
| Dimensions | `FIntPoint` | The number of columns and rows. |
| CellRadius | `double` | Distance from a cell center to a corner. Must be positive. |
| Orientation | `EDirectiveUtilHexOrientation` | Pointy Top or Flat Top. |
| Gap | `double` | Signed edge-to-edge gap between neighboring cells. Negative values overlap cells. |
| Centered | `bool` | Centers the grid bounds on Origin when true. |

**Returns:** The cell centers. Non-positive dimensions, invalid layout input, a non-positive center spacing, coordinate overflow, or a point count above the array limit returns an empty array.

## Generate Rectangular Hex Grid Transforms
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FTransform> GenerateRectangularHexGridTransforms(const FVector& Origin, const FRotator& Rotation,
    FIntPoint Dimensions, double CellRadius,
    EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
    double Gap = 0.0, bool bCentered = true,
    FRotator InstanceRotation = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));
```

Generates transforms at the cells of a rectangular hex grid, in the same cell order as Generate Rectangular Hex Grid. Every transform shares Instance Rotation and Scale.

| Parameter | Type | Description |
|-----------|------|-------------|
| InstanceRotation | `FRotator` | Shared rotation applied to every transform. |
| Scale | `FVector` | Shared scale applied to every transform. |

Remaining parameters match Generate Rectangular Hex Grid.

**Returns:** The cell transforms. Invalid input returns an empty array.

## Get Rectangular Hex Grid Coordinates
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FIntPoint> GetRectangularHexGridCoordinates(FIntPoint Dimensions,
    EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop);
```

Returns the axial coordinate of every cell of a rectangular hex grid, in the same cell order as Generate Rectangular Hex Grid. Pairing the two arrays by index binds grid state to cell locations.

| Parameter | Type | Description |
|-----------|------|-------------|
| Dimensions | `FIntPoint` | The number of columns and rows. |
| Orientation | `EDirectiveUtilHexOrientation` | Pointy Top or Flat Top. |

**Returns:** The axial coordinates. Non-positive dimensions, an invalid orientation, or a count above the array limit returns an empty array.

## Generate Hexagonal Hex Grid
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FVector> GenerateHexagonalHexGrid(const FVector& Origin, const FRotator& Rotation,
    int32 GridRadius, double CellRadius,
    EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
    double Gap = 0.0);
```

Generates a hexagon-shaped set of cell centers. Grid Radius is the number of rings around the center, so zero returns one point, one returns seven, and two returns nineteen. Points are ordered by axial R, then Q.

| Parameter | Type | Description |
|-----------|------|-------------|
| Origin | `const FVector&` | The center cell location. |
| Rotation | `const FRotator&` | The grid plane rotation. |
| GridRadius | `int32` | Number of cell rings around the center. |
| CellRadius | `double` | Distance from a cell center to a corner. Must be positive. |
| Orientation | `EDirectiveUtilHexOrientation` | Pointy Top or Flat Top. |
| Gap | `double` | Signed edge-to-edge gap between neighboring cells. Negative values overlap cells. |

**Returns:** `1 + 3 * GridRadius * (GridRadius + 1)` cell centers. A negative grid radius, invalid layout input, a non-positive center spacing, coordinate overflow, or a point count above the array limit returns an empty array.

## Generate Hexagonal Hex Grid Transforms
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FTransform> GenerateHexagonalHexGridTransforms(const FVector& Origin, const FRotator& Rotation,
    int32 GridRadius, double CellRadius,
    EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
    double Gap = 0.0,
    FRotator InstanceRotation = FRotator(0.0, 0.0, 0.0), FVector Scale = FVector(1.0, 1.0, 1.0));
```

Generates transforms at the cells of a hexagon-shaped grid, in the same cell order as Generate Hexagonal Hex Grid. Every transform shares Instance Rotation and Scale.

| Parameter | Type | Description |
|-----------|------|-------------|
| InstanceRotation | `FRotator` | Shared rotation applied to every transform. |
| Scale | `FVector` | Shared scale applied to every transform. |

Remaining parameters match Generate Hexagonal Hex Grid.

**Returns:** The cell transforms. Invalid input returns an empty array.

## Hex Coordinate To Location
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static FVector HexCoordinateToLocation(FIntPoint Coordinate, const FVector& Origin,
    const FRotator& Rotation, double CellRadius,
    EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
    double Gap = 0.0);
```

Converts an axial coordinate to its cell center. `Coordinate.X` is Q and `Coordinate.Y` is R. Origin is the center of coordinate `(0, 0)`.

| Parameter | Type | Description |
|-----------|------|-------------|
| Coordinate | `FIntPoint` | Axial Q and R coordinate. |
| Origin | `const FVector&` | Location of axial coordinate `(0, 0)`. |
| Rotation | `const FRotator&` | The grid plane rotation. |
| CellRadius | `double` | Distance from a cell center to a corner. Must be positive. |
| Orientation | `EDirectiveUtilHexOrientation` | Pointy Top or Flat Top. |
| Gap | `double` | Signed edge-to-edge gap between neighboring cells. |

**Returns:** The cell center. Invalid layout input, a non-positive center spacing, or coordinate overflow returns the zero vector.

## Location To Hex Coordinate
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static FIntPoint LocationToHexCoordinate(const FVector& Location, const FVector& Origin,
    const FRotator& Rotation, double CellRadius,
    EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
    double Gap = 0.0);
```

Projects a location onto the rotated grid plane and returns the nearest axial coordinate. `Coordinate.X` is Q and `Coordinate.Y` is R. Cube-coordinate rounding keeps the result on a valid hex cell.

| Parameter | Type | Description |
|-----------|------|-------------|
| Location | `const FVector&` | World-space location to convert. |
| Origin | `const FVector&` | Location of axial coordinate `(0, 0)`. |
| Rotation | `const FRotator&` | The grid plane rotation. |
| CellRadius | `double` | Distance from a cell center to a corner. Must be positive. |
| Orientation | `EDirectiveUtilHexOrientation` | Pointy Top or Flat Top. |
| Gap | `double` | Signed edge-to-edge gap between neighboring cells. |

**Returns:** The nearest axial coordinate. Invalid layout input, a non-positive center spacing, or a coordinate outside the `FIntPoint` range returns `(0, 0)`.

## Get Hex Neighbors
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FIntPoint> GetHexNeighbors(FIntPoint Coordinate);
```

Returns the six adjacent axial coordinates in this order: `(1, 0)`, `(1, -1)`, `(0, -1)`, `(-1, 0)`, `(-1, 1)`, `(0, 1)`, each added to Coordinate.

| Parameter | Type | Description |
|-----------|------|-------------|
| Coordinate | `FIntPoint` | Axial Q and R coordinate. |

**Returns:** Six adjacent coordinates. If any neighbor would exceed the `FIntPoint` range, the function returns an empty array.

## Get Hex Distance
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static int64 GetHexDistance(FIntPoint A, FIntPoint B);
```

Returns the minimum number of neighbor steps between two axial coordinates. The calculation uses 64-bit intermediates across the full `FIntPoint` range.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `FIntPoint` | First axial Q and R coordinate. |
| B | `FIntPoint` | Second axial Q and R coordinate. |

**Returns:** The hex-grid distance.

## Get Hexes In Range
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FIntPoint> GetHexesInRange(FIntPoint Center, int32 Range);
```

Returns every axial coordinate within a number of neighbor steps of a center cell, ordered by axial R, then Q. With a zero center the order matches the cells of Generate Hexagonal Hex Grid, so the two arrays pair by index. Useful for movement ranges and areas of effect.

| Parameter | Type | Description |
|-----------|------|-------------|
| Center | `FIntPoint` | The center axial coordinate. |
| Range | `int32` | The maximum number of steps from the center. |

**Returns:** `1 + 3 * Range * (Range + 1)` coordinates. A negative range, coordinate overflow, or a count above the array limit returns an empty array.

## Get Hex Ring
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FIntPoint> GetHexRing(FIntPoint Center, int32 Radius);
```

Returns the axial coordinates exactly Radius steps from a center cell. Consecutive entries are adjacent and trace the ring once. A radius of zero returns the center cell.

| Parameter | Type | Description |
|-----------|------|-------------|
| Center | `FIntPoint` | The center axial coordinate. |
| Radius | `int32` | The exact number of steps from the center. |

**Returns:** `6 * Radius` coordinates, or one for a zero radius. A negative radius or coordinate overflow returns an empty array.

## Get Hex Line
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FIntPoint> GetHexLine(FIntPoint Start, FIntPoint End);
```

Returns the axial coordinates along the straight line between two cells, including both endpoints. Consecutive entries are adjacent. Useful for line of sight and path previews.

| Parameter | Type | Description |
|-----------|------|-------------|
| Start | `FIntPoint` | The first axial coordinate. |
| End | `FIntPoint` | The last axial coordinate. |

**Returns:** One coordinate per hex-grid step plus the start. A line length above the array limit returns an empty array.

## Get Hex Cell Corners
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Hex Grid`

```cpp
static TArray<FVector> GetHexCellCorners(FIntPoint Coordinate, const FVector& Origin, const FRotator& Rotation,
    double CellRadius, EDirectiveUtilHexOrientation Orientation = EDirectiveUtilHexOrientation::PointyTop,
    double Gap = 0.0);
```

Returns the six corner locations of a hex cell on the rotated local XY plane, ordered counter-clockwise. Corners lie at Cell Radius from the cell center; Gap only moves the center. Useful for drawing cell outlines and building cell meshes.

| Parameter | Type | Description |
|-----------|------|-------------|
| Coordinate | `FIntPoint` | The axial coordinate of the cell. |
| Origin | `const FVector&` | The location of cell (0, 0). |
| Rotation | `const FRotator&` | The grid plane rotation. |
| CellRadius | `double` | Distance from the cell center to a corner. Must be positive. |
| Orientation | `EDirectiveUtilHexOrientation` | Pointy Top or Flat Top. |
| Gap | `double` | Signed edge-to-edge gap between neighboring cells. |

**Returns:** The six corner locations. Invalid layout input or coordinate overflow returns an empty array.

## Generate Points Along Direction
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GeneratePointsAlongDirection(const FVector& Origin, const FVector& Direction,
    int32 Count, double Spacing, bool bCentered = false);
```

Generates a line of points with fixed center-to-center spacing. Direction is normalized once, so its input magnitude does not change the spacing.

| Parameter | Type | Description |
|-----------|------|-------------|
| Origin | `const FVector&` | The first point, or the formation center when Centered is true. |
| Direction | `const FVector&` | The direction of travel. |
| Count | `int32` | The number of points. |
| Spacing | `double` | Signed distance between adjacent points. |
| Centered | `bool` | Centers the formation on Origin when true. |

**Returns:** The generated points. A non-positive count, zero direction, non-finite input, or coordinate overflow returns an empty array.

## Generate Points Between Locations
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GeneratePointsBetweenLocations(const FVector& Start, const FVector& End,
    int32 Count, bool bIncludeEndpoints = true);
```

Generates evenly spaced points across a line segment. When endpoints are excluded, every point lies inside the segment. A count of one returns the midpoint.

| Parameter | Type | Description |
|-----------|------|-------------|
| Start | `const FVector&` | The start of the segment. |
| End | `const FVector&` | The end of the segment. |
| Count | `int32` | The number of points. |
| IncludeEndpoints | `bool` | Includes Start and End when at least two points are generated. |

**Returns:** The generated points. A non-positive count, non-finite input, or coordinate overflow returns an empty array.

## Generate Points Along Spline
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GeneratePointsAlongSpline(const USplineComponent* Spline, double Spacing,
    bool bIncludeEndpoint = true,
    EDirectiveUtilSplineSpacingMode SpacingMode = EDirectiveUtilSplineSpacingMode::Fixed,
    ESplineCoordinateSpace::Type CoordinateSpace = ESplineCoordinateSpace::World,
    double StartDistance = 0.0, double EndDistance = -1.0);
```

Samples locations at fixed distances along a spline, starting at Start Distance. Fixed spacing samples every Spacing units, so appending the endpoint of an open range may make the final interval shorter. Even spacing shrinks the spacing so the samples divide the range evenly. A closed spline sampled over its full length does not repeat its first point.

| Parameter | Type | Description |
|-----------|------|-------------|
| Spline | `const USplineComponent*` | Spline component to sample. |
| Spacing | `double` | Distance between regular samples. Must be positive. |
| IncludeEndpoint | `bool` | Appends the exact end of an open sampling range when true. |
| SpacingMode | `EDirectiveUtilSplineSpacingMode` | Keeps Spacing exactly, or shrinks it to divide the range evenly. |
| CoordinateSpace | `ESplineCoordinateSpace::Type` | Space of the returned points. |
| StartDistance | `double` | Distance where sampling starts. Clamped to the spline length. |
| EndDistance | `double` | Distance where sampling ends. Negative means the end of the spline. |

**Returns:** The sampled points. A null or empty spline, non-positive or non-finite spacing, invalid spline length, an end distance before the start distance, coordinate overflow, or a point count above the array limit returns an empty array. A zero-length sampling range returns one location.

## Generate Points Along Spline by Count
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GeneratePointsAlongSplineByCount(const USplineComponent* Spline, int32 Count,
    bool bIncludeEndpoints = true,
    ESplineCoordinateSpace::Type CoordinateSpace = ESplineCoordinateSpace::World,
    double StartDistance = 0.0, double EndDistance = -1.0);
```

Samples a fixed number of evenly spaced locations along a spline. A closed spline sampled over its full length spreads the points around the loop without repeating the first one. A count of one on an open range returns its midpoint.

| Parameter | Type | Description |
|-----------|------|-------------|
| Spline | `const USplineComponent*` | Spline component to sample. |
| Count | `int32` | The number of points. |
| IncludeEndpoints | `bool` | Includes both ends of an open sampling range when at least two points are generated. |
| CoordinateSpace | `ESplineCoordinateSpace::Type` | Space of the returned points. |
| StartDistance | `double` | Distance where sampling starts. Clamped to the spline length. |
| EndDistance | `double` | Distance where sampling ends. Negative means the end of the spline. |

**Returns:** The sampled points. A non-positive count, null or empty spline, invalid spline length, an end distance before the start distance, or coordinate overflow returns an empty array.

## Generate Transforms Along Spline
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FTransform> GenerateTransformsAlongSpline(const USplineComponent* Spline,
    double Spacing, bool bIncludeEndpoint = true,
    EDirectiveUtilSplineSpacingMode SpacingMode = EDirectiveUtilSplineSpacingMode::Fixed,
    ESplineCoordinateSpace::Type CoordinateSpace = ESplineCoordinateSpace::World,
    bool bUseSplineScale = true, FRotator RotationOffset = FRotator(0.0, 0.0, 0.0),
    FVector ScaleMultiplier = FVector(1.0, 1.0, 1.0),
    double StartDistance = 0.0, double EndDistance = -1.0);
```

Samples transforms at fixed distances along a spline. Rotation follows the spline tangent and roll. Sampling behaves like Generate Points Along Spline, including the spacing mode and sampling range.

| Parameter | Type | Description |
|-----------|------|-------------|
| Spline | `const USplineComponent*` | Spline component to sample. |
| Spacing | `double` | Distance between regular samples. Must be positive. |
| IncludeEndpoint | `bool` | Appends the exact end of an open sampling range when true. |
| SpacingMode | `EDirectiveUtilSplineSpacingMode` | Keeps Spacing exactly, or shrinks it to divide the range evenly. |
| CoordinateSpace | `ESplineCoordinateSpace::Type` | Space of the returned transforms. |
| UseSplineScale | `bool` | Includes the spline's interpolated scale when true. |
| RotationOffset | `FRotator` | Local rotation applied after the spline rotation. |
| ScaleMultiplier | `FVector` | Multiplies the selected spline scale. |
| StartDistance | `double` | Distance where sampling starts. Clamped to the spline length. |
| EndDistance | `double` | Distance where sampling ends. Negative means the end of the spline. |

**Returns:** The sampled transforms. Invalid input or a point count above the array limit returns an empty array. A zero-length sampling range returns one transform.

## Generate Transforms Along Spline by Count
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FTransform> GenerateTransformsAlongSplineByCount(const USplineComponent* Spline,
    int32 Count, bool bIncludeEndpoints = true,
    ESplineCoordinateSpace::Type CoordinateSpace = ESplineCoordinateSpace::World,
    bool bUseSplineScale = true, FRotator RotationOffset = FRotator(0.0, 0.0, 0.0),
    FVector ScaleMultiplier = FVector(1.0, 1.0, 1.0),
    double StartDistance = 0.0, double EndDistance = -1.0);
```

Samples a fixed number of evenly spaced transforms along a spline. Rotation follows the spline tangent and roll. Sampling behaves like Generate Points Along Spline by Count.

| Parameter | Type | Description |
|-----------|------|-------------|
| Spline | `const USplineComponent*` | Spline component to sample. |
| Count | `int32` | The number of transforms. |
| IncludeEndpoints | `bool` | Includes both ends of an open sampling range when at least two transforms are generated. |
| CoordinateSpace | `ESplineCoordinateSpace::Type` | Space of the returned transforms. |
| UseSplineScale | `bool` | Includes the spline's interpolated scale when true. |
| RotationOffset | `FRotator` | Local rotation applied after the spline rotation. |
| ScaleMultiplier | `FVector` | Multiplies the selected spline scale. |
| StartDistance | `double` | Distance where sampling starts. Clamped to the spline length. |
| EndDistance | `double` | Distance where sampling ends. Negative means the end of the spline. |

**Returns:** The sampled transforms. Invalid input returns an empty array.

## Generate Points On Circle
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GeneratePointsOnCircle(const FVector& Center, const FRotator& Rotation,
    double Radius, int32 Count, double StartAngleDegrees = 0.0);
```

Generates evenly spaced points around a circle on the rotated local XY plane. The first point is not repeated at the end. Negative radii use their absolute value.

| Parameter | Type | Description |
|-----------|------|-------------|
| Center | `const FVector&` | The circle center. |
| Rotation | `const FRotator&` | The circle plane rotation. |
| Radius | `double` | The circle radius. |
| Count | `int32` | The number of points. |
| StartAngleDegrees | `double` | The first point's angle around local Z. |

**Returns:** The generated points. A non-positive count, non-finite input, or coordinate overflow returns an empty array.

## Generate Transforms On Circle
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FTransform> GenerateTransformsOnCircle(const FVector& Center,
    const FRotator& Rotation, double Radius, int32 Count,
    double StartAngleDegrees = 0.0,
    EDirectiveUtilRadialOrientation Orientation = EDirectiveUtilRadialOrientation::FaceCenter,
    FRotator RotationOffset = FRotator(0.0, 0.0, 0.0),
    FVector Scale = FVector(1.0, 1.0, 1.0));
```

Generates transforms around a circle on the rotated local XY plane. Fixed uses Rotation for every transform. Face Center and Face Away From Center align local X radially. Follow Path and Face Against Path align local X with or against the counterclockwise point order.

| Parameter | Type | Description |
|-----------|------|-------------|
| Center | `const FVector&` | World-space center of the circle. |
| Rotation | `const FRotator&` | Circle plane rotation and fixed orientation. |
| Radius | `double` | Distance from Center. Negative values use their absolute value. |
| Count | `int32` | Number of transforms. |
| StartAngleDegrees | `double` | Angle of the first transform on the local XY plane. |
| Orientation | `EDirectiveUtilRadialOrientation` | Fixed, radial, or path-relative orientation. |
| RotationOffset | `FRotator` | Local rotation applied after the generated rotation. |
| Scale | `FVector` | Scale applied to every transform. |

**Returns:** The generated transforms without repeating the first location. Invalid input returns an empty array.

## Generate Points On Arc
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GeneratePointsOnArc(const FVector& Center, const FRotator& Rotation,
    double Radius, int32 Count, double StartAngleDegrees = 0.0, double ArcAngleDegrees = 90.0,
    bool bIncludeEndpoint = true);
```

Generates evenly spaced points along an arc on the rotated local XY plane. Positive angles travel from local X toward local Y. Negative radii use their absolute value. A count of one returns the start-angle point.

| Parameter | Type | Description |
|-----------|------|-------------|
| Center | `const FVector&` | The arc center. |
| Rotation | `const FRotator&` | The arc plane rotation. |
| Radius | `double` | The arc radius. |
| Count | `int32` | The number of points. |
| StartAngleDegrees | `double` | The first point's angle around local Z. |
| ArcAngleDegrees | `double` | The signed angular span. |
| IncludeEndpoint | `bool` | Places the final point at the end of the angular span when true. |

**Returns:** The generated points. A non-positive count, non-finite input, or coordinate overflow returns an empty array.

## Generate Transforms On Arc
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FTransform> GenerateTransformsOnArc(const FVector& Center,
    const FRotator& Rotation, double Radius, int32 Count,
    double StartAngleDegrees = 0.0, double ArcAngleDegrees = 90.0,
    bool bIncludeEndpoint = true,
    EDirectiveUtilRadialOrientation Orientation = EDirectiveUtilRadialOrientation::FaceCenter,
    FRotator RotationOffset = FRotator(0.0, 0.0, 0.0),
    FVector Scale = FVector(1.0, 1.0, 1.0));
```

Generates transforms along an arc on the rotated local XY plane. Follow Path uses the sign of Arc Angle Degrees, so a negative arc reverses the generated forward direction. Face Against Path points in the opposite direction.

| Parameter | Type | Description |
|-----------|------|-------------|
| Center | `const FVector&` | World-space center of the arc. |
| Rotation | `const FRotator&` | Arc plane rotation and fixed orientation. |
| Radius | `double` | Distance from Center. Negative values use their absolute value. |
| Count | `int32` | Number of transforms. |
| StartAngleDegrees | `double` | Angle of the first transform on the local XY plane. |
| ArcAngleDegrees | `double` | Signed angular span and path direction. |
| IncludeEndpoint | `bool` | Places the final transform at Start Angle plus Arc Angle when true. |
| Orientation | `EDirectiveUtilRadialOrientation` | Fixed, radial, or path-relative orientation. |
| RotationOffset | `FRotator` | Local rotation applied after the generated rotation. |
| Scale | `FVector` | Scale applied to every transform. |

**Returns:** The generated transforms. Invalid input returns an empty array.

## Generate Points On Disc
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GeneratePointsOnDisc(const FVector& Center, const FRotator& Rotation,
    double Radius, int32 Count, double AngleOffsetDegrees = 0.0);
```

Generates a deterministic sunflower distribution across a disc on the rotated local XY plane. Negative radii use their absolute value. A count of one returns the center.

| Parameter | Type | Description |
|-----------|------|-------------|
| Center | `const FVector&` | The disc center. |
| Rotation | `const FRotator&` | The disc plane rotation. |
| Radius | `double` | The disc radius. |
| Count | `int32` | The number of points. |
| AngleOffsetDegrees | `double` | Rotates the distribution around local Z. |

**Returns:** Deterministic points with approximately even area coverage. A non-positive count, non-finite input, or coordinate overflow returns an empty array.

## Generate Points On Sphere
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> GeneratePointsOnSphere(const FVector& Center, const FRotator& Rotation,
    double Radius, int32 Count, double AngleOffsetDegrees = 0.0);
```

Generates a deterministic Fibonacci distribution across a sphere surface. Negative radii use their absolute value. A count of one returns the point on the rotated local positive Z axis.

| Parameter | Type | Description |
|-----------|------|-------------|
| Center | `const FVector&` | The sphere center. |
| Rotation | `const FRotator&` | Rotates the distribution. |
| Radius | `double` | The sphere radius. |
| Count | `int32` | The number of points. |
| AngleOffsetDegrees | `double` | Rotates the distribution around local Z before applying Rotation. |

**Returns:** Deterministic points with approximately even surface coverage. A non-positive count, non-finite input, or coordinate overflow returns an empty array.

## Offset Locations By Noise
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FVector> OffsetLocationsByNoise(const TArray<FVector>& Locations, double NoiseScale,
    double Amplitude, FVector Direction = FVector(0.0, 0.0, 1.0),
    FVector NoiseOffset = FVector(0.0, 0.0, 0.0));
```

Offsets each location along a direction by Perlin noise sampled at that location, varying smoothly between minus and plus Amplitude across the noise field. The result is deterministic for the same inputs. Feed it the output of any generator to add height variation or organic irregularity.

| Parameter | Type | Description |
|-----------|------|-------------|
| Locations | `const TArray<FVector>&` | The locations to offset. |
| NoiseScale | `double` | World-space size of the noise features. Must be positive. |
| Amplitude | `double` | Maximum offset distance along the direction. |
| Direction | `FVector` | The offset direction. Its magnitude is ignored. |
| NoiseOffset | `FVector` | World-space shift of the noise field, for varying the pattern between layers. |

**Returns:** The offset locations. A non-positive or non-finite noise scale, non-finite amplitude, zero direction, or non-finite input returns an empty array.

## Offset Transforms By Noise
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Point Generation`

```cpp
static TArray<FTransform> OffsetTransformsByNoise(const TArray<FTransform>& Transforms, double NoiseScale,
    double Amplitude, FVector Direction = FVector(0.0, 0.0, 1.0),
    FVector NoiseOffset = FVector(0.0, 0.0, 0.0));
```

Offsets each transform location along a direction by Perlin noise sampled at that location. Rotation and scale are unchanged. Behaves like Offset Locations By Noise.

| Parameter | Type | Description |
|-----------|------|-------------|
| Transforms | `const TArray<FTransform>&` | The transforms to offset. |

Remaining parameters match Offset Locations By Noise.

**Returns:** The offset transforms. Invalid input returns an empty array.

## Ease Alpha
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static float EaseAlpha(float Alpha, EDirectiveUtilEaseType EaseType);
```

Applies a Back/Elastic/Bounce easing curve to a normalized alpha. These are the Penner easing curves the engine's built-in "Ease" node (EEasingFunc) does not provide; for Sinusoidal/Exponential/Circular/power easings, use the engine's "Ease" node instead.

| Parameter | Type | Description |
|-----------|------|-------------|
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased alpha. Endpoints are exact (0 at Alpha 0, 1 at Alpha 1). Back and Elastic curves intentionally overshoot the [0, 1] range between the endpoints.

## Ease (Float)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static float EaseFloat(float A, float B, float Alpha, EDirectiveUtilEaseType EaseType);
```

Eases a float from A to B using a Back/Elastic/Bounce easing curve.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `float` | The start value (returned at Alpha 0). |
| B | `float` | The target value (returned at Alpha 1). |
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased value between A and B.

## Ease (Vector)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static FVector EaseVector(const FVector& A, const FVector& B, float Alpha, EDirectiveUtilEaseType EaseType);
```

Eases a vector from A to B using a Back/Elastic/Bounce easing curve.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FVector&` | The start vector (returned at Alpha 0). |
| B | `const FVector&` | The target vector (returned at Alpha 1). |
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased vector between A and B.

## Ease (Rotator)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static FRotator EaseRotator(const FRotator& A, const FRotator& B, float Alpha, EDirectiveUtilEaseType EaseType);
```

Eases a rotator from A to B using a Back/Elastic/Bounce easing curve (shortest-path interpolation).

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FRotator&` | The start rotator (returned at Alpha 0). |
| B | `const FRotator&` | The target rotator (returned at Alpha 1). |
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased rotator between A and B.

## Ease (Color)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static FLinearColor EaseColor(const FLinearColor& A, const FLinearColor& B, float Alpha, EDirectiveUtilEaseType EaseType);
```

Eases a color from A to B using a Back/Elastic/Bounce easing curve.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FLinearColor&` | The start color (returned at Alpha 0). |
| B | `const FLinearColor&` | The target color (returned at Alpha 1). |
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased color between A and B.

## Ease (Transform)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static FTransform EaseTransform(const FTransform& A, const FTransform& B, float Alpha, EDirectiveUtilEaseType EaseType);
```

Eases a transform from A to B. Rotation takes the shortest path; location and scale interpolate linearly before the eased alpha is applied.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FTransform&` | The start transform (returned at Alpha 0). |
| B | `const FTransform&` | The target transform (returned at Alpha 1). |
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased transform between A and B.

## Ease Location Arrays
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static TArray<FVector> EaseLocationArrays(const TArray<FVector>& From, const TArray<FVector>& To,
    float Alpha, EDirectiveUtilEaseType EaseType, const TArray<float>& PerElementAlphas);
```

Eases each location in From toward the same index in To. Generate two layouts with the same count and drive Alpha over time to blend one formation into another; the Linear ease type gives a plain blend.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `const TArray<FVector>&` | The start locations (returned at Alpha 0). |
| To | `const TArray<FVector>&` | The target locations (returned at Alpha 1). Must match the From count. |
| Alpha | `float` | The shared input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |
| PerElementAlphas | `const TArray<float>&` | When non-empty, one alpha per element replaces Alpha for staggered blends. |

**Returns:** The eased locations. Mismatched lengths, a per-element alpha count that differs from the location count, or non-finite input returns an empty array.

## Ease Transform Arrays
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static TArray<FTransform> EaseTransformArrays(const TArray<FTransform>& From, const TArray<FTransform>& To,
    float Alpha, EDirectiveUtilEaseType EaseType, const TArray<float>& PerElementAlphas);
```

Eases each transform in From toward the same index in To. Rotation takes the shortest path; location and scale interpolate linearly before the eased alpha is applied. Behaves like Ease Location Arrays.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `const TArray<FTransform>&` | The start transforms (returned at Alpha 0). |
| To | `const TArray<FTransform>&` | The target transforms (returned at Alpha 1). Must match the From count. |

Remaining parameters match Ease Location Arrays.

**Returns:** The eased transforms. Invalid input returns an empty array.

## Round To Decimals
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static float RoundToDecimals(float Value, int32 Decimals);
```

Rounds a float to a given number of decimal places, rounding halves away from zero (matching "Round To Decimals (Text)"). Due to floating-point representation the returned value may not display exactly; use "Round To Decimals (Text)" for clean display.

| Parameter | Type | Description |
|-----------|------|-------------|
| Value | `float` | The value to round. |
| Decimals | `int32` | The number of decimal places to round to. Clamped to the [0, 10] range. |

**Returns:** The rounded value.

## Round To Decimals (Text)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static FText RoundToDecimalsAsText(float Value, int32 Decimals);
```

Rounds a float to a given number of decimal places and returns it as display text, rounding halves away from zero (matching "Round To Decimals").

| Parameter | Type | Description |
|-----------|------|-------------|
| Value | `float` | The value to round. |
| Decimals | `int32` | The maximum number of decimal places to display. Clamped to the [0, 10] range. |

**Returns:** The rounded value as text, formatted with the current locale.

## Format Bytes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Formatting`

```cpp
static FText FormatBytes(int64 Bytes, int32 Decimals = 1);
```

Formats a byte count as a human-readable size using binary units (1024): B, KB, MB, GB, TB, PB. Decimals are applied only from KB up (`532 B`, `1.4 MB`). Negative input formats the absolute value with a leading minus sign. Output is English-only.

| Parameter | Type | Description |
|-----------|------|-------------|
| Bytes | `int64` | The byte count to format. |
| Decimals | `int32` | The number of decimal places to show from KB up. Clamped to the [0, 3] range. |

**Returns:** The formatted size text.

## Format Duration
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Formatting`

```cpp
static FText FormatDuration(float Seconds, bool bIncludeSeconds = true);
```

Formats a duration in seconds as d/h/m/s units from the largest nonzero unit down, with two-digit padding after the first (`1h 03m 05s`, `2d 04h`, `45s`). With `bIncludeSeconds` false the seconds unit is dropped and sub-minute durations return `0m`. Negative input gets a leading minus sign when a nonzero unit remains; non-finite input returns `0s`. Output is English-only.

| Parameter | Type | Description |
|-----------|------|-------------|
| Seconds | `float` | The duration in seconds. |
| bIncludeSeconds | `bool` | Whether to include the seconds unit. |

**Returns:** The formatted duration text.

## Format Relative Time
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Formatting`

```cpp
static FText FormatRelativeTime(const FDateTime& Timestamp);
```

Formats a timestamp relative to the current local time: `just now` (under a minute), `N minute(s)/hour(s)/day(s) ago`, or `in N ...` for future timestamps. Uses local time, pairing with Get Save Slot Timestamp. Not pure because it reads the current clock each call. Output is English-only.

| Parameter | Type | Description |
|-----------|------|-------------|
| Timestamp | `const FDateTime&` | The local timestamp to describe. |

**Returns:** The formatted relative time text.

## Get Int Array Sum
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static int64 GetIntArraySum(const TArray<int32>& Values);
```

Returns the sum of an integer array as a 64-bit integer, so large arrays cannot overflow int32.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<int32>&` | The values to sum. |

**Returns:** The sum of the values, or 0 if the array is empty.

## Get Int Array Average
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetIntArrayAverage(const TArray<int32>& Values);
```

Returns the arithmetic mean of an integer array.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<int32>&` | The values to average. |

**Returns:** The average of the values, or 0 if the array is empty.

## Get Int Array Median
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetIntArrayMedian(const TArray<int32>& Values);
```

Returns the median of an integer array (computed on a sorted copy; the input is not modified). For an even count, returns the average of the two middle values.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<int32>&` | The values to take the median of. |

**Returns:** The median of the values, or 0 if the array is empty.

## Get Int Array Standard Deviation
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetIntArrayStandardDeviation(const TArray<int32>& Values);
```

Returns the population standard deviation of an integer array (divides by N, not N-1).

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<int32>&` | The values to measure. |

**Returns:** The population standard deviation, or 0 if the array is empty.

## Get Float Array Sum
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetFloatArraySum(const TArray<float>& Values);
```

Returns the sum of a float array. Accumulates in double internally for precision.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to sum. |

**Returns:** The sum of the values, or 0 if the array is empty.

## Get Float Array Average
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetFloatArrayAverage(const TArray<float>& Values);
```

Returns the arithmetic mean of a float array. Accumulates in double internally for precision.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to average. |

**Returns:** The average of the values, or 0 if the array is empty.

## Get Float Array Median
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetFloatArrayMedian(const TArray<float>& Values);
```

Returns the median of a float array (computed on a sorted copy; the input is not modified). For an even count, returns the average of the two middle values.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to take the median of. |

**Returns:** The median of the values, or 0 if the array is empty.

## Get Float Array Standard Deviation
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetFloatArrayStandardDeviation(const TArray<float>& Values);
```

Returns the population standard deviation of a float array (divides by N, not N-1). Accumulates in double internally for precision.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to measure. |

**Returns:** The population standard deviation, or 0 if the array is empty.

## Get Angle Array Average
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static bool GetAngleArrayAverage(const TArray<float>& Angles, float& AverageAngle, float& ResultantStrength);
```

Calculates a circular mean in degrees across the -180/180 and 0/360 seams. Resultant Strength ranges from 0 for fully dispersed directions to 1 for identical directions.

| Parameter | Type | Description |
|-----------|------|-------------|
| Angles | `const TArray<float>&` | The angles in degrees. |
| AverageAngle | `float&` | Receives the circular mean in the [-180, 180] range. |
| ResultantStrength | `float&` | Receives the directional concentration in the [0, 1] range. |

**Returns:** False for an empty array, non-finite input, or an undefined or numerically indeterminate mean such as two opposite directions.

## Get Weighted Float Array Average
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static bool GetWeightedFloatArrayAverage(const TArray<float>& Values, const TArray<float>& Weights, float& Average);
```

Calculates a weighted arithmetic mean. Negative and non-finite weights are treated as zero.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to average. |
| Weights | `const TArray<float>&` | One weight per value. |
| Average | `float&` | Receives the weighted average. |

**Returns:** False when the arrays are empty or differ in size, a value is non-finite, or no positive weight remains.

## Get Weighted Vector Array Average
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static bool GetWeightedVectorArrayAverage(const TArray<FVector>& Values, const TArray<float>& Weights, FVector& Average);
```

Calculates a component-wise weighted vector average. Negative and non-finite weights are treated as zero.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<FVector>&` | The vectors to average. |
| Weights | `const TArray<float>&` | One weight per vector. |
| Average | `FVector&` | Receives the weighted average. |

**Returns:** False when the arrays are empty or differ in size, a vector is non-finite, or no positive weight remains.

## Normalize Float Array To Range
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static bool NormalizeFloatArrayToRange(const TArray<float>& Values, float OutputMinimum, float OutputMaximum,
    TArray<float>& NormalizedValues);
```

Maps an array's minimum and maximum values to an output range. Reversed output bounds are accepted. A constant array maps to Output Minimum.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to normalize. |
| OutputMinimum | `float` | The output value assigned to the input minimum. |
| OutputMaximum | `float` | The output value assigned to the input maximum. |
| NormalizedValues | `TArray<float>&` | Receives the normalized values in their original order. |

**Returns:** False for an empty array or non-finite input and clears the output.

## Normalize Weights
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static bool NormalizeWeights(const TArray<float>& Weights, TArray<float>& NormalizedWeights);
```

Normalizes positive weights so their sum is one. Negative and non-finite weights become zero.

| Parameter | Type | Description |
|-----------|------|-------------|
| Weights | `const TArray<float>&` | The weights to normalize. |
| NormalizedWeights | `TArray<float>&` | Receives the normalized weights. |

**Returns:** False for an empty array or when no positive weight remains and clears the output.

## Get Float Array Percentile
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static bool GetFloatArrayPercentile(const TArray<float>& Values, float Percentile, float& Value);
```

Calculates a percentile using the Type 7 linear method. For a sorted array of `N` values, the sample position is `(N - 1) * Percentile / 100`, with linear interpolation between adjacent values. Percentile is clamped to the [0, 100] range. The input array is not modified.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The sample values. |
| Percentile | `float` | The requested percentile from 0 to 100. |
| Value | `float&` | Receives the interpolated percentile value. |

**Returns:** False for an empty array or non-finite input.

## Get Float Array Root Mean Square
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static bool GetFloatArrayRootMeanSquare(const TArray<float>& Values, float& RootMeanSquare);
```

Calculates the square root of the arithmetic mean of the squared values.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to measure. |
| RootMeanSquare | `float&` | Receives the root mean square. |

**Returns:** False for an empty array, non-finite input, or a non-finite result.

## Get Random Index From Weights
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static int32 GetRandomIndexFromWeights(const TArray<float>& Weights);
```

Returns a random index into the Weights array, where each index's probability is proportional to its weight. Useful for loot tables and weighted spawning. Negative and non-finite weights are treated as zero.

| Parameter | Type | Description |
|-----------|------|-------------|
| Weights | `const TArray<float>&` | The per-index weights. |

**Returns:** The selected index, or INDEX_NONE (-1) if the array is empty or all weights are zero.

## Get Random Index From Weights (Stream)
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static int32 GetRandomIndexFromWeightsFromStream(UPARAM(ref) FRandomStream& Stream, const TArray<float>& Weights);
```

Deterministic version of Get Random Index From Weights that draws from (and advances) the provided random stream.

| Parameter | Type | Description |
|-----------|------|-------------|
| Stream | `FRandomStream&` | The random stream to draw from. |
| Weights | `const TArray<float>&` | The per-index weights. Negative and non-finite weights are treated as zero. |

**Returns:** The selected index, or INDEX_NONE (-1) if the array is empty or all weights are zero.

## Random Point In Circle
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static FVector2D RandomPointInCircle(float Radius);
```

Returns a point uniformly distributed by area inside a 2D circle. Negative radii use their absolute value.

| Parameter | Type | Description |
|-----------|------|-------------|
| Radius | `float` | The circle radius. |

**Returns:** A random point inside the circle, or zero for a zero or non-finite radius.

## Random Point In Circle (Stream)
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static FVector2D RandomPointInCircleFromStream(UPARAM(ref) FRandomStream& Stream, float Radius);
```

Deterministic version of Random Point In Circle that draws from and advances the provided stream. A zero or non-finite radius returns zero without advancing the stream.

| Parameter | Type | Description |
|-----------|------|-------------|
| Stream | `FRandomStream&` | The random stream to draw from. |
| Radius | `float` | The circle radius. |

**Returns:** A random point inside the circle, or zero for a zero or non-finite radius.

## Random Point In Annulus
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static FVector2D RandomPointInAnnulus(float InnerRadius, float OuterRadius);
```

Returns a point uniformly distributed by area between two 2D radii. Reversed and negative radii are accepted.

| Parameter | Type | Description |
|-----------|------|-------------|
| InnerRadius | `float` | One annulus radius. |
| OuterRadius | `float` | The other annulus radius. |

**Returns:** A random point inside the annulus, or zero when both radii are zero or either is non-finite.

## Random Point In Annulus (Stream)
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static FVector2D RandomPointInAnnulusFromStream(UPARAM(ref) FRandomStream& Stream, float InnerRadius, float OuterRadius);
```

Deterministic version of Random Point In Annulus that draws from and advances the provided stream. Zero radii or a non-finite radius return zero without advancing the stream.

| Parameter | Type | Description |
|-----------|------|-------------|
| Stream | `FRandomStream&` | The random stream to draw from. |
| InnerRadius | `float` | One annulus radius. |
| OuterRadius | `float` | The other annulus radius. |

**Returns:** A random point inside the annulus, or zero when both radii are zero or either is non-finite.

## Random Point In Sphere
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static FVector RandomPointInSphere(float Radius);
```

Returns a point uniformly distributed by volume inside a sphere. Negative radii use their absolute value.

| Parameter | Type | Description |
|-----------|------|-------------|
| Radius | `float` | The sphere radius. |

**Returns:** A random point inside the sphere, or zero for a zero or non-finite radius.

## Random Point In Sphere (Stream)
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static FVector RandomPointInSphereFromStream(UPARAM(ref) FRandomStream& Stream, float Radius);
```

Deterministic version of Random Point In Sphere that draws from and advances the provided stream. A zero or non-finite radius returns zero without advancing the stream.

| Parameter | Type | Description |
|-----------|------|-------------|
| Stream | `FRandomStream&` | The random stream to draw from. |
| Radius | `float` | The sphere radius. |

**Returns:** A random point inside the sphere, or zero for a zero or non-finite radius.

---

## EDirectiveUtilRadialOrientation

Rotation applied to transforms generated around a center point.

| Value | Display Name | Description |
|-------|--------------|-------------|
| Fixed | Fixed | Uses the plane rotation plus Rotation Offset for every instance. |
| FaceCenter | Face Center | Local X faces the center. |
| FaceAwayFromCenter | Face Away From Center | Local X faces away from the center. |
| FollowPath | Follow Path | Local X follows the path tangent around the circle or arc. |
| FaceAgainstPath | Face Against Path | Local X faces against the path tangent. |

## EDirectiveUtilSplineSpacingMode

Spacing behavior for spline sample generation.

| Value | Display Name | Description |
|-------|--------------|-------------|
| Fixed | Fixed | Samples every Spacing units. The final interval may be shorter. |
| Even | Even | Shrinks Spacing so the samples divide the range evenly. |

## EDirectiveUtilEaseType

Easing curves not provided by the engine's built-in Ease node (EEasingFunc): the classic Penner Back, Elastic and Bounce curves, plus Linear for un-eased interpolation.

| Value | Display Name | Description |
|-------|--------------|-------------|
| BackIn | Back In | Overshoots slightly at the start before easing in. |
| BackOut | Back Out | Overshoots slightly past the end before settling. |
| BackInOut | Back In Out | Overshoots at both the start and the end. |
| ElasticIn | Elastic In | Oscillates with increasing amplitude before easing in. |
| ElasticOut | Elastic Out | Oscillates with decreasing amplitude after the end. |
| ElasticInOut | Elastic In Out | Oscillates at both the start and the end. |
| BounceIn | Bounce In | Bounces with increasing energy before easing in. |
| BounceOut | Bounce Out | Bounces with decreasing energy after the end. |
| BounceInOut | Bounce In Out | Bounces at both the start and the end. |
| Linear | Linear | Interpolates at a constant rate without easing. |
