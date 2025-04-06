#include "colorhandler.h"
#include <iostream>
#include <QColor>
#include <QPalette>
#include "node.h"

ColorHandler::ColorHandler()
{
    palette.setColor(QPalette::Window, bg);
}

QPalette ColorHandler::getPalette() const
{
    return palette;
}

const QColor* ColorHandler::getColors() const
{
    return colorList;
}

void ColorHandler::setColors(const std::vector<Node*>& nodes)
{
    for (Node* node : nodes) {
        int variant = node->nodeVariant;

        // Ensure variant stays within valid range
        if (variant < 0 || variant >= COLOR_COUNT) {
            variant = 0; // Default to first color
            node->nodeVariant = variant; // Optional: correct invalid variant
        }

        QColor color = colorList[variant];
        QString style = QString("background: %1; color: black;")
                            .arg(color.name());
        node->setStyleSheet(style);
    }
}
