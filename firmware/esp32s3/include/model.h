#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class SenseWeaveModel {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        float votes[3] = { 0.0f };
                        float theta[6] = { 0 };
                        float sigma[6] = { 0 };
                        theta[0] = 0.001097551326; theta[1] = -0.000316198772; theta[2] = 0.999968306765; theta[3] = 4.781873340641; theta[4] = 2.274852194626; theta[5] = -0.244113550315;
                        sigma[0] = 0.000411305132; sigma[1] = 0.000439232428; sigma[2] = 0.000356206901; sigma[3] = 200.376635606; sigma[4] = 84.967308416402; sigma[5] = 148.800542906993;
                        votes[0] = 0.495711835334 - gauss(x, theta, sigma);
                        theta[0] = 0.176156663761; theta[1] = 0.506586346103; theta[2] = 1.002102461544; theta[3] = 6.556736324131; theta[4] = 1.897786410518; theta[5] = -0.089296574898;
                        sigma[0] = 0.085604008656; sigma[1] = 0.165464721103; sigma[2] = 0.238897345724; sigma[3] = 201.586154953886; sigma[4] = 102.955255495272; sigma[5] = 135.260397466651;
                        votes[1] = 0.38936535163 - gauss(x, theta, sigma);
                        theta[0] = 2.759225183381; theta[1] = -1.587705945411; theta[2] = 2.68309592756; theta[3] = 140.930727852611; theta[4] = -122.473399945136; theta[5] = 196.232163982379;
                        sigma[0] = 2.218318432341; sigma[1] = 3.118880179322; sigma[2] = 6.431750848697; sigma[3] = 2705.627195774043; sigma[4] = 2686.21639670851; sigma[5] = 4138.72825341045;
                        votes[2] = 0.114922813036 - gauss(x, theta, sigma);
                        // return argmax of votes
                        uint8_t classIdx = 0;
                        float maxVotes = votes[0];

                        for (uint8_t i = 1; i < 3; i++) {
                            if (votes[i] > maxVotes) {
                                classIdx = i;
                                maxVotes = votes[i];
                            }
                        }

                        return classIdx;
                    }

                protected:
                    /**
                    * Compute gaussian value
                    */
                    float gauss(float *x, float *theta, float *sigma) {
                        float gauss = 0.0f;

                        for (uint16_t i = 0; i < 6; i++) {
                            gauss += log(sigma[i]);
                            gauss += abs(x[i] - theta[i]) / sigma[i];
                        }

                        return gauss;
                    }
                };
            }
        }
    }