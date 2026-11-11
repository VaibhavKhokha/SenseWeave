#pragma once
#define MODEL_NAME "NaiveBayes_IMU_Only"
#define NUM_FEATURES 6
#define NUM_TEST_VECTORS 3
namespace senseweave {
    const float kTestVectors[NUM_TEST_VECTORS][NUM_FEATURES] = {
        { 0.002525f, 0.021373f, 1.005947f, -7.509794f, 1.577238f, -8.895731f },
        { 0.522566f, 0.226984f, 0.686508f, -18.223460f, -3.746330f, 3.852866f },
        { 4.539267f, -1.218887f, 4.263798f, 134.324666f, -134.662815f, 205.350925f },
    };
}
