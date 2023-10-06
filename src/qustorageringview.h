#ifndef QUSTORAGERINGVIEW_H
#define QUSTORAGERINGVIEW_H

#include <QGraphicsView>
#include <QDomDocument>
#include <cumbiapool.h>

class CuControlsFactoryPool;
class QuStorageRingView_P;

class QuStorageRingView : public QGraphicsView
{
    Q_OBJECT
public:
    QuStorageRingView(QWidget* parent, CumbiaPool* cu_pool, const CuControlsFactoryPool &fp);
    bool load(const QString& jsonf = QString());

protected:
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);

private:
    void m_add_magnet(const QString& type, double x, double y, double rad, QDomElement &svgroot);

    QuStorageRingView_P *d;
};

#endif // QUSTORAGERINGVIEW_H
