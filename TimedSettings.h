#ifndef TIMEDSETTINGS_H
#define TIMEDSETTINGS_H
#include "Logger.h"

class TimedSettings {
public:
    TimedSettings();
    TimedSettings(double basal, double correction, double carbRatio, double targetBG);

    void Update(double basal, double correction, double carbRatio, double targetBG);
    void Display() const;

    double basal_rate;
    double correction_factor;
    double carb_ratio;
    double target_bg;
};

#endif // TIMEDSETTINGS_H
