/*!
 * @file rtmp-output-config.cpp
 * @brief Implementation of RTMP-specific output configuration.
 */

#include "rtmp-output-config.h"
#include <QRegularExpression>

RTMPOutputConfig::RTMPOutputConfig() = default;
RTMPOutputConfig::~RTMPOutputConfig() = default;

void RTMPOutputConfig::loadFromObs(obs_data_t *data)
{
    OutputConfig::loadFromObs(data);
    serverUrl = QString::fromUtf8(obs_data_get_string(data, "server_url"));
    backupServerUrl = QString::fromUtf8(obs_data_get_string(data, "backup_server_url"));
    streamKey = QString::fromUtf8(obs_data_get_string(data, "stream_key"));
    useAutoReconnect = obs_data_get_bool(data, "use_auto_reconnect");
}

void RTMPOutputConfig::saveToObs(obs_data_t *data) const
{
    OutputConfig::saveToObs(data);
    obs_data_set_string(data, "server_url", serverUrl.toUtf8().constData());
    obs_data_set_string(data, "backup_server_url", backupServerUrl.toUtf8().constData());
    obs_data_set_string(data, "stream_key", streamKey.toUtf8().constData());
    obs_data_set_bool(data, "use_auto_reconnect", useAutoReconnect);
}

bool RTMPOutputConfig::validate() const
{
    if (!OutputConfig::validate()) return false;

    // Validate RTMP-specific fields
    if (!validateRtmpUrl(serverUrl)) return false;
    if (!backupServerUrl.isEmpty() && !validateRtmpUrl(backupServerUrl)) return false;
    if (streamKey.isEmpty()) return false;

    return true;
}

bool RTMPOutputConfig::validateRtmpUrl(const QString &url)
{
    static const QRegularExpression re(R"(^rtmps?://.+)");
    return re.match(url).hasMatch();
} 

