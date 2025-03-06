#include "colorhandler.h"
#include <iostream>;

#include <QColor>;
#include <QPalette>;

using namespace std;

ColorHandler::ColorHandler() {
        //0. orange
        //1. blue
        //2. light-blue
        //3. pink
        //4. red
        //5. blue-green
        //6. gray
    this->palette.setColor(QPalette::Window, this->colorList[6]);
}

QPalette ColorHandler::getPalette() {
    return this->palette;
}


QColor* ColorHandler::getColors() {
    return this->colorList;
}
