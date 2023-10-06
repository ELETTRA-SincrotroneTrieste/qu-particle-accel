#ifndef QUSVGCOMPONENTLOADER_H
#define QUSVGCOMPONENTLOADER_H

#include <QString>
#include <QDomDocument>

class QuSvgComponentLoader
{
public:
    QuSvgComponentLoader(const QString& fnam, const QString &id);
    QuSvgComponentLoader(const QString &id);
    QDomElement element();
    QDomElement line(double x1, double y1, double x2, double y2);

    QString error;


private:
    QDomDocument m_domd;
    QString m_id;
};

#endif // QUSVGCOMPONENTLOADER_H
