#ifndef COLORHANDLER_H
#define COLORHANDLER_H

#include "node.h"

#include <QColor>;
#include <QPalette>;

class ColorHandler
{
public:
    ColorHandler();
    QPalette getPalette();
    QColor* getColors();
    void setColors(std::vector<Node*> nodes);
    QColor bg = QColor("#DDDDDD"); //7. gray

private:
    QColor colorList[8] = {
        QColor("#CC6677"), //0. light-red
        QColor("#DDCC77"), //1. yellow
        QColor("#117733"), //2. green
        QColor("#88CCEE"), //3. light-blue
        QColor("#44AA99"), //4. blue-green
        QColor("#882255"), //5. rose
        QColor("#AA4499"), //6. pink
    };

    QPalette palette;
};

#endif // COLORHANDLER_H
