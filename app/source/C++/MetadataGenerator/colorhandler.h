#ifndef COLORHANDLER_H
#define COLORHANDLER_H

#include "node.h"
#include <QColor>
#include <QPalette>

class ColorHandler
{
public:
    ColorHandler();
    QPalette getPalette() const;
    const QColor* getColors() const;
    void setColors(const std::vector<Node*>& nodes);

private:
    static constexpr int COLOR_COUNT = 7;
    QColor bg = QColor("#DDDDDD"); // Background gray

    // Node color variants (0-6)
    QColor colorList[COLOR_COUNT] = {
        QColor("#CC6677"), // 0. Light red/pink
        QColor("#DDCC77"), // 1. Yellow
        QColor("#117733"), // 2. Green
        QColor("#88CCEE"), // 3. Light blue
        QColor("#44AA99"), // 4. Blue-green
        QColor("#882255"), // 5. Rose
        QColor("#AA4499")  // 6. Pink
    };

    QPalette palette;
};

#endif // COLORHANDLER_H
