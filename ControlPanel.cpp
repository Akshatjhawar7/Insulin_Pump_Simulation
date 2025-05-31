#include "ControlPanel.h"
#include "Insulin.h"
#include <iostream>


ControlPanel::ControlPanel(int insulinLevel){
    isPaused = false;
    insulin = new Insulin(insulinLevel);
}

ControlPanel::~ControlPanel(){
    delete insulin;
}
void ControlPanel::shutoff(){
    std::cout<< "Shutting off" << std::endl;
    exit(0);
}

void ControlPanel::sleep(){
    std::cout<< "System going to sleep" << std::endl;
    isPaused = true;
    while(isPaused){
        continue;
    }
}

void ControlPanel::resume(){
    if(!isPaused){
        std::cout << "System already running!" << std::endl;
        return;
    }
    std::cout<< "System resuming" << std::endl;
    isPaused = false;
}


// add person
