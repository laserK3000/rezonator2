#ifndef START_WINDOW_H
#define START_WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QBoxLayout;
class QJsonObject;
class QFileInfo;
QT_END_NAMESPACE

class CustomCssWidget : public QWidget
{
protected:
    // Paint event should overriden to apply styles heets
    void paintEvent(QPaintEvent*) override;
};

//------------------------------------------------------------------------------

class MruStartItem : public CustomCssWidget
{
    Q_OBJECT

public:
    explicit MruStartItem(const QFileInfo& fileInfo);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QString _filePath, _displayFilePath;
    QLabel* _filePathLabel;
};

//------------------------------------------------------------------------------

class StartPanel : public CustomCssWidget
{
    Q_OBJECT

protected:
    explicit StartPanel(const QString& objectName);

    QWidget* makeHeader(const QString& title);
    QWidget* makeButton(const QString& iconPath, const QString& title, const char* slot);

signals:
    void onClose();
};

//------------------------------------------------------------------------------

class MruStartPanel : public StartPanel
{
    Q_OBJECT

public:
    explicit MruStartPanel();

private:
    void makeEmpty();
};

//------------------------------------------------------------------------------

class TipsStartPanel : public StartPanel
{
    Q_OBJECT

public:
    explicit TipsStartPanel();
    ~TipsStartPanel();

private:
    QLabel *_tipText, *_tipPreview;

    void showNextTip();
    void showPrevTip();
    void showTip(const QJsonObject& tip);
};

//------------------------------------------------------------------------------

class ActionsStartPanel : public StartPanel
{
    Q_OBJECT

public:
    explicit ActionsStartPanel();

private slots:
    void makeSchemaSW();
    void makeSchemaRR();
    void makeSchemaSP();
};

//------------------------------------------------------------------------------

class ToolsStartPanel : public StartPanel
{
    Q_OBJECT

public:
    explicit ToolsStartPanel();

signals:
    void onEditStyleSheet();

private slots:
    void showGaussCalculator();
    void editStyleSheet();
};

//------------------------------------------------------------------------------

class StartWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StartWindow(QWidget *parent = nullptr);

private:
    void editStyleSheet();
};

#endif // START_WINDOW_H
