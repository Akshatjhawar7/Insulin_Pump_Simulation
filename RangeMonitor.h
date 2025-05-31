#ifndef RANGEMONITOR_H
#define RANGEMONITOR_H
#include "Logger.h"

#include <QString>
#include <QLabel>

/**
 * @brief RangeMonitor decides which symbol/message to display
 * based on the current BG (blood glucose) value.
 */
class RangeMonitor
{
public:
    RangeMonitor();

    /**
     * @brief Updates the given label with the symbol/text and color
     *        based on the BG value.
     * @param bg The current or predicted BG in mmol/L
     * @param label A pointer to a QLabel that will be updated
     */
    void updateSymbol(double bg, QLabel *label);

private:
    // Helper function or constants if you like
};

#endif // RANGEMONITOR_H
