#ifndef CALCULATOR_H
#define CALCULATOR_H
#include "Logger.h"

struct CalculationResult {
    double foodBolus;
    double correctionBolus;
    double totalBolus;
    double finalBolus;
};

struct ExtendedBolusResult {
    double immediateBolus;
    double extendedBolus;
    double bolusPerHour;
};

class Calculator
{
public:
    static CalculationResult calculateBolus(double carbs,
                                            double bg,
                                            double targetBG,
                                            double correctionFactor,
                                            double carbRatio,
                                            double IOB);

    // Original 60/40 approach (kept for reference if needed)
    static ExtendedBolusResult calculateExtendedBolus(double finalBolus, double duration);

    // NEW: Custom approach using user-specified immediate and extended percentages
    static ExtendedBolusResult calculateExtendedBolusCustom(double finalBolus,
                                                            double immediatePercent,
                                                            double extendedPercent,
                                                            double duration);

    static double calculatePredictedBG(double currentBG);
};

#endif // CALCULATOR_H
