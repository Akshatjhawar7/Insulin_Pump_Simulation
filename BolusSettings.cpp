#include "BolusSettings.h"
#include <iostream>
using namespace std;

BolusSettings::BolusSettings()
    : immediateDose(0.0), extendedDose(0.0), duration(0.0) {}

BolusSettings::BolusSettings(double immediate, double extended, double dur)
    : immediateDose(immediate), extendedDose(extended), duration(dur) {}

void BolusSettings::Update(double immediate, double extended, double dur)
{
    immediateDose = immediate;
    extendedDose = extended;
    duration = dur;
    Logger::log( "BolusSettings updated: Immediate " + std::to_string(immediateDose) + ", Extended " + std::to_string(extendedDose) + ", Duration " + std::to_string(duration));
}

void BolusSettings::Display() const
{
    cout << "Bolus Settings:" << endl;
    cout << "Immediate: " << immediateDose
         << ", Extended: " << extendedDose
         << ", Duration: " << duration << endl;
}
