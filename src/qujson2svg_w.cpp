#include "qujson2svg_w.h"
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
#include <QGraphicsView>
#include <QDomDocument>
#include <QMap>
#include <QSet>
#include <QSvgRenderer>
#include <QWheelEvent>
#include <qugraphicssvgitem.h>
#include <qupaitem.h>
#include <qusvgcomponentloader.h>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <qgridlayout.h>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QSettings>
#include <QMouseEvent>

MyGraphicsView::MyGraphicsView(QWidget *parent) : QGraphicsView(parent) {
    setMouseTracking(true);
}

void MyGraphicsView::mousePressEvent(QMouseEvent *e) {
    qDebug() << __PRETTY_FUNCTION__ << mapToScene(e->pos());
    QGraphicsView::mousePressEvent(e);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *e) {
    qDebug() << __PRETTY_FUNCTION__ << itemAt(e->pos());
}

void MyGraphicsView::wheelEvent(QWheelEvent *e) {
    int dy = e->angleDelta().y();
    scale(dy > 0 ? 1.15 : 1.0/1.15, dy > 0 ? 1.15 : 1.0/1.15);
}

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
    QSet<QString> ids;
    QDomDocument doc;
};

QuJsonToSvgW::QuJsonToSvgW(QWidget *parent)
    : QWidget(parent), d(new QuStorageRingView_P) {
    QGridLayout *lo = new QGridLayout(this);
    QGraphicsView *gv = new MyGraphicsView(this);
    gv->setRenderHint(QPainter::Antialiasing, true);
    QGraphicsScene *scene = new QGraphicsScene(this);
    gv->setScene(scene);
    QLabel *scaleLabel = new QLabel("Objects scale factor", this);
    QLabel *inLabel = new QLabel("JSON in", this);
    QLabel *outLabel = new QLabel("SVG out", this);
    QPushButton *pbApplyScale = new QPushButton("Apply", this);
    QPushButton *pbSaveSVG = new QPushButton("Save SVG", this);
    pbSaveSVG->setObjectName("save");
    QPushButton *pbSetOutFnam = new QPushButton("Set Out file", this);
    pbSetOutFnam->setObjectName("pbSetOutFnam");
    QPushButton *pbSetJsonIn = new QPushButton("Set JSON input file", this);
    QPushButton *pbLoad = new QPushButton("Load", this);
    pbLoad->setObjectName("load");
    QDoubleSpinBox *sbScale = new QDoubleSpinBox(this);
    sbScale->setObjectName("sbScale");
    QLineEdit *finLe = new QLineEdit(this);
    QLineEdit *foutLe = new QLineEdit(this);
    finLe->setReadOnly(true);
    foutLe->setReadOnly(true);
    foutLe->setObjectName("fout");
    finLe->setObjectName("fin");
    finLe->setPlaceholderText("Click on Load JSON...");
    foutLe->setPlaceholderText("Click on convert...");
    sbScale->setMinimum(1);
    sbScale->setMaximum(100);
    sbScale->setValue(25.0);
    connect(pbApplyScale, SIGNAL(clicked()), this, SLOT(rescale()));
    connect(pbSaveSVG, SIGNAL(clicked()), this, SLOT(save_svg()));
    connect(pbSetJsonIn, SIGNAL(clicked()), this, SLOT(open_json()));
    connect(pbSetOutFnam, SIGNAL(clicked()), this, SLOT(set_out_file()));
    connect(pbLoad, SIGNAL(clicked()), this, SLOT(load()));
    lo->addWidget(gv, 0, 0, 10, 10);
    lo->addWidget(scaleLabel, 10, 0, 1,1);
    lo->addWidget(sbScale, 10, 1, 1, 1);
    lo->addWidget(pbApplyScale, 10, 2, 1, 1);
    lo->addWidget(inLabel, 10, 3, 1, 1);
    lo->addWidget(pbSetJsonIn, 10, 4, 1, 1);
    lo->addWidget(finLe, 10, 5, 1, 4);
    lo->addWidget(pbLoad, 10, 9, 1, 1);
    lo->addWidget(outLabel, 11, 0, 1, 1);
    lo->addWidget(foutLe, 11, 1, 1, 7);
    lo->addWidget(pbSetOutFnam, 11, 8, 1, 1);
    lo->addWidget(pbSaveSVG, 11, 9, 1, 1);
    foreach(QLabel *l, findChildren<QLabel *>())
        l->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    QSettings s;
    finLe->setText(s.value("fin").toString());
    foutLe->setText(s.value("fout").toString());
    pbSaveSVG->setEnabled(foutLe->text().length() > 0);
    pbLoad->setEnabled(finLe->text().length() > 0);
}

/*! \brief load the storage ring scene from a json document
 *
 *  \param jsonf the JSON source file name. If empty (default), use the
 *         library *elettra* lattice file under *lattice/elettra-lattice.json*
 *  \return true if loading is successful, false otherwise
 */
bool QuJsonToSvgW::m_load() {
    QString jsonf = findChild<QLineEdit *>("fin")->text();
    d->msgs.clear();
    d->x_offset = 0;
    d->y_offset = 0;
    double max_x = -1e9, max_y = -1e9;
    QPointF p0;
    QJsonObject bending_o;
    d->doc.clear();
    QDomElement svg = d->doc.createElement("svg");
    m_set_id(jsonf, svg);
    d->doc.appendChild(svg);
    int linecnt = 0;
    QFile jf(jsonf);
    if(!jf.open(QIODevice::ReadOnly|QIODevice::Text)) {
        d->msgs += jf.errorString();
        perr("%s: %s", __PRETTY_FUNCTION__, qstoc(d->msgs.last()));
        emit op(d->msgs);
        return false;
    }
    QJsonParseError pe;
    QJsonDocument jd = QJsonDocument::fromJson(jf.readAll(), &pe);
    jf.close();
    if(pe.error != QJsonParseError::NoError) {
        d->msgs += pe.errorString();
        emit op(d->msgs);
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
        QDomElement div_e = d->doc.createElement("g");
        m_set_id(div_name, div_e);
        svg.appendChild(div_e);
        d->last_x = d->last_y = -1000000;
        double x = -d->last_x, y = -d->last_y, xs, ys, last_xs, last_ys; // absolute x and y, scaled xs and ys
        const QJsonValue& div_v = root.value(div_name);
        if(!div_v.isObject()) {
            d->msgs += div_name + " is not a JSon object";
        }
        else {
            int i = 0;
            QJsonObject div_o = div_v.toObject();
            if(div_o.contains("sections") && div_o.value("sections").isArray()) {
                bending_o = QJsonObject();
                p0 = QPointF();
                QJsonArray sections = div_o.value("sections").toArray();
                for(i = 0; i < sections.size(); i++) {
                    QDomElement sectionE = d->doc.createElement("g");
                    m_set_id( QString("%1_section_%2").arg(div_name).arg(i), sectionE);
                    div_e.appendChild(sectionE);
                    const QJsonValue &sec = sections[i];
                    const QJsonObject& sec_o = sec.toObject();
                    bool chamber = sec_o.contains("chamber");
                    m_get_coords(sec_o.value("start"), &x, &y, &xs, &ys, &last_xs, &last_ys); // start object
                    if(i == 0 && chamber)
                        p0 = QPointF(x, y);                        
                    if (d->last_x > -1000000 && chamber) {
                        QGraphicsLineItem *line = scene()->addLine(last_xs, last_ys, xs, ys);
                        line->setPen(QPen(Qt::blue));
                        sectionE.appendChild(QuSvgComponentLoader(QString("l_%1").arg(linecnt)).line(last_xs, last_ys, xs, ys));
                    }
                    if (d->last_x > -1000000 && y != d->last_y && x != d->last_x && chamber) {
                        QMap<QString, QString> props;
                        // bending_o in last_x, last_y from previous section
                        if(!bending_o.isEmpty()) {
                            foreach(const QString& bok, bending_o.keys())
                                props[bok] = bending_o[bok].toString();
                            // atan2 2-variable version of tangent. returns from [-pi,pi]
                            double arad = atan2(y - d->last_y, x - d->last_x) + M_PI;
                            // dipole placed at beginning of this section (last x (scaled), last y (scaled) )
                            const QString& id = bending_o.value("name").toString();
                            m_add_component("bending", id, QPointF(last_xs, last_ys), arad, sectionE, props);
                        }
                        // subcomponents follow, from end of last section (last_x, last_y) to end of this section (x,y)
                        m_add_subcomponents(components_v, QPointF(d->last_x, d->last_y), QPointF(x, y), props, sectionE);
                    }
                    if(sec_o.contains("components")) {
                        components_v = sec_o.value("components");
                    }
                    if(sec_o.contains("bending") && sec_o.value("bending").isObject()) {
                        bending_o = sec_o.value("bending").toObject();
                    }
                    d->last_x = x;
                    d->last_y = y;
                }
            } // end of sections for a given div
            // shall we close the path?
            if(!p0.isNull()) {
                QDomElement sectionE = d->doc.createElement("g");
                m_set_id( QString("section_%1").arg(i), sectionE);
                div_e.appendChild(sectionE);
                QMap<QString, QString> props;
                // line goes from p1 (last section drawn) to p2 (first section drawn)
                // dipole placed in p1 (beginning of section)
                QPointF p1 = m_transform(QPointF(d->last_x, d->last_y)), p2 = m_transform(p0);
                QGraphicsLineItem *line = scene()->addLine(p1.x(), p1.y(), p2.x(), p2.y());
                line->setPen(QPen(Qt::green));
                sectionE.appendChild(QuSvgComponentLoader(QString("l_%1").arg(++linecnt)).line(p1.x(), p1.y(), p2.x(), p2.y()));

                if(!bending_o.isEmpty()) {
                    m_add_component("bending", bending_o.value("name").toString(), QPointF(p1.x(), p1.y()), atan2(p2.y() - p1.y(), p2.x() - p1.x()) + M_PI, sectionE, m_map_props(bending_o));
                }
                if(components_v.isArray())
                    m_add_subcomponents(components_v, QPointF(d->last_x, d->last_y), p0, props, sectionE);
            }
        }
    }
    emit op(d->msgs);
    QSettings s; // save json input file name
    s.setValue("fin", jsonf);
    return true;
}

bool QuJsonToSvgW::error() const {
    return d->msgs.size() > 0;
}

QStringList QuJsonToSvgW::msgs() const {
    return d->msgs;
}

QGraphicsScene *QuJsonToSvgW::scene() const {
    return findChild<QGraphicsScene *>();
}

void QuJsonToSvgW::m_add_component(const QString& type,
                                   const QString& id,
                                   const QPointF& pt,
                                   double rad,
                                   QDomElement &svgroot,
                                   const QMap<QString, QString> &props) {
    const double bendingSize = findChild<QDoubleSpinBox *>("sbScale")->value();
    const double x = pt.x(), y = pt.y();
    QuPAItem *mag = new QuPAItem(bendingSize, bendingSize/2.0);
    QuSvgComponentLoader svgl(":lattice/components/" + type + ".svg", id);
    if(svgl.error.isEmpty()) {
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
            tooltip += pnam + " -> " + props[pnam] + "\n";
            mag->setToolTip(tooltip);
        }

        svgroot.appendChild(svgl.element());
        mag->setSharedRenderer(d->rendermap[type]);
        mag->setTransformOriginPoint(mag->boundingRect().center());
        mag->setPos(x - bendingSize / 2.0, y - bendingSize / 4.0);
        scene()->addItem(mag);
        mag->setRotation(rad * 180.0/M_PI );
        //    qDebug() << __PRETTY_FUNCTION__ << type << " default size: " << d->rendermap[type]->defaultSize();
    }
    return;
}

bool QuJsonToSvgW::m_get_bounds(double *max_x, double *max_y, double *x_offset, double *y_offset, const QJsonObject &root) const
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

bool QuJsonToSvgW::m_get_coords(const QJsonValue &start_v, double *x, double *y, double *xs, double *ys, double *last_xs, double *last_ys) const
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

bool QuJsonToSvgW::m_add_subcomponents(const QJsonValue& components_v,
                                       const QPointF& p0,
                                       const QPointF& p1,
                                       QMap<QString, QString> &props,
                                       QDomElement &svg) {
    if(components_v.isArray()) {
        double arad;
        QJsonArray component_a = components_v.toArray();
        foreach(const QJsonValue& comp_v, component_a) {
            if(comp_v.isObject()) {
                props.clear();
                const QJsonObject &comp_o = comp_v.toObject();
                const QString& type = comp_o["type"].toString(QString());
                const double &pos = comp_o["position"].toDouble(-1);
                const QString& name = comp_o["name"].toString(QString());
                arad = atan2(p1.y() - p0.y(), p1.x() - p0.x());
                QPointF scaled_p = m_transform(QPointF(p0.x() + pos * cos(arad), p0.y() + pos * sin(arad) ));
                if(type.length() > 0 && pos > -1)
                    m_add_component(type, name, scaled_p, arad, svg, m_map_props(comp_o));
            }
        }
    }
    return components_v.isArray();
}

QMap<QString, QString> QuJsonToSvgW::m_map_props(const QJsonObject &jo) const {
    QMap<QString, QString> pmap;
    foreach(const QString& mak, jo.keys()) {
        const QJsonValue &v = jo[mak];
        if(v.isDouble())
            pmap[mak] = QString::number(v.toDouble());
        else if(v.isBool())
            pmap[mak] = v.toBool() ? "true" : "false";
        else if(v.isString())
            pmap[mak] = v.toString();
        else if (v.isArray())
            pmap[mak] = "array: " + v.toString();
        else
            pmap[mak] = jo[mak].toString();
    }
    return pmap;
}

QPointF QuJsonToSvgW::m_transform(const QPointF &pabs) const {
    return QPointF((pabs.x() - d->x_offset)/d->scale_f, (pabs.y() - d->y_offset)/d->scale_f);
}

void QuJsonToSvgW::m_set_id(const QString &id, QDomElement &e) const {
    if(this->d->ids.contains(id)) {
        d->msgs.append(QString("duplicated id '%1'").arg(id));
    }
    e.setAttribute("id", id);
}

void QuJsonToSvgW::load() {
    const QString& jsonf = findChild<QLineEdit *>("fin")->text();
    if(jsonf.isEmpty())
        this->open_json();
    if(!jsonf.isEmpty())
        m_load();
}

void QuJsonToSvgW::set_out_file() {
    QSettings s;
    QString fin = findChild<QLineEdit *>("fin")->text();
    fin = (fin != findChild<QLineEdit *>("fin")->placeholderText()) ? fin.section('/', -1).remove(".json") : "out";
    const QString& fn = s.value("outfnam", QDir::currentPath() + "/" + fin + ".svg").toString();
    QString fnam = QFileDialog::getOpenFileName(this, "Save SVG", fn);
    if(!fnam.isEmpty())
        findChild<QLineEdit *>("fout")->setText(fnam);
    findChild<QPushButton *>("save")->setEnabled(!findChild<QLineEdit *>("fout")->text().isEmpty());
}

void QuJsonToSvgW::rescale() {
    QGraphicsScene *scene = findChild<QGraphicsScene *>();
    scene->clear();
    this->m_load();
}

void QuJsonToSvgW::save_svg() {
    QSettings s;
    const QString& fnam = findChild<QLineEdit *>("fout")->text();
    if(fnam.length() > 0) {
        QString fout(fnam);
        QFile fw(fout);
        if(fw.open(QIODevice::WriteOnly|QIODevice::Text)) {
            QTextStream out(&fw);
            out << d->doc.toString();
            fw.close();
            s.setValue("fout", fnam);
        }
        else
            d->msgs << "error opening file " + fnam + " for writing: " + fw.errorString();
    }
    emit op(d->msgs);
}

void QuJsonToSvgW::open_json() {
    QSettings s;
    QString inf = s.value("fin", QDir::currentPath() + "in.json").toString();
    inf = QFileDialog::getOpenFileName(this, "Open JSON input file", inf, "*.json");
    QLineEdit *inle = findChild<QLineEdit *>("fin");
    if(!inf.isEmpty())
        inle->setText(inf);
    findChild<QPushButton *>("load")->setEnabled(!inle->text().isEmpty());
}
