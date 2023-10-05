#ifndef QUSTORAGERINGVIEW_H
#define QUSTORAGERINGVIEW_H

#include <QGraphicsView>
#include <cumbiapool.h>

class CuControlsFactoryPool;

class QuStorageRingView : public QGraphicsView
{
    Q_OBJECT
public:
    QuStorageRingView(QWidget* parent, CumbiaPool* cu_pool, const CuControlsFactoryPool &fp);
    bool load(const QString& jsonf = QString());
};

#endif // QUSTORAGERINGVIEW_H
