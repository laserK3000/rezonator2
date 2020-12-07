#ifndef SCHEMA_READER_JSON_H
#define SCHEMA_READER_JSON_H

#include "../core/Report.h"
#include <QtWidgets>

class Element;
class Schema;
class PumpParams;

class SchemaReaderJson
{
public:
    SchemaReaderJson(Schema *schema) : _schema(schema) {}

    void readFromFile(const QString& fileName);
    void readFromString(const QString& text);
    void readFromUtf8(const QByteArray& data);

    const Z::Report& report() const { return _report; }

private:
    Schema *_schema;
    Z::Report _report;

    void readGeneral(const QJsonObject& root);
    void readCustomParams(const QJsonObject& root);
    void readCustomParam(const QJsonObject& root, const QString& alias);
    void readPumps(const QJsonObject& root);
    void readElements(const QJsonObject& root);
    void readParamLinks(const QJsonObject& root);
    void readParamLink(const QJsonObject& root);
    void readFormulas(const QJsonObject& root);
    void readFormula(const QJsonObject& root);
    void readWindows(const QJsonObject& root);
    void readWindow(const QJsonObject& root);
};

namespace Z {
namespace IO {
namespace Json {

QList<Element*> readElements(const QJsonObject& root, Z::Report* report);
Element* readElement(const QJsonObject& root, Z::Report* report);

QList<PumpParams*> readPumps(const QJsonObject& root, Z::Report* report);
PumpParams* readPump(const QJsonObject& root, Z::Report* report);

} // namespace Json
} // namespace IO
} // namespace Z

#endif // SCHEMA_READER_JSON_H
