#include "qupaitem.h"
#include "qsvgrenderer.h"

#include <QPainter>
#include <QtDebug>

class QuPAItem_P {
public:
    QuPAItem_P() {}

};

QuPAItem::QuPAItem(QGraphicsItem *parent)
    : QGraphicsSvgItem(parent) {
    d = new QuPAItem_P();
}



QuPAItem::~QuPAItem() {
    delete d;
}


void QuPAItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QSvgRenderer *r = renderer();
    if (!r || !r->isValid())
        return;
    if (elementId().isEmpty())
        r->render(painter, boundingRect());
    else
        r->render(painter, elementId(), boundingRect());
    painter->setPen(Qt::cyan);
    painter->drawRect(boundingRect());
}
