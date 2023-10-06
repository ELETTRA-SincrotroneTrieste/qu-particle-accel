#ifndef QUPAITEM_H
#define QUPAITEM_H

#include <QGraphicsSvgItem>

class QuPAItem_P;

class QuPAItem : public QGraphicsSvgItem
{
public:
    QuPAItem(const QSize& size, QGraphicsItem *parent = nullptr);
    QuPAItem(int width, int height, QGraphicsItem* parent = nullptr);
    virtual ~QuPAItem();

    QRectF boundingRect() const override;
    virtual void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;


private:
    QuPAItem_P* d;
};

#endif // QUPAITEM_H
