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
#include <QMap>
#include <QSvgRenderer>
#include <QWheelEvent>
#include <qugraphicssvgitem.h>
#include <qupaitem.h>
#include <qusvgcomponentloader.h>

class QuStorageRingView_P {
public:
    QuStorageRingView_P() : last_x(-1000000), last_y(-1000000), x_offset(0), y_offset(0), scale_f(60.0) {}
    QStringList msgs;
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
    QPointF p0;
    QDomDocument doc;
    QDomElement svg = doc.createElement("svg");
    svg.setAttribute("id", jsonf);
    doc.appendChild(svg);
    int linecnt = 0;
    QFile jf(!jsonf.isEmpty() ? jsonf : ":lattice/elettra_lattice.json");
    if(!jf.open(QIODevice::ReadOnly|QIODevice::Text)) {
        d->msgs += jf.errorString();
        perr("%s: %s", __PRETTY_FUNCTION__, qstoc(d->msgs.last()));
        return false;
    }
    QJsonParseError pe;
    QJsonDocument jd = QJsonDocument::fromJson(jf.readAll(), &pe);
    jf.close();
    if(pe.error != QJsonParseError::NoError) {
        d->msgs += pe.errorString();
        perr("%s: %s", __PRETTY_FUNCTION__, qstoc(d->msgs.last()));
        return false;
    }

    const QJsonObject &root = jd.object();
    if(!m_get_bounds(&max_x, &max_y, &d->x_offset, &d->y_offset, root))
        return false;
    // document width and height according
    max_x *= 1.05;
    max_y *= 1.05; // a little margin
    d->x_offset *= 1.05; d->y_offset *= 1.05;
    svg.setAttribute("width", (max_x - d->x_offset)/d->scale_f);
    svg.setAttribute("height", (max_y - d->y_offset)/d->scale_f);
    scene()->setSceneRect(0, 0, svg.attribute("width").toDouble(), svg.attribute("height").toDouble());
    scene()->addRect(scene()->sceneRect());

    QJsonValue components_v;
    // top level objects represent machine divisions (preinj, booster, sr)
    foreach (const QString& div_name, root.keys()) {
        d->last_x = d->last_y = -1000000;
        double x = -d->last_x, y = -d->last_y, xs, ys, last_xs, last_ys; // absolute x and y, scaled xs and ys
        const QJsonValue& div_v = root.value(div_name);
        if(!div_v.isObject()) {
            d->msgs += div_name + " is not a JSon object";
            return false;
        }
        QJsonObject div_o = div_v.toObject();
        if(div_o.contains("sections") && div_o.value("sections").isArray()) {
            p0 = QPointF();
            QJsonArray sections = div_o.value("sections").toArray();
            for(int i = 0; i < sections.size(); i++) {
                const QJsonValue &sec = sections[i];
                const QJsonObject& sec_o = sec.toObject();
                bool chamber = sec_o.contains("chamber");
                m_get_coords(sec_o.value("start"), &x, &y, &xs, &ys, &last_xs, &last_ys); // start object
                if(i == 0 && chamber)
                    p0 = QPointF(x, y);
                if (d->last_x > -1000000 && chamber) {
                    qDebug() << __PRETTY_FUNCTION__ << "adding line " << last_xs <<  last_ys <<  xs << ys;
                    QGraphicsLineItem *line = scene()->addLine(last_xs, last_ys, xs, ys);
                    line->setPen(QPen(Qt::blue));
                    svg.appendChild(QuSvgComponentLoader(QString("l_%1").arg(linecnt)).line(last_xs, last_ys, xs, ys));
                }
                if (d->last_x > -1000000 && y != d->last_y && x != d->last_x && chamber) {
                    QMap<QString, QString> props;
                    const QJsonValue bending = sec_o.value("bending");
                    const QJsonObject bo = bending.toObject();
                    foreach(const QString& bok, bo.keys())
                        props[bok] = bo[bok].toString();
                    // atan2 2-variable version of tangent. returns from [-pi,pi]
                    double arad = atan2(y - d->last_y, x - d->last_x) + M_PI;
                    // dipole placed at beginning of this section (last x (scaled), last y (scaled) )
                    m_add_component("dipole", QPointF(last_xs, last_ys), arad, svg, props);
                    // subcomponents follow, from end of last section (last_x, last_y) to end of this section (x,y)
                    m_add_subcomponents(components_v, QPointF(d->last_x, d->last_y), QPointF(x, y), props, svg);
                }
                if(sec_o.contains("components")) {
                    components_v = sec_o.value("components");
                }
                d->last_x = x;
                d->last_y = y;
            }


        } // end of sections for a given div
        // shall we close the path?
        if(!p0.isNull()) {
            QMap<QString, QString> props;
            qDebug() << __PRETTY_FUNCTION__ << "closing line: p0 is" << p0 << "last_xs, last_ys" << last_xs << last_ys;
            // line goes from p1 (last section drawn) to p2 (first section drawn)
            // dipole placed in p1 (beginning of section)
            QPointF p1 = m_transform(QPointF(d->last_x, d->last_y)), p2 = m_transform(p0);
            QGraphicsLineItem *line = scene()->addLine(p1.x(), p1.y(), p2.x(), p2.y());
            line->setPen(QPen(Qt::green));
            m_add_component("dipole", QPointF(p1.x(), p1.y()), atan2(p2.y() - p1.y(), p2.x() - p1.x()) + M_PI, svg, props);
            if(components_v.isArray())
                m_add_subcomponents(components_v, QPointF(d->last_x, d->last_y), p0, props, svg);
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

bool QuStorageRingView::error() const {
    return d->msgs.size() > 0;
}

QStringList QuStorageRingView::msgs() const {
    return d->msgs;
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

void QuStorageRingView::m_add_component(const QString& type,
                                        const QPointF& pt,
                                        double rad,
                                        QDomElement &svgroot,
                                        const QMap<QString, QString> &props) {
    const double bendingSize = 25;
    const double x = pt.x(), y = pt.y();
    QuPAItem *mag = new QuPAItem(bendingSize, bendingSize/2.0);
    QuSvgComponentLoader svgl(":lattice/components/" + type + ".svg", type + "_id");
    if(!d->rendermap.contains(type))
        d->rendermap.insert(type, new QSvgRenderer(":lattice/components/" + type + ".svg", this));
    const QSize& ds = d->rendermap[type]->defaultSize();
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
    QString tooltip;
    foreach(const QString& pnam, props.keys()) {
        el.setAttribute(pnam, props[pnam]);
        tooltip += pnam + " -> " + props[pnam];
        mag->setToolTip(tooltip);
    }

    svgroot.appendChild(svgl.element());
    mag->setSharedRenderer(d->rendermap[type]);
    mag->setTransformOriginPoint(mag->boundingRect().center());
    mag->setPos(x - bendingSize / 2.0, y - bendingSize / 4.0);
    scene()->addItem(mag);
    mag->setRotation(rad * 180.0/M_PI );
//    qDebug() << __PRETTY_FUNCTION__ << type << " default size: " << d->rendermap[type]->defaultSize();
    return;
}

bool QuStorageRingView::m_get_bounds(double *max_x, double *max_y, double *x_offset, double *y_offset, const QJsonObject &root) const
{
    foreach (const QString& div_name, root.keys()) {
        const QJsonValue& div_v = root.value(div_name);
        if(!div_v.isObject()) {
            d->msgs += div_name + " is not a JSon object";
            return false;
        }
        QJsonObject div_o = div_v.toObject();
        if(div_o.contains("sections") && div_o.value("sections").isArray()) {
            QJsonArray sections = div_o.value("sections").toArray();
            for(int i = 0; i < sections.size(); i++) {
                const QJsonValue &sec = sections[i];
                // iterate through "sections" array
                if(sec.isObject()) {
                    const QJsonObject& so = sec.toObject();
                    const QJsonValue& sv = so.value("start");
                    if(sv.isObject()) {
                        double x = sv.toObject().value("x").toDouble();
                        double y = sv.toObject().value("z").toDouble();
                        if(x < *x_offset)
                            *x_offset = x;
                        if(y < *y_offset)
                            *y_offset = y;
                        if(x > *max_x) *max_x = x;
                        if(y > *max_y) *max_y = y;
                    }
                }
                else
                    d->msgs.append(QString("section at index \"%1\" is not an object").arg(i));
            }
        }
    }
    return true;
}

bool QuStorageRingView::m_get_coords(const QJsonValue &start_v, double *x, double *y, double *xs, double *ys, double *last_xs, double *last_ys) const
{
    *x = 0, *y = 0, *xs = 0, *ys = 0, *last_xs = 0, *last_ys = 0;
    if(start_v.isObject()) {
        *x  = start_v.toObject().value("x").toDouble();
        *y  = start_v.toObject().value("z").toDouble();
        const QPointF& p1 = m_transform(QPointF(*x, *y));
        const QPointF& p0 = m_transform(QPointF(d->last_x, d->last_y));
        *xs = p1.x();
        *ys = p1.y();
        *last_xs = p0.x();
        *last_ys = p0.y();
    }
    return true;
}

bool QuStorageRingView::m_add_subcomponents(const QJsonValue& components_v,
                                            const QPointF& p0,
                                            const QPointF& p1,
                                            QMap<QString, QString> &props,
                                            QDomElement &svg) {
    if(components_v.isArray()) {
        double arad, xs, ys;
        QJsonArray component_a = components_v.toArray();
        foreach(const QJsonValue& comp_v, component_a) {
            if(comp_v.isObject()) {
                props.clear();
                const QJsonObject &comp_o = comp_v.toObject();
                const QString& type = comp_o["type"].toString(QString());
                const double &pos = comp_o["position"].toDouble(-1);
                foreach(const QString& mak, comp_o.keys())
                    props[mak] = comp_o[mak].toString();
                const QString& name = comp_o["name"].toString(QString());
                arad = atan2(p1.y() - p0.y(), p1.x() - p0.x());
                QPointF scaled_p = m_transform(QPointF(p0.x() + pos * cos(arad), p0.y() + pos * sin(arad) ));
                if(type.length() > 0 && pos > -1)
                    m_add_component(type, scaled_p, arad, svg, props);
            }
        }
    }
    return components_v.isArray();
}

QPointF QuStorageRingView::m_transform(const QPointF &pabs) const {
    return QPointF((pabs.x() - d->x_offset)/d->scale_f, (pabs.y() - d->y_offset)/d->scale_f);
}

