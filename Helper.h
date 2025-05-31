#ifndef HELPER_H
#define HELPER_H

#include <QWidget>

// Forward declaration of MainWindow so we can declare functions that take MainWindow*
class MainWindow;

namespace Helper {

// Transfers the personal profiles UI update code.
void updatePersonalProfilesUI(MainWindow *mw);

// Transfers the update dialog for profile settings.
void showUpdateDialog(MainWindow *mw);

// Creates and returns a pointer to the Extended Bolus screen.
QWidget* createExtendedBolusScreen(MainWindow *mw);

// Updates the UI for power state based on m_powerOn.
void updateUIForPowerState(MainWindow *mw);

// Updates pump infusion logic based on IOB and reservoir.
// (Called periodically by the pump timer.)
void updatePumpInfusion(MainWindow *mw);

// Updates the bolus calculation based on carbs, BG, and profile parameters.
void updateBolusCalculation(MainWindow *mw);

// Offers a reduction to the bolus units when BG is below target.
void maybeOfferReduction(MainWindow *mw, double &units);

// Sets the visibility of the simulation (graph container).
void setSimulationVisible(MainWindow *mw, bool visible);

} // namespace Helper

#endif // HELPER_H
