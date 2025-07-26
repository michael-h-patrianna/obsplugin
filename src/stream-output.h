/*!
 * @file stream-output.h
 * @brief Declaration of the base stream output class and related types.
 */

#pragma once

#include <QWidget>
#include <QString>
#include <QIcon>
#include <memory>

class QPushButton;
class QLabel;
class OBSConfigHelper;

/**
 * @brief Represents the state of a streaming output
 */
enum class OutputState {
    Unused,     ///< Not configured
    Offline,    ///< Configured but not streaming
    Connecting, ///< OBS connecting/handshaking
    Online,     ///< Actively streaming
    Error       ///< Failed to start / lost connection
};

/**
 * @brief Base class for all streaming outputs (PlayFame, RTMP services, etc.)
 */
class StreamOutput : public QWidget {
    Q_OBJECT

public:
    explicit StreamOutput(OBSConfigHelper *cfg, const QString &serviceName, const QString &serviceIconPath, QWidget *parent = nullptr);
    ~StreamOutput() override;

    void initialize();
    void setState(OutputState newState);
    OutputState state() const { return state_; }

    QString serviceName() const { return serviceName_; }
    QString serviceIconPath() const { return serviceIconPath_; }

    static QString stateToString(OutputState state);
    static QString stateToStyleClass(OutputState state);

protected:
    void updateUI();
    virtual void showConfig();
    virtual void startStream();
    virtual void stopStream();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual bool validateConfig() const;

protected:
    OutputState state_{OutputState::Unused};
    OBSConfigHelper *cfg_;
    QString serviceName_;
    QString serviceIconPath_;
    
    // UI Elements
    QLabel *iconLabel_{nullptr};
    QLabel *titleLabel_{nullptr};
    QLabel *statusLabel_{nullptr};
    QLabel *messageLabel_{nullptr};
    QPushButton *startButton_{nullptr};
    QPushButton *stopButton_{nullptr};
    QPushButton *configButton_{nullptr};

private:
    void setupUI();
    void setupConnections();
    void updateButtonStates();
};

