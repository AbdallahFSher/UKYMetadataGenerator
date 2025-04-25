#ifndef DRAWCONNECTION_H
#define DRAWCONNECTION_H

#include <QWidget>
#include <QList>
#include <QPaintEvent>

class DrawConnection : public QWidget
{
    Q_OBJECT

public:
    explicit DrawConnection(QWidget *parent = nullptr);
    void addWidgets(const QWidget *from, const QWidget *to);
    void removeWidgets(const QWidget *from, const QWidget *to);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct WidgetsConnected {
        const QWidget* from;
        const QWidget* to;
    };

    QList<WidgetsConnected> list;
};

#endif // DRAWCONNECTION_H
