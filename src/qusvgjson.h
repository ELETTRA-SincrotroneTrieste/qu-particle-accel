#ifndef QUSVGJSON_H
#define QUSVGJSON_H


class QuSvgJson
{
public:
    QuSvgJson();
    bool readJsonFile(const QString &file_path, QJsonDocument &json_doc);
private:
    void m_add_magnet(QWidget *pw, QString type, double x, double y, double deg);
};

#endif // QUSVGJSON_H
