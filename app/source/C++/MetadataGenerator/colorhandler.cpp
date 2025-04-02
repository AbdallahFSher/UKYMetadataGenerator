#include "colorhandler.h"
#include <iostream>;

#include <QColor>;
#include <QPalette>;
#include "node.h"

using namespace std;

ColorHandler::ColorHandler() {
        //0. light-red
        //1. yellow
        //2. green
        //3. light-blue
        //4. blue-green
        //5. rose
        //6. pink
    this->palette.setColor(QPalette::Window, this->bg);
}

QPalette ColorHandler::getPalette() {
    return this->palette;
}


QColor* ColorHandler::getColors() {
    return this->colorList;
}

void ColorHandler::setColors(std::vector<Node*> nodes) {
    for (Node *node : nodes) {
        QColor color = colorList[node->nodeVariant];
        QString style = "background: rgb(%1, %2, %3);";
        node->setStyleSheet(style.arg(color.red()).arg(color.green()).arg(color.blue()) + "; color:black");
    }

}
