#include "mainwidget.h"
#include <QApplication>
#include <QWidgetAction>
#include <QSettings>
#include <QStandardPaths>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QProcess>
#include <QDir>

const QString MainWidget::undefinedStr = "undefined";
const QString MainWidget::AppName = "1ClickAudioSwap";

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent)
{
    // Will be a system tray app, no window shown
    sticon = new QSystemTrayIcon(this);
    sticon->setIcon(QIcon(":/img/icon.png"));

    buildContextMenu();
    sticon->setContextMenu(stmenu);

    // Loads and applies settings saved (such as "launch on Windows startup" or the devices we used at last run
    loadSettings();
    applySettings();

    sticon->show();

    // Updates the system tray icon state and tooltip
    iconUpdate();

    connect(sticon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(swapDeviceFromIconClick(QSystemTrayIcon::ActivationReason)));

    // Saves settings to be sure the "launch on Windows startup" entry in register is correctly done (otherwise, in case of first launch and crash before quit, it wouldn't)
    saveSettings();
}

MainWidget::~MainWidget()
{
}

//Builds the menu when right click on system tray icon
void MainWidget::buildContextMenu()
{
    stmenu = new QMenu(this);

    QAction* titleAction = new QAction(QIcon(":/img/icon.png"),tr("1 Click Audio Swap"),stmenu);
    titleAction->setEnabled(false);
    QFont titleFont = titleAction->font();
    titleFont.setBold(true);
    titleAction->setFont(titleFont);

    swapAction =  new QAction(tr("Swap (same as a click on the system tray icon)"),stmenu);
    connect(swapAction, SIGNAL(triggered()), this, SLOT(swapDevice()));

    configureMenu = new QMenu(tr("Configure the Swap"),stmenu);
    buildConfigureSwapMenu();

    listChangeAudioOutputMenu = new QMenu(tr("Change default Audio Device"),stmenu);
    buildListChangeAudioOutput();

    QWidgetAction* refreshAction = new QWidgetAction(stmenu);
    refreshButton = new QPushButton(tr("Refresh devices"), stmenu);
    refreshAction->setDefaultWidget(refreshButton);
    connect(refreshButton, SIGNAL(released()), this, SLOT(refreshDevicesLists()));

    QWidgetAction* startupAction = new QWidgetAction(stmenu);
    startupCheckBox = new QCheckBox(tr("Launch on Windows Startup"), stmenu);
    startupAction->setDefaultWidget(startupCheckBox);
    connect(startupCheckBox, SIGNAL(toggled(bool)), this, SLOT(changeStartupOption(bool)));

    quitAction = new QAction(tr("Quit"),stmenu);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApp()));

    stmenu->addAction(titleAction);
    stmenu->addSeparator();
    stmenu->addAction(swapAction);
    stmenu->addMenu(configureMenu);
    stmenu->addMenu(listChangeAudioOutputMenu);
    stmenu->addAction(refreshAction);
    stmenu->addAction(startupAction);
    stmenu->addAction(quitAction);
}

void MainWidget::loadSettings()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::defaultFormat(), QSettings::UserScope,
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QSettings settings;

    doLaunchOnStartup = settings.value("launchOnStartup", true).toBool();
    sourceAname = settings.value("sourceAname", undefinedStr).toString();
    sourceBname = settings.value("sourceBname", undefinedStr).toString();
}

void MainWidget::applySettings()
{
    if(startupCheckBox)
        startupCheckBox->setChecked(doLaunchOnStartup);

    if(sourceAname != undefinedStr)
    {
        for(auto act : listSourceAMenu->actions())
        {
            act->setChecked(false);
            if(act->text() == sourceAname)
                act->setChecked(true);
        }
    }

    if(sourceBname != undefinedStr)
    {
        for(auto act : listSourceBMenu->actions())
        {
            act->setChecked(false);
            if(act->text() == sourceBname)
                act->setChecked(true);
        }
    }
}

void MainWidget::saveSettings()
{
    QSettings settings;
    settings.setValue("launchOnStartup", doLaunchOnStartup);
    settings.setValue("sourceAname", sourceAname);
    settings.setValue("sourceBname", sourceBname);

    // Saves or deletes key in register for launching on Windows startup
    if(doLaunchOnStartup)
    {
        QSettings startSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        startSettings.setValue(AppName, QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
        startSettings.sync();
    }
    else
    {
        QSettings startSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        startSettings.remove(AppName);
    }

}

// Builds sub menu "Configure Swap"
void MainWidget::buildConfigureSwapMenu()
{
    configureMenu->clear();

    listSourceAMenu = new QMenu(tr("First audio source"),configureMenu);
    listSourceAMenu->setIcon(QIcon(":/img/sourceA.png"));
    listSourceBMenu = new QMenu(tr("Second audio source"),configureMenu);
    listSourceBMenu->setIcon(QIcon(":/img/sourceB.png"));

    QAction *undefinedA = new QAction(undefinedStr,listSourceAMenu);
    undefinedA->setCheckable(true);
    QAction *undefinedB = new QAction(undefinedStr,listSourceBMenu);
    undefinedB->setCheckable(true);
    listSourceAMenu->addAction(undefinedA);
    listSourceBMenu->addAction(undefinedB);

    auto devices = QMediaDevices::audioOutputs();

    for (int i = 0; i < devices.size(); ++i)
    {
        auto ad = devices.at(i);

        QAction *actionA = new QAction(listSourceAMenu);
        actionA->setText(ad.description());
        actionA->setCheckable(true);
        listSourceAMenu->addAction(actionA);

        QAction *actionB = new QAction(listSourceBMenu);
        actionB->setText(ad.description());
        actionB->setCheckable(true);
        listSourceBMenu->addAction(actionB);
    }

    undefinedA->setChecked(true);
    undefinedB->setChecked(true);


    configureMenu->addMenu(listSourceAMenu);
    configureMenu->addMenu(listSourceBMenu);

    connect(listSourceAMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSourceAChoice(QAction*)));
    connect(listSourceBMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSourceBChoice(QAction*)));
}

// Builds sub menu to manually change default audio device
void MainWidget::buildListChangeAudioOutput()
{
    listChangeAudioOutputMenu->clear();

    auto devices = QMediaDevices::audioOutputs();
    if(devices.size() == 0)
        return;
    for (int i = 0; i < devices.size(); ++i)
    {
        auto ad = devices.at(i);
        QAction *action = new QAction(listChangeAudioOutputMenu);
        action->setText(ad.description());
        action->setCheckable(true);
        if(ad.isDefault())
        {
            action->setChecked(true);
            actualDefaultDeviceName = ad.description();
        }
        listChangeAudioOutputMenu->addAction(action);
    }

    connect(listChangeAudioOutputMenu, SIGNAL(triggered(QAction*)), this, SLOT(onChangeListAudioOutput(QAction*)));
}

// Uses a executable process for setting the default audio device
void MainWidget::executeAudioDeviceChange(const QString &targetDeviceName)
{
    QProcess *process = new QProcess(this);
    process->start("EndPointController.exe", QStringList() << targetDeviceName);
    process->waitForFinished();
}

// Updates the system tray icon state and tooltip
void MainWidget::iconUpdate()
{
    QString iconName;
    QString tooltip;
    QString separator = " - ";
    QString swapSentence = tr("Click to swap to ");

    if(actualDefaultDeviceName != sourceAname && actualDefaultDeviceName != sourceBname
            && sourceAname != undefinedStr && sourceBname != undefinedStr) // In case the devices for the swap are not configured
    {
        iconName = ":/img/iconUnknownSources.png";
        if(tooltip.isEmpty())
            tooltip = actualDefaultDeviceName;
        else
            tooltip = actualDefaultDeviceName + separator + tr("One of the sources is undefined");
    }
    else
    {
        tooltip = "["+actualDefaultDeviceName+"]";
        if(actualDefaultDeviceName == sourceAname)
        {
            if(sourceBname != undefinedStr)
            {
                iconName = ":/img/iconSourceA.png";
                tooltip += separator + swapSentence + sourceBname;
            }
            else
            {
                iconName = ":/img/iconUnsetSources.png";
                tooltip += separator + tr("The second source is undefined");
            }
        }
        else
        {
            if(sourceAname != undefinedStr)
            {
                iconName = ":/img/iconSourceB.png";
                tooltip += separator + swapSentence + sourceAname;
            }
            else
            {
                iconName = ":/img/iconUnsetSources.png";
                tooltip += separator + tr("The first source is undefined");
            }
        }
    }

    sticon->setIcon(QIcon(iconName));
    sticon->setToolTip(tooltip);
}

// On Swap action
void MainWidget::swapDevice()
{
    if(sourceAname == undefinedStr || sourceBname == undefinedStr)
    {
        sticon->showMessage(tr("Undefined sources"),tr("One of the sources is undefined, please configure those"));
        iconUpdate();
    }
    else
    {
        if(actualDefaultDeviceName == sourceAname)
            actualDefaultDeviceName = sourceBname;
        else
            actualDefaultDeviceName = sourceAname;

        executeAudioDeviceChange(actualDefaultDeviceName);
        iconUpdate();

        // Refresh the device to be checked for the sub menu that permits to manually change the audio device
        for(auto act : listChangeAudioOutputMenu->actions())
        {
            act->setChecked(false);
            if(act->text() == actualDefaultDeviceName)
                act->setChecked(true);
        }
    }
}

// Refreshes all audio devices if needed (in case of connection/disconnection of device for example)
void MainWidget::refreshDevicesLists()
{
    buildListChangeAudioOutput();
    buildConfigureSwapMenu();
    applySettings();
    iconUpdate();
    saveSettings();
}

void MainWidget::swapDeviceFromIconClick(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger
            || reason == QSystemTrayIcon::MiddleClick)
    swapDevice();
}

void MainWidget::quitApp()
{
    saveSettings();

    QApplication::quit();
}

void MainWidget::changeStartupOption(bool checked)
{
    doLaunchOnStartup = checked;
    saveSettings();
}

void MainWidget::onChangeListAudioOutput(QAction *action)
{
    for(auto act : listChangeAudioOutputMenu->actions())
        act->setChecked(false);

    action->setChecked(true);
    actualDefaultDeviceName = action->text();

    executeAudioDeviceChange(actualDefaultDeviceName);
    iconUpdate();
    saveSettings();
}

void MainWidget::onSourceAChoice(QAction *action)
{
    for(auto act : listSourceAMenu->actions())
        act->setChecked(false);
    action->setChecked(true);
    sourceAname = action->text();

    iconUpdate();
    saveSettings();
}

void MainWidget::onSourceBChoice(QAction *action)
{
    for(auto act : listSourceBMenu->actions())
        act->setChecked(false);
    action->setChecked(true);
    sourceBname = action->text();

    iconUpdate();
    saveSettings();
}
