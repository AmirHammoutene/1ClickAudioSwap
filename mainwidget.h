#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCheckBox>
#include <QPushButton>

namespace Ui {
class MainWidget;
}

// Declared as a QWidget, but will be used as a system tray icon
class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
protected:
    static const QString undefinedStr;
    static const QString AppName;
    QSystemTrayIcon* sticon;
    QMenu* stmenu;
    QAction* swapAction;
    QPushButton* refreshButton;
    QAction* quitAction;
    QCheckBox* startupCheckBox;
    void buildContextMenu();
    bool doLaunchOnStartup;
    QString sourceAname;
    QString sourceBname;
    QString actualDefaultDeviceName;
    void loadSettings();
    void applySettings();
    void saveSettings();
    QMenu* listChangeAudioOutputMenu;
    void buildListChangeAudioOutput();
    QMenu* configureMenu;
    void buildConfigureSwapMenu();
    QMenu* listSourceAMenu;
    QMenu* listSourceBMenu;
    void executeAudioDeviceChange(const QString & targetDeviceName);
public slots:
    void iconUpdate();
    void swapDevice();
    void refreshDevicesLists();
    void swapDeviceFromIconClick(QSystemTrayIcon::ActivationReason reason);
    void quitApp();
    void changeStartupOption(bool checked);
    void onChangeListAudioOutput(QAction* action);
    void onSourceAChoice(QAction* action);
    void onSourceBChoice(QAction* action);
};

#endif // MAINWIDGET_H
