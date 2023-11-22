#include "qusvgcomponentloader.h"
#include <QFile>
#include <QtDebug>

QuSvgComponentLoader::QuSvgComponentLoader(const QString &fnam) {
    if(fnam.length() > 0) {
        m_domd = QDomDocument(fnam);
        QFile file(fnam);
        if (file.open(QIODevice::ReadOnly)) {
            m_domd.setContent(&file, &error);
            file.close();
        }
        else {
            error = file.errorString();
        }
    }
}

QDomElement QuSvgComponentLoader::element()  {
    QDomDocument doc;
    QDomElement firstChild = m_domd.documentElement();
    firstChild.setTagName("g");
    firstChild.setAttribute("item", "true");
    firstChild.setAttribute("clickable", "true");

    firstChild.removeAttribute("xmlns");
    firstChild.removeAttribute("xmlns:svg");
    firstChild.removeAttribute("version");
    firstChild.removeAttribute("width");
    firstChild.removeAttribute("height");
    //    QDomNodeList children = firstChild.childNodes();
    //    qDebug() << __PRETTY_FUNCTION__ << "children siz" << children.size() << "first child" << firstChild.tagName();
    //    QDomElement el = doc.createElement("g");
    //    el.setAttribute("id", m_id);
    //    QDomNode child0;
    //    if(children.size() > 0)
    //        child0 = el.appendChild(children.at(0));
    //    for(int i = 1; child0.isElement() && i < children.size(); i++) {
    //        qDebug() << i << "+" << children.at(i).toElement().tagName();
    //        QDomNode node = el.insertAfter(children.at(i), child0);
    //        if(node.isNull())
    //            qDebug() << __PRETTY_FUNCTION__ << "Failed to add child!";
    //    }
    return firstChild;
}

QDomElement QuSvgComponentLoader::line(double x1, double y1, double x2, double y2)  {
    QDomDocument doc("dummydoc");
    QDomElement line = doc.createElement("line");
    line.setAttribute("x1", x1);
    line.setAttribute("x2", x2);
    line.setAttribute("y1", y1);
    line.setAttribute("y2", y2);
    line.setAttribute("z", -100);
    line.setAttribute("stroke", "blue");
    line.setAttribute("item", "true"); // for cumbia-svg
    return line;
}
