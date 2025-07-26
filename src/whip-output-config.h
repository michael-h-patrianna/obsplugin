/*!
 * @file whip-output-config.h
 * @brief Declaration of WHIP-specific output configuration for PlayFame.
 */

#pragma once

#include "output-config.h"
#include <QDateTime>

/**
 * @brief Configuration specific to WHIP outputs (PlayFame)
 */
class WHIPOutputConfig : public OutputConfig {
public:
    explicit WHIPOutputConfig();
    ~WHIPOutputConfig() override;

    // WHIP-specific fields
    bool useOAuth{true};
    bool useAutoReconnect{true};  // Whether to automatically reconnect on disconnect
    QString accessToken;
    QString streamKey;
    QString ingestUrl;
    QDateTime expiresAt;
    QString whipTransport{"tcp"};  // tcp or udp
    QString dtlsRole{"auto"};      // auto, server, client

    void loadFromObs(obs_data_t *data) override;
    void saveToObs(obs_data_t *data) const override;
    bool validate() const override;

    /// Check if the access token is expired or about to expire
    bool isTokenExpired() const;
    bool isTokenExpiringSoon() const;

    /// Get a list of supported WHIP transport types
    static QStringList supportedTransports();

    /// Get a list of supported DTLS roles
    static QStringList supportedDtlsRoles();

private:
    static bool validateUrl(const QString &url);
    static constexpr int kExpiryWarningHours = 48;
}; 
