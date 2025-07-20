#pragma once
#include <obs-frontend-api.h>
#include <QMessageBox>
#include <QString>
#include <QWidget>

/* -------------------------------------------------------------------------
 *  showToast(QWidget*, QString) – call OBS' UI toast if the enum exists,
 *  otherwise fall back to a simple information box.
 * ------------------------------------------------------------------------- */
inline void showToast(QWidget *parent, const QString &msg,
                      bool warning = false)
{
#if defined(OBS_FRONTEND_NOTIFICATION_INFO)
    /* OBS 29+ full-tree build – enum & function are visible */
    obs_frontend_push_ui_notification(
        warning ? OBS_FRONTEND_NOTIFICATION_WARNING
                : OBS_FRONTEND_NOTIFICATION_INFO,
        msg.toUtf8().constData());
#else
    /* SDK build: use a regular Qt popup instead */
    QMessageBox::information(parent,
                             warning ? QObject::tr("Warning")
                                     : QObject::tr("Information"),
                             msg);
#endif
}
