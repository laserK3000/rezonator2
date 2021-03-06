#ifndef COMMONDATA_H
#define COMMONDATA_H

#include "AppSettings.h"

#include <QObject>

namespace Ori {
    class Styler;
    class Translator;
    class MruFileList;
}

class CommonData : public QObject, public IAppSettingsListener
{
    Q_OBJECT

public:
    explicit CommonData(QObject *parent = nullptr);
    ~CommonData() override;

    static CommonData* instance() { return _instance; }

    Ori::Styler* styler() { return _styler; }
    Ori::Translator* translator() { return _translator; }
    Ori::MruFileList* mruList() { return _mruList; }

    void addFileToMruList(const QString& fileName);

    // Implementation of SettingsListener
    void settingsChanged() override;

private:
    static CommonData* _instance;

    Ori::Styler* _styler;
    Ori::Translator* _translator;
    Ori::MruFileList* _mruList;
};

#endif // COMMONDATA_H
