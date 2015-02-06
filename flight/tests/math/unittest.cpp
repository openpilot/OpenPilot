#include "gtest/gtest.h"

#include <stdio.h> /* printf */
#include <stdlib.h> /* abort */
#include <string.h> /* memset */

extern "C" {
#include "mathmisc.h"
}

#define epsilon 0.00001f
// From pios_math.h
#define IS_REAL(f)           (!isnan(f) && !isinf(f))
#define length(points_array) (sizeof(points_array) / sizeof(points_array[0]))

// To use a test fixture, derive a class from testing::Test.
class MathTestRaw : public testing::Test {};

TEST_F(MathTestRaw, y_on_line0) {
    pointf points[] = {
        { 0.0f, -0.30f },
        { 0.5f, 0.30   }
    };

    EXPECT_NEAR(-0.60f, y_on_line(-0.25f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(-0.30f, y_on_line(0.00f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(0.00f, y_on_line(0.25f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(0.30f, y_on_line(0.50f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(0.60f, y_on_line(0.75f, &points[0], &points[1]), epsilon);
}

TEST_F(MathTestRaw, y_on_line1) {
    pointf points[] = {
        { 0.25f, -0.30f },
        { 0.50f, 0.30   }
    };

    EXPECT_NEAR(-1.50f, y_on_line(-0.25f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(-0.90f, y_on_line(0.00f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(-0.30f, y_on_line(0.25f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(0.30f, y_on_line(0.50f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(0.90f, y_on_line(0.75f, &points[0], &points[1]), epsilon);
}

TEST_F(MathTestRaw, y_on_line2) {
    pointf points[] = {
        { -0.25f, -0.30f },
        { 0.50f,  0.30   }
    };

    EXPECT_NEAR(-0.30f, y_on_line(-0.25f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(-0.10f, y_on_line(0.00f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(0.10f, y_on_line(0.25f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(0.30f, y_on_line(0.50f, &points[0], &points[1]), epsilon);
    EXPECT_NEAR(0.50f, y_on_line(0.75f, &points[0], &points[1]), epsilon);
}

TEST_F(MathTestRaw, y_on_line3) {
    pointf points[] = {
        { 0.25f, -0.30f },
        { 0.25f, 0.30   }
    };


    EXPECT_FALSE(IS_REAL(y_on_line(-0.25f, &points[0], &points[1])));
}

TEST_F(MathTestRaw, y_on_curve0) {
    pointf points[] = {
        { 0.00f, -0.40f },
        { 0.25f, -0.20f },
        { 0.50f, 0.00f  },
        { 0.75f, 0.20   },
        { 1.00f, 0.40   }
    };

    EXPECT_NEAR(-0.50f, y_on_curve(-0.125f, points, length(points)), epsilon);
    EXPECT_NEAR(-0.40f, y_on_curve(0.000f, points, length(points)), epsilon);
    EXPECT_NEAR(-0.30f, y_on_curve(0.125f, points, length(points)), epsilon);
    EXPECT_NEAR(-0.20f, y_on_curve(0.250f, points, length(points)), epsilon);
    EXPECT_NEAR(-0.10f, y_on_curve(0.375f, points, length(points)), epsilon);
    EXPECT_NEAR(0.00f, y_on_curve(0.500f, points, length(points)), epsilon);
    EXPECT_NEAR(0.10f, y_on_curve(0.625f, points, length(points)), epsilon);
    EXPECT_NEAR(0.20f, y_on_curve(0.750f, points, length(points)), epsilon);
    EXPECT_NEAR(0.30f, y_on_curve(0.875f, points, length(points)), epsilon);
    EXPECT_NEAR(0.40f, y_on_curve(1.000f, points, length(points)), epsilon);
    EXPECT_NEAR(0.50f, y_on_curve(1.125f, points, length(points)), epsilon);
}


TEST_F(MathTestRaw, y_on_curve1) {
    pointf points[] = {
        { -0.25f, 0.10f },
        { 0.00f,  0.20f },
        { 0.50f,  0.30f },
        { 1.00f,  -0.30 },
        { 2.00f,  -0.50 }
    };

    EXPECT_NEAR(0.00f, y_on_curve(-0.500f, points, length(points)), epsilon);
    EXPECT_NEAR(0.10f, y_on_curve(-0.250f, points, length(points)), epsilon);
    EXPECT_NEAR(0.15f, y_on_curve(-0.125f, points, length(points)), epsilon);
    EXPECT_NEAR(0.20f, y_on_curve(0.000f, points, length(points)), epsilon);
    EXPECT_NEAR(0.22f, y_on_curve(0.100f, points, length(points)), epsilon);
    EXPECT_NEAR(0.30f, y_on_curve(0.500f, points, length(points)), epsilon);
    EXPECT_NEAR(0.00f, y_on_curve(0.750f, points, length(points)), epsilon);
    EXPECT_NEAR(-0.30f, y_on_curve(1.000f, points, length(points)), epsilon);
    EXPECT_NEAR(-0.35f, y_on_curve(1.250f, points, length(points)), epsilon);
    EXPECT_NEAR(-0.50f, y_on_curve(2.000f, points, length(points)), epsilon);
}
