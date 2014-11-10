#include "guidrawline.h"
#include <QTimer>
#include <iostream>

GuiDrawLine::GuiDrawLine() {

}

QRectF GuiDrawLine::boundingRect() const
{
    return QRectF(0,0,0,0);
}

void GuiDrawLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    (void)option;
    (void)widget;

    QPointF origin(x1, y1);
    QPointF end(x2, y2);
    if (age == 5) {
        painter->setPen(QPen(QColor::fromRgb(0,255,0,255), 20, Qt::DotLine, Qt::RoundCap, Qt::MiterJoin));
    }
    if (age == 4) {
        painter->setPen(QPen(QColor::fromRgb(173,255,47,200), 20, Qt::DotLine, Qt::RoundCap, Qt::MiterJoin));
    }
    if (age == 3) {
        painter->setPen(QPen(QColor::fromRgb(255,255,0,155), 20, Qt::DotLine, Qt::RoundCap, Qt::MiterJoin));
    }
    if (age == 2) {
        painter->setPen(QPen(QColor::fromRgb(255,165,0,100), 20, Qt::DotLine, Qt::RoundCap, Qt::MiterJoin));
    }
    if (age == 1) {
        painter->setPen(QPen(QColor::fromRgb(255,69,0,55), 20, Qt::DotLine, Qt::RoundCap, Qt::MiterJoin));
    }
    if (age == 0) {
        painter->setPen(QPen(QColor::fromRgb(255,0,0,5), 20, Qt::DotLine, Qt::RoundCap, Qt::MiterJoin));
    }

    painter->drawLine(origin, end);
}

void GuiDrawLine::ageLine() {
    int milliseconds = lifeSpan*1000;
    int stages = 5;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(decay()));
    if (age == 5) {
        timer->start(milliseconds/stages);
    } else {
        timer->stop();
    }
}

void GuiDrawLine::decay() {
    if (age > 0) {
        age--;
//        std::cout << "age: " << age << "\n";
    } else {
        //nothing
    }

}

//void GuiDrawLine::drawLine(Point origin, Point end) {
//    //    guidrawline = new GuiDrawLine();
//    //    guidrawline->setZValue(3);
//    //    guidrawline->setX(100);
//    //    guidrawline->setY(100);
//    //    scene->addItem(guidrawline);

//            x1 = originX;
//            y1 = originY;
//            x2 = endX;
//            y2 = endY;
////            dash->ui->gView_field->update();
//}


