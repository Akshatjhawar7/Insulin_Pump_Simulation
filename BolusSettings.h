#ifndef BOLUSSETTINGS_H
#define BOLUSSETTINGS_H
#include "Logger.h"

class BolusSettings {
public:
    BolusSettings();
    BolusSettings(double immediateDose, double extendedDose, double duration);

    void Update(double immediateDose, double extendedDose, double duration);
    void Display() const;

    double immediateDose;
    double extendedDose;
    double duration;
};

#endif // BOLUSSETTINGS_H
