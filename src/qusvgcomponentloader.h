#ifndef QUSVGCOMPONENTLOADER_H
#define QUSVGCOMPONENTLOADER_H

#include <QString>
#include <QDomDocument>

class QuSvgComponentLoader
{
public:
    QuSvgComponentLoader(const QString& fnam = QString());
    QDomElement element();
    QDomElement line(double x1, double y1, double x2, double y2);

    QString error;


private:
    QDomDocument m_domd;
};

#endif // QUSVGCOMPONENTLOADER_H
