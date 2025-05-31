#include "Insulin.h"
#include <iostream>

Insulin::Insulin(int level){
    insulinLevel = level;
    isPaused = false;
}

void Insulin::run(int shot){
    if(!isPaused){
        insulinLevel = insulinLevel + shot;
    }
}

void Insulin::pause(){
    std::cout<< "Pausing insulin delivery" << std::endl;
    isPaused = true;
}
