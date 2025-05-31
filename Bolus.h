#ifndef BOLUS_H
#define BOLUS_H

class Calculator;

class Bolus {
public:
    Bolus();
    Bolus(double glucose_level, double carb_intake);
    double glucose_level;
    double carb_intake;
    Calculator* calculator;
    void Init();
    void manualOverride(double newDose);
    void startExtendedBolus(double duration);
    void startQuickBolus();
    void stop();
};

#endif // BOLUS_H
