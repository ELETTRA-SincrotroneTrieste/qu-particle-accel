#ifndef QUJSON2SVG_W_H
#define QUJSON2SVG_W_H

#include <QWidget>
#include <QDomDocument>
#include <QMap>
#include <QGraphicsView>
#include <QSet>

class QuStorageRingView_P;
class QGraphicsScene;

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    MyGraphicsView(QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
};

/*!
 * \brief Storage ring view loaded from a JSON file
 *
 * This class loads a view of the storage ring loaded from a JSON file
 */
class QuJsonToSvgW : public QWidget
{
    Q_OBJECT
public:
    QuJsonToSvgW(QWidget* parent);
    bool error() const;
    QStringList msgs() const;
    QGraphicsScene *scene() const;


private slots:
    void rescale();
    void save_svg();
    void open_json();
    void load();
    void set_out_file();

signals:
    void op(const QStringList &);

private:
    void m_add_component(const QString& type, const QString &id, const QPointF &pt, double rad, QDomElement &svgroot, const QMap<QString, QString>& props);
    bool m_get_bounds(double *max_x, double *max_y, double* x_offset, double *y_offset, const QJsonObject &root) const;
    bool m_get_coords(const QJsonValue& start_v, double *x, double* y, double *xs, double *ys, double *last_xs, double* last_ys) const;
    bool m_add_subcomponents(const QJsonValue &components_v, const QPointF &p0, const QPointF &p1, QMap<QString, QString> &props, QDomElement &svg);
    QMap<QString, QString> m_map_props(const QJsonObject& jo) const;
    QPointF m_transform(const QPointF &pabs) const;
    void m_set_id(const QString& id, QDomElement& e);
    bool m_load();
    QuStorageRingView_P *d;
    QDomElement m_recursive_set_id(const QDomElement &parente);
    QStringList m_ids;
};

#endif // QUJSON2SVG_W_H
