#include "qustorageringview.h"
#include <QGraphicsScene>

QuStorageRingView::QuStorageRingView(QWidget *parent,
                                     CumbiaPool *cu_pool,
                                     const CuControlsFactoryPool &fp)
    : QGraphicsView(parent) {
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);
}

bool QuStorageRingView::load(const QString &jsonf) {

}
