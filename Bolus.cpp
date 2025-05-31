#include "Bolus.h"
#include <iostream>

using namespace std;

Bolus::Bolus()
    : glucose_level(0.0), carb_intake(0.0), calculator(nullptr) {}

Bolus::Bolus(double glucose_level, double carb_intake)
    : glucose_level(glucose_level), carb_intake(carb_intake), calculator(nullptr) {}

void Bolus::Init() {
    cout << "Bolus initiation started." << endl;
    cout << "Current Blood Glucose Level: " << glucose_level << endl;
    cout << "Current Carbohydrate Intake: " << carb_intake << endl;

    double recommendedDose = 0.0;
    if (calculator) {
        recommendedDose = (glucose_level / 50.0) + (carb_intake / 10.0);
    } else {
        recommendedDose = (glucose_level / 50.0) + (carb_intake / 10.0);
    }
    cout << "Recommended bolus dose: " << recommendedDose << " units." << endl;
}

void Bolus::manualOverride(double newDose) {
    cout << "Bolus dose overridden to: " << newDose << " units." << endl;
    // In a real implementation, you would update a member variable to hold the new dose.
}

void Bolus::startExtendedBolus(double duration) {
    cout << "Extended bolus started. Delivery will be spread over " << duration << " minutes." << endl;
}

void Bolus::startQuickBolus() {
    cout << "Quick bolus initiated for immediate correction." << endl;
}

void Bolus::stop() {
    cout << "Bolus delivery has been stopped." << endl;
}
