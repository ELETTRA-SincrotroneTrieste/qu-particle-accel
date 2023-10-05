#include "qusvgjson.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QtDebug>

QuSvgJson::QuSvgJson()
{

}


bool QuSvgJson::readJsonFile(const QString& file_path, QJsonDocument& json_doc) {
    QJsonObject json_obj = json_doc.object();
    double x = 0;
    double y = 0;
    for (unsigned i=0; i<json_obj.keys().size(); i++) {
        if (json_obj.keys()[i]=="envelope") continue;
        qDebug() << __PRETTY_FUNCTION__ << "Key" << i << ": " << json_obj.keys()[i].toStdString() << endl;
        QJsonValue jval = json_obj.value(json_obj.keys()[i]);
        QJsonArray jarr = jval.toArray();
        lastX = x = lastY = y = -1000000;
        foreach (const QJsonValue & value, jarr) {
            QJsonObject obj = value.toObject();
            bool chamber = false;
            for (unsigned j=0; j<obj.keys().size(); j++) {
                cout << j << " " << obj.keys()[j].toStdString();
                if (obj.keys()[j]=="chamber") {
                    chamber = true;
                }
                if (obj.keys()[j]=="bending") {
                    QJsonValue bval = obj.value(obj.keys()[j]);
                    QJsonObject bobj = bval.toObject();
                    for (unsigned k=0; k<bobj.keys().size(); k++) {
                        QJsonValue bname = obj.value(bobj.keys()[k]);
                        if (bobj.keys()[k]=="name")
                            qDebug() << __PRETTY_FUNCTION__ << " - " << bobj.value(bobj.keys()[k]).toString().toStdString();
                    }
                }
                if (obj.keys()[j]=="start") {
                    QJsonValue bval = obj.value(obj.keys()[j]);
                    QJsonObject bobj = bval.toObject();
                    for (unsigned k=0; k<bobj.keys().size(); k++) {
                        QJsonValue bname = obj.value(bobj.keys()[k]);
                        if (bobj.keys()[k]=="x") {
                            x = bobj.value(bobj.keys()[k]).toDouble()/scaleFactor+xOffset;
                            qDebug() << __PRETTY_FUNCTION__  << " x: " << x;
                        }
                        if (bobj.keys()[k]=="z") {
                            qDebug() << __PRETTY_FUNCTION__  << " y: " << bobj.value(bobj.keys()[k]).toDouble();
                            y = bobj.value(bobj.keys()[k]).toDouble()/scaleFactor+yOffset;
                        }
                    }
                }
                if ((y > -1000000) && (lastY > -1000000) && (x > -1000000) && (lastX > -1000000) && chamber) {
                    EArrow* line = new EArrow(this);
                    line->setGeometry((x<lastX? x: lastX)-3, (y<lastY? y: lastY)-3, abs(x-lastX)+6, abs(y-lastY)+6);
                    line->setArrowSize(0);
                    line->setArrowDirection(x<lastX != y<lastY? EArrow::RIGTHTOP: EArrow::RIGTHBOTTOM);
                    cout << " drawLine()" << lastX << ", " << lastY << ", " << x << ", " << y << " atan: " << 180.0/M_PI*atan2(y-lastY, x-lastX);
                }
                if ((y != lastY) && (x != lastX))
                    addMagnet(this, "bending", x, y, 180.0/M_PI*atan2(y-lastY, x-lastX)+180);
                lastX = x;
                lastY = y;
                cout << endl;
            }
        }
    }
