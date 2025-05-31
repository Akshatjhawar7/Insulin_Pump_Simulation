#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "Insulin.h"

class ControlPanel{
public:
    ControlPanel(int);
    ~ControlPanel();
    void shutoff();
    void sleep();
    void resume();
private:
    bool isPaused;
    Insulin* insulin;
};

#endif // CONTROLPANEL_H
