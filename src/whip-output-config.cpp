/*!
 * @file whip-output-config.cpp
 * @brief Implementation of WHIP-specific output configuration for PlayFame.
 */

#include "whip-output-config.h"
#include <QRegularExpression>

WHIPOutputConfig::WHIPOutputConfig() = default;
WHIPOutputConfig::~WHIPOutputConfig() = default;

void WHIPOutputConfig::loadFromObs(obs_data_t *data)
{
    OutputConfig::loadFromObs(data);
    useOAuth = obs_data_get_bool(data, "use_oauth");
    accessToken = QString::fromUtf8(obs_data_get_string(data, "access_token"));
    streamKey = QString::fromUtf8(obs_data_get_string(data, "stream_key"));
    ingestUrl = QString::fromUtf8(obs_data_get_string(data, "ingest_url"));
    expiresAt = QDateTime::fromString(
        QString::fromUtf8(obs_data_get_string(data, "expires_at")),
        Qt::ISODate);
    whipTransport = QString::fromUtf8(obs_data_get_string(data, "whip_transport"));
    dtlsRole = QString::fromUtf8(obs_data_get_string(data, "dtls_role"));
}

void WHIPOutputConfig::saveToObs(obs_data_t *data) const
{
    OutputConfig::saveToObs(data);
    obs_data_set_bool(data, "use_oauth", useOAuth);
    obs_data_set_string(data, "access_token", accessToken.toUtf8().constData());
    obs_data_set_string(data, "stream_key", streamKey.toUtf8().constData());
    obs_data_set_string(data, "ingest_url", ingestUrl.toUtf8().constData());
    obs_data_set_string(data, "expires_at", 
        expiresAt.toString(Qt::ISODate).toUtf8().constData());
    obs_data_set_string(data, "whip_transport", whipTransport.toUtf8().constData());
    obs_data_set_string(data, "dtls_role", dtlsRole.toUtf8().constData());
}

bool WHIPOutputConfig::validate() const
{
    if (!OutputConfig::validate()) return false;

    // Validate WHIP-specific fields
    if (useOAuth) {
        if (accessToken.isEmpty()) return false;
        if (isTokenExpired()) return false;
    }

    if (!validateUrl(ingestUrl)) return false;
    if (streamKey.isEmpty()) return false;

    // Validate transport and DTLS role
    if (!supportedTransports().contains(whipTransport)) return false;
    if (!supportedDtlsRoles().contains(dtlsRole)) return false;

    return true;
}

bool WHIPOutputConfig::isTokenExpired() const
{
    return !expiresAt.isValid() || expiresAt <= QDateTime::currentDateTime();
}

bool WHIPOutputConfig::isTokenExpiringSoon() const
{
    if (!expiresAt.isValid()) return true;
    return expiresAt <= QDateTime::currentDateTime().addSecs(kExpiryWarningHours * 3600);
}

QStringList WHIPOutputConfig::supportedTransports()
{
    return {"tcp", "udp"};
}

QStringList WHIPOutputConfig::supportedDtlsRoles()
{
    return {"auto", "server", "client"};
}

bool WHIPOutputConfig::validateUrl(const QString &url)
{
    static const QRegularExpression re(R"(^https?://.+)");
    return re.match(url).hasMatch();
} 
