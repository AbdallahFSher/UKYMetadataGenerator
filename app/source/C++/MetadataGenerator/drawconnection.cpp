// drawconnection.cpp
#include "drawconnection.h"
#include <QPainter>
#include <QPen>

DrawConnection::DrawConnection(QWidget *parent)
    : QWidget{parent}
{}

// add a new line between two widgets
void DrawConnection::addWidgets(const QWidget *from, const QWidget *to)
{
    list.append(WidgetsConnected{from, to});
    update();
}

// remove any connections matching this pair
void DrawConnection::removeWidgets(const QWidget *from, const QWidget *to)
{
    for (int i = list.size() - 1; i >= 0; --i) {
        if (list[i].from == from && list[i].to == to)
            list.removeAt(i);
    }
    update();
}

// paint all visible connections
void DrawConnection::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen(Qt::blue, 5);
    painter.setPen(pen);

    for (const WidgetsConnected &el : list) {
        const QWidget* from = el.from;
        const QWidget* to   = el.to;

        if (!to->isVisible())
            continue;

        QPoint start = from->mapToGlobal(
            from->rect().topRight() + QPoint(0, from->height()/2));
        QPoint end = to->mapToGlobal(
            to->rect().topLeft() + QPoint(0, to->height()/2));

        painter.drawLine(mapFromGlobal(start), mapFromGlobal(end));
    }
}
