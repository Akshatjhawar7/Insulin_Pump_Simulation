#include "Calculator.h"
#include <iostream>
#include <cstdlib>
#include <QtGlobal>
#include <ctime>
using namespace std;

CalculationResult Calculator::calculateBolus(double carbs,
                                             double bg,
                                             double targetBG,
                                             double correctionFactor,
                                             double carbRatio,
                                             double IOB)
{
    CalculationResult result;
    result.foodBolus = carbs / carbRatio;
    result.correctionBolus = (bg > targetBG) ? (bg - targetBG) / correctionFactor : 0.0;
    result.totalBolus = result.foodBolus + result.correctionBolus;
    result.finalBolus = (result.totalBolus > IOB) ? (result.totalBolus - IOB) : 0.0;

    cout << "[DEBUG][Calculator] Carbs: " << carbs
         << ", BG: " << bg
         << ", Target BG: " << targetBG << endl;
    cout << "[DEBUG][Calculator] Food Bolus: " << result.foodBolus
         << " units, Correction Bolus: " << result.correctionBolus
         << " units" << endl;
    cout << "[DEBUG][Calculator] Total Bolus: " << result.totalBolus
         << " units, Final Bolus: " << result.finalBolus << IOB
         << " units" << endl;

    return result;
}

ExtendedBolusResult Calculator::calculateExtendedBolus(double finalBolus, double duration)
{
    // Original fixed 60%/40% approach
    ExtendedBolusResult result;
    result.immediateBolus = 0.6 * finalBolus;
    result.extendedBolus = 0.4 * finalBolus;
    result.bolusPerHour = result.extendedBolus / duration;

    cout << "[DEBUG][Calculator] Extended Bolus Calculation (60/40): "
         << "Immediate: " << result.immediateBolus
         << " units, Extended: " << result.extendedBolus
         << " units, Bolus per Hour: " << result.bolusPerHour
         << " units/hr" << endl;

    return result;
}

ExtendedBolusResult Calculator::calculateExtendedBolusCustom(double finalBolus,
                                                             double immediatePercent,
                                                             double extendedPercent,
                                                             double duration)
{
    ExtendedBolusResult result;
    // Convert percentages (e.g. 60, 40) to fractions (0.60, 0.40)
    double immFrac = immediatePercent / 100.0;
    double extFrac = extendedPercent / 100.0;

    result.immediateBolus = immFrac * finalBolus;
    result.extendedBolus  = extFrac * finalBolus;
    if (duration <= 0) duration = 1.0;  // prevent divide-by-zero
    result.bolusPerHour   = result.extendedBolus / duration;

    cout << "[DEBUG][Calculator] Extended Bolus Calculation (Custom): "
         << "Immediate: " << result.immediateBolus
         << " units, Extended: " << result.extendedBolus
         << " units, Bolus per Hour: " << result.bolusPerHour
         << " units/hr" << endl;

    return result;
}

double Calculator::calculatePredictedBG(double currentBG)
{
    // Generate a random offset between -1.0 and 1.0
    double randomOffset = (qrand() % 2001 - 1000) / 1000.0;  // [-1.0, 1.0]
    double predictedBG = currentBG + randomOffset;
    cout << "[DEBUG][Calculator] Predicted BG: " << predictedBG
         << " (Current BG: " << currentBG
         << ", Offset: " << randomOffset << ")" << endl;
    return predictedBG;
}
