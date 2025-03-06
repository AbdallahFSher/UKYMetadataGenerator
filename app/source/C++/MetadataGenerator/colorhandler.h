#ifndef COLORHANDLER_H
#define COLORHANDLER_H

#include <QColor>;
#include <QPalette>;

class ColorHandler
{
public:
    ColorHandler();
    QPalette getPalette();
    QColor* getColors();
private:
    QColor colorList[7] = {
        QColor("#EE7733"), //0. orange
        QColor("#0077BB"), //1. blue
        QColor("#33BBEE"), //2. light-blue
        QColor("#EE3377"), //3. pink
        QColor("#CC3311"), //4. red
        QColor("#009988"), //5. blue-green
        QColor("#BBBBBB") //6. gray
    };

    QPalette palette;
};

#endif // COLORHANDLER_H
