#include "idlenotificationdialog.h"
#include "ui_idlenotificationdialog.h"

#include <QDebug>

#include "toggl_api.h"
#include "settingsview.h"

#include <X11/extensions/scrnsaver.h>

IdleNotificationDialog::IdleNotificationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IdleNotificationDialog),
    idleStarted(0),
    timer(new QTimer(this))
{
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displayIdleNotification(QString,QString,uint64_t)),
            this, SLOT(displayIdleNotification(QString,QString,uint64_t)));

    connect(TogglApi::instance, SIGNAL(displaySettings(bool,SettingsView*)),
            this, SLOT(displaySettings(bool,SettingsView*)));

    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

IdleNotificationDialog::~IdleNotificationDialog()
{
    delete ui;
}

void IdleNotificationDialog::on_keepTimeButton_clicked()
{
    hide();
}

void IdleNotificationDialog::on_discardTimeButton_clicked()
{

}

void IdleNotificationDialog::displaySettings(
    const bool open,
    SettingsView *settings)
{
    if (settings->UseIdleDetection && !timer->isActive())
    {
        timer->start(1000);
    }
    else if (!settings->UseIdleDetection && timer->isActive())
    {
        timer->stop();
    }
}

void IdleNotificationDialog::displayIdleNotification(
    const QString since,
    const QString duration,
    const uint64_t started)
{
    idleStarted = started;

    ui->idleSince->setText(since);
    ui->idleDuration->setText(duration);

    show();
}

void IdleNotificationDialog::timeout()
{
    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        return;
    }
    XScreenSaverInfo *info = XScreenSaverAllocInfo();
    if (XScreenSaverQueryInfo(display, DefaultRootWindow(display), info))
    {
        uint64_t idleSeconds = info->idle / 1000;
        qDebug() << "idle seconds " << idleSeconds;
        TogglApi::instance->setIdleSeconds(idleSeconds);
    }
    XFree(info);
    XCloseDisplay(display);
}