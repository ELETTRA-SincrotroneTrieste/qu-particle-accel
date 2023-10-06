#include "qupaitem.h"
#include "qsvgrenderer.h"

#include <QPainter>
#include <QtDebug>

class QuPAItem_P {
public:
    QuPAItem_P(const QSize& s) : size(s) {}

    QSize size;
};

QuPAItem::QuPAItem(const QSize &size, QGraphicsItem *parent)
    : QGraphicsSvgItem(parent) {
    d = new QuPAItem_P(size);
}

QuPAItem::QuPAItem(int width, int height, QGraphicsItem *parent) : QGraphicsSvgItem(parent) {
    d = new QuPAItem_P(QSize(width, height));
}

QuPAItem::~QuPAItem() {
    delete d;
}

QRectF QuPAItem::boundingRect() const {
    return QRectF(0, 0, d->size.width(), d->size.height());
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
//    painter->drawRect(boundingRect());
}
