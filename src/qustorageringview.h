#ifndef QUSTORAGERINGVIEW_H
#define QUSTORAGERINGVIEW_H

#include <QGraphicsView>
#include <QDomDocument>
#include <cumbiapool.h>
#include <QMap>

class CuControlsFactoryPool;
class QuStorageRingView_P;


/*!
 * \brief Storage ring view loaded from a JSON file
 *
 * This class loads a view of the storage ring loaded from a JSON file
 */
class QuStorageRingView : public QGraphicsView
{
    Q_OBJECT
public:
    QuStorageRingView(QWidget* parent, CumbiaPool* cu_pool, const CuControlsFactoryPool &fp);
    bool load(const QString& jsonf = QString());
    bool error() const;
    QStringList msgs() const;

protected:
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);

private:
    void m_add_component(const QString& type, const QPointF &pt, double rad, QDomElement &svgroot, const QMap<QString, QString>& props);
    bool m_get_bounds(double *max_x, double *max_y, double* x_offset, double *y_offset, const QJsonObject &root) const;
    bool m_get_coords(const QJsonValue& start_v, double *x, double* y, double *xs, double *ys, double *last_xs, double* last_ys) const;
    bool m_add_subcomponents(const QJsonValue &components_v, const QPointF &p0, const QPointF &p1, QMap<QString, QString> &props, QDomElement &svg);
    QPointF m_transform(const QPointF &pabs) const;
    QuStorageRingView_P *d;
};

#endif // QUSTORAGERINGVIEW_H
