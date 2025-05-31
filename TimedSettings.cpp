#include "TimedSettings.h"
#include <iostream>
using namespace std;

TimedSettings::TimedSettings()
    : basal_rate(0.0), correction_factor(0.0), carb_ratio(0.0), target_bg(0.0) {}

TimedSettings::TimedSettings(double basal, double correction, double carbRatio, double targetBG)
    : basal_rate(basal), correction_factor(correction), carb_ratio(carbRatio), target_bg(targetBG) {}

void TimedSettings::Update(double basal, double correction, double carbRatio, double targetBG)
{
    basal_rate = basal;
    correction_factor = correction;
    carb_ratio = carbRatio;
    target_bg = targetBG;
    Logger::log("TimedSettings updated: Basal " + std::to_string(basal_rate)
         + ", Correction " + std::to_string(correction_factor)
         + ", Carb Ratio " + std::to_string(carb_ratio)
         + ", Target BG " + std::to_string(target_bg));
}

void TimedSettings::Display() const
{
    cout << "Timed Settings:" << endl;
    cout << "Basal: " << basal_rate
         << ", Correction Factor: " << correction_factor
         << ", Carb Ratio: " << carb_ratio
         << ", Target BG: " << target_bg << endl;
}
