#include "drawconnection.h"
#include <QPainter>
#include <QPen>

DrawConnection::DrawConnection(QWidget *parent)
    : QWidget{parent}
{}

void DrawConnection::addWidgets(const QWidget * from, const QWidget * to)
{
    list.append(WidgetsConnected{from , to});
    update();
}

void DrawConnection::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen = QPen();
    pen.setColor(Qt::blue);
    pen.setWidth(5);
    painter.setPen(pen);
    for(const WidgetsConnected el: list){
        const QWidget* from = el.from;
        const QWidget* to = el.to;

        QPoint start =  from->mapToGlobal(from->rect().topRight() +  QPoint(0, from->height()/2));
        QPoint end = to->mapToGlobal(to->rect().topLeft() +  QPoint(0, to->height()/2));

        if(to->isVisible())
            painter.drawLine(mapFromGlobal(start), mapFromGlobal(end));
    }
}
