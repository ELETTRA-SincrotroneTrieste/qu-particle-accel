#include "qustorageringview.h"
#include "qusvgcomponentloader.h"
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QtDebug>
#include <cumacros.h>
#include <math.h>
#include <QMouseEvent>

#include <QDomDocument>
#include <QSvgRenderer>
#include <QWheelEvent>
#include <qugraphicssvgitem.h>
#include <qupaitem.h>
#include <qusvgcomponentloader.h>

class QuStorageRingView_P {
public:
    QuStorageRingView_P() : last_x(-1000000), last_y(-1000000), x_offset(0), y_offset(0), scale_f(60.0) {}
    QString msg;
    double last_x, last_y;
    double x_offset;
    double y_offset;
    double scale_f;
    QMap<QString, QuSvgComponentLoader *> compmap;
    QMap<QString, QSvgRenderer *> rendermap;
};

QuStorageRingView::QuStorageRingView(QWidget *parent,
                                     CumbiaPool *cu_pool,
                                     const CuControlsFactoryPool &fp)
    : QGraphicsView(parent), d(new QuStorageRingView_P) {
    QGraphicsScene *scene = new QGraphicsScene(this);
    //    scene->setSceneRect(-10000,-10000, 20000, 20000);
    setScene(scene);
}

/*! \brief load the storage ring scene from a json document
 *
 *  \param jsonf the JSON source file name. If empty (default), use the
 *         library *elettra* lattice file under *lattice/elettra-lattice.json*
 *  \return true if loading is successful, false otherwise
 */
bool QuStorageRingView::load(const QString &jsonf) {
    d->x_offset = 0;
    d->y_offset = 0;
    double max_x = -1e9, max_y = -1e9;
    QDomDocument doc;
    QDomElement svg = doc.createElement("svg");
    svg.setAttribute("id", jsonf);
    doc.appendChild(svg);
    int linecnt = 0;
    QFile jf(!jsonf.isEmpty() ? jsonf : ":lattice/elettra_lattice.json");
    if(!jf.open(QIODevice::ReadOnly|QIODevice::Text)) {
        d->msg = jf.errorString();
        perr("%s: %s", __PRETTY_FUNCTION__, qstoc(d->msg));
        return false;
    }
    QJsonParseError pe;
    QJsonDocument jd = QJsonDocument::fromJson(jf.readAll(), &pe);
    if(pe.error != QJsonParseError::NoError) {
        d->msg = pe.errorString();
        perr("%s: %s", __PRETTY_FUNCTION__, qstoc(d->msg));
        return false;
    }
    const QStringList& sections { "preinjector", "booster", "bts", "sr"};
    QJsonObject root = jd.object();
    foreach(const QString& s, sections) {
        const QJsonValue& v = root.value(s);
        if(v.isArray()) {
            const QJsonArray& a = v.toArray();
            for(int i = 0; i < a.size(); i++) {
                const QJsonObject& ao = a.at(i).toObject();
                const QJsonValue&sv = ao.value("start");
                if(sv.isObject()) {
                    double x = sv.toObject().value("x").toDouble();
                    double y = sv.toObject().value("z").toDouble();
                    if(x < d->x_offset)
                        d->x_offset = x;
                    if(y < d->y_offset)
                        d->y_offset = y;
                    if(x > max_x) max_x = x;
                    if(y > max_y) max_y = y;
                }
            }
        }
    }
    svg.setAttribute("width", (max_x - d->x_offset)/d->scale_f);
    svg.setAttribute("height", (max_y - d->y_offset)/d->scale_f);

    foreach(const QString& s, sections) {
        double x, y;
        d->last_x = d->last_y = -1000000;
        const QJsonValue &v = root.value(s);
        if(v.isArray()) {
            const QJsonArray& a = v.toArray();
            for(int i = 0; i < a.size(); i++) {
                const QJsonObject& ao = a.at(i).toObject();
                bool chamber = ao.contains("chamber");
                const QJsonValue &sv = ao.value("start"); // start object
                if(sv.isObject()) {
                    x  = sv.toObject().value("x").toDouble() - d->x_offset;
                    y  = sv.toObject().value("z").toDouble() - d->y_offset;
                    qDebug() << __PRETTY_FUNCTION__  << s << " x: " << x << "y: " << y;
                    x = x/d->scale_f;
                    y = y/d->scale_f;
                }
                if (y > -1000000 && d->last_y > -1000000 && x > -1000000 && d->last_x > -1000000 /*&& chamber*/) {
                    if(x != d->last_x && y != d->last_y && chamber /*&& linecnt < 5*/) {
                        linecnt++;
                        // QGraphicsLineItem *line = scene()->addLine((x<d->last_x? x: d->last_x)-3, (y<d->last_y? y: d->last_y)-3, abs(x-d->last_x)+6, abs(y-d->last_y)+6);
                        QGraphicsLineItem *line = scene()->addLine(d->last_x, d->last_y, x, y);
                        //                        qDebug() << __PRETTY_FUNCTION__ << " drawLine()" << d->last_x << ", " << d->last_y << ", " << x << ", " << y << " atan: " << 180.0/M_PI*atan2(y-d->last_y, x-d->last_x);
                        svg.appendChild(QuSvgComponentLoader(QString("l_%1").arg(linecnt)).line(d->last_x, d->last_y, x, y));
                    }
                }
                if ((y != d->last_y) && (x != d->last_x))
                    // atan2 2-variable version of tangent. returns from [-pi,pi]
                    m_add_magnet("bending", x, y,
                                 atan2(y-d->last_y, x-d->last_x) + M_PI, svg);
                d->last_x = x;
                d->last_y = y;
            }
        }
    }
    QString fout("elettra_lattice.svg");
    QFile fw(fout);
    qDebug() << __PRETTY_FUNCTION__ << "saving on file" << fout;
    if(fw.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QTextStream out(&fw);
        out << doc.toString();
        fw.close();
    }
    return true;
}

void QuStorageRingView::mousePressEvent(QMouseEvent *e) {
    qDebug() << __PRETTY_FUNCTION__ << mapToScene(e->pos());
}

void QuStorageRingView::wheelEvent(QWheelEvent *e) {
    double dx = e->angleDelta().x();

    qDebug() << __PRETTY_FUNCTION__ << dx;
    scale(dx > 0 ? 1.15 : 1.0/1.15, dx > 0 ? 1.15 : 1.0/1.15);

}

int magcnt = 0;

void QuStorageRingView::m_add_magnet(const QString& type, double x, double y, double rad, QDomElement &svgroot) {
    const double bendingSize = 25;

    QuPAItem *mag = new QuPAItem(bendingSize, bendingSize/2.0);
    QuSvgComponentLoader svgl(":lattice/components/dipole.svg", "dipole_id");
    if(!d->rendermap.contains("dipole"))
        d->rendermap.insert("dipole", new QSvgRenderer(QLatin1String(":lattice/components/dipole.svg"), this));
    const QSize& ds = d->rendermap["dipole"]->defaultSize();
    QDomElement el = svgl.element();
    double scale = bendingSize / ds.width();
    double scaledw = ds.width() * scale;
    double scaledh = ds.height() * scale;
    // top left translated by (half width, half height)
    double xt = x - scaledw/2.0;
    double yt = y - scaledh/2.0;
    double m11 = cos(rad) * scale, m21 = sin(rad) * scale,
        m12 = -m21, m22 = m11;
    double m13 = scaledw/2.0 * (1-cos(rad))+ scaledh/2.0 * sin(rad);
    double m23 = scaledh/2.0  * (1-cos(rad))- scaledw/2.0 * sin(rad);
    // m1..m6 rotation matrix + translation
    // m5 and m6 account for the translation of the rotation point
    // to the center of the rect
    el.setAttribute("transform", QString("matrix(%1,%2,%3,%4,%5,%6)")
                                     .arg(m11).arg(m21).arg(m12).arg(m22)
                                     .arg(xt + m13).arg(yt + m23));
    svgroot.appendChild(svgl.element());
    mag->setSharedRenderer(d->rendermap["dipole"]);
    mag->setTransformOriginPoint(mag->boundingRect().center());
    mag->setPos(x - bendingSize / 2.0, y - bendingSize / 4.0);
    scene()->addItem(mag);
    mag->setRotation(rad * 180.0/M_PI );
    qDebug() << __PRETTY_FUNCTION__ << "dipole default size: " << d->rendermap["dipole"]->defaultSize();
    return;
}

