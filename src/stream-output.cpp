/*!
 * @file stream-output.cpp
 * @brief Implementation of the base stream output class.
 */

#include "stream-output.h"
#include "plugin-support.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <obs-module.h>

StreamOutput::StreamOutput(OBSConfigHelper *cfg, const QString &serviceName, const QString &serviceIconPath, QWidget *parent)
    : QWidget(parent)
    , cfg_(cfg)
    , serviceName_(serviceName)
    , serviceIconPath_(serviceIconPath)
{
    // Initialize will be called from plugin-dock.cpp after construction
}

StreamOutput::~StreamOutput() = default;

void StreamOutput::initialize()
{
    setupUI();
    setupConnections();
    updateUI();
}

void StreamOutput::setupUI()
{
    // Safety check: Don't set layout if one already exists
    if (layout() != nullptr) {
        obs_log(LOG_WARNING, "[StreamOutput] setupUI() called but layout already exists");
        return;
    }

    // Main horizontal layout matching .stream-output-panel
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0); // gap: 10px
   
    setMinimumHeight(50); // min-height: 50px

    // Left panel - .stream-output-panel-left (48px + 10px padding on each side = 68px total)
    auto *leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(68); // Fixed width to prevent compression
    leftPanel->setStyleSheet("background: #323540;");
    leftPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    layout->addWidget(leftPanel);
    
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(10, 10, 10, 10); // padding: 10px on all sides
    leftLayout->setAlignment(Qt::AlignCenter);
    
    // Service logo - 48px width, auto height, placeholder for now
    iconLabel_ = new QLabel(leftPanel);
    iconLabel_->setFixedSize(48, 48);
    iconLabel_->setStyleSheet("background: white; border: none;");
    iconLabel_->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(iconLabel_);

    // Right panel - .stream-output-panel-right with correct background color
    auto *rightPanel = new QWidget(this);
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightPanel->setMinimumWidth(200); // Ensure minimum width for text
    layout->addWidget(rightPanel, 1); // width: 100%
    rightPanel->setStyleSheet("background: #1D1F26; font-size: 14px;");

    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(10, 10, 10, 10); // padding: 10px
    rightLayout->setSpacing(10); // gap: 10px

    // Header with title and status - .stream-output-header
    auto *headerWidget = new QWidget(rightPanel);
    headerWidget->setStyleSheet("background: transparent;"); // Inherit from parent
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(10); // Small gap between title and status
    rightLayout->addWidget(headerWidget);

    // Title - .stream-output-header-title (14px font size)
    titleLabel_ = new QLabel(headerWidget);
    titleLabel_->setStyleSheet("color: #fff; font-weight: 700; font-size: 14px; background: transparent;");
    titleLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titleLabel_->setWordWrap(false); // Don't wrap service names
    headerLayout->addWidget(titleLabel_, 1); // Give title space to expand
    
    // Status badge - .stream-output-status with pill shape and 12px font
    statusLabel_ = new QLabel(headerWidget);
    statusLabel_->setStyleSheet(
        "border-radius: 12px; " // Pill shape (half of height for perfect pill)
        "padding: 4px 8px; "
        "font-size: 12px; " // 12px font size as requested
        "line-height: 12px; "
        "text-transform: uppercase;"
    );
    statusLabel_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    headerLayout->addWidget(statusLabel_);

    // Message row - .stream-output-message (14px font size)
    messageLabel_ = new QLabel(rightPanel);
    messageLabel_->setStyleSheet("color: white; font-size: 14px; background: transparent;");
    messageLabel_->setWordWrap(true);
    messageLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    rightLayout->addWidget(messageLabel_);

    // Button row - .stream-output-buttons
    auto *buttonWidget = new QWidget(rightPanel);
    buttonWidget->setStyleSheet("background: transparent;"); // Inherit from parent
    auto *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(4); // gap: 4px (not 10px!)
    rightLayout->addWidget(buttonWidget);

    // Buttons with proper icons and 14px font size
    startButton_ = new QPushButton(buttonWidget);
    startButton_->setFixedSize(32, 32); // height: 32px; width: 32px
    startButton_->setText("▶"); // Solid play triangle
    startButton_->setStyleSheet(
        "QPushButton {"
        "  height: 32px; width: 32px; "
        "  border: 1px solid #999999; "
        "  background: transparent; "
        "  color: #999999; "
        "  font-size: 14px; " // 14px font size as requested
        "  text-align: center; "
        "  font-weight: bold; "
        "} "
        "QPushButton.enabled {"
        "  border: 1px solid #fff; "
        "  background: transparent; "
        "  color: #fff; "
        "  cursor: pointer; "
        "}"
    );
    buttonLayout->addWidget(startButton_);

    stopButton_ = new QPushButton(buttonWidget);
    stopButton_->setFixedSize(32, 32);
    stopButton_->setText("⏸"); // Solid pause symbol
    stopButton_->setStyleSheet(
        "QPushButton {"
        "  height: 32px; width: 32px; "
        "  border: 1px solid #999999; "
        "  background: transparent; "
        "  color: #999999; "
        "  font-size: 14px; " // 14px font size
        "  text-align: center; "
        "  font-weight: bold; "
        "} "
        "QPushButton.enabled {"
        "  border: 1px solid #fff; "
        "  background: transparent; "
        "  color: #fff; "
        "  cursor: pointer; "
        "}"
    );
    buttonLayout->addWidget(stopButton_);

    configButton_ = new QPushButton(buttonWidget);
    configButton_->setFixedSize(32, 32);
    configButton_->setText("⚙"); // Solid gear symbol
    configButton_->setStyleSheet(
        "QPushButton {"
        "  height: 32px; width: 32px; "
        "  border: 1px solid #999999; "
        "  background: transparent; "
        "  color: #999999; "
        "  font-size: 14px; " // 14px font size
        "  text-align: center; "
        "  font-weight: bold; "
        "} "
        "QPushButton.enabled {"
        "  border: 1px solid #fff; "
        "  background: transparent; "
        "  color: #fff; "
        "  cursor: pointer; "
        "}"
    );
    buttonLayout->addWidget(configButton_);

    // Add stretch to button layout to keep buttons on the left
    buttonLayout->addStretch();

    // Add stretch to right layout to align content to top
    rightLayout->addStretch();
}

void StreamOutput::setupConnections()
{
    connect(startButton_, &QPushButton::clicked, this, &StreamOutput::startStream);
    connect(stopButton_, &QPushButton::clicked, this, &StreamOutput::stopStream);
    connect(configButton_, &QPushButton::clicked, this, &StreamOutput::showConfig);
}

void StreamOutput::setState(OutputState newState)
{
    if (state_ != newState) {
        state_ = newState;
        updateUI();
    }
}

void StreamOutput::updateUI()
{
    if (!iconLabel_ || !titleLabel_ || !statusLabel_ || !messageLabel_) return;

    // Set the service name as title
    titleLabel_->setText(serviceName());

    // Update status badge and message based on state - matching HTML CSS classes
    QString statusText;
    QString statusStyle;
    QString messageText;
    QString messageStyle = "color: white; font-size: 14px; background: transparent;"; // Default message style
    
    switch (state_) {
        case OutputState::Unused:
            statusText = "UNUSED";
            statusStyle = "color: #999999; background: transparent;"; // .stream-output-status-unused
            messageText = "";
            break;
        case OutputState::Offline:
            statusText = "OFFLINE";
            statusStyle = "color: #fff; background: #5C5C5C;"; // .stream-output-status-offline
            messageText = "";
            break;
        case OutputState::Connecting:
            statusText = "CONNECTING";
            statusStyle = "color: #fff; background: #B88A16;"; // .stream-output-status-connecting
            messageText = "Connecting to server...";
            break;
        case OutputState::Error:
            statusText = "ERROR";
            statusStyle = "color: #fff; background: #C01C37;"; // .stream-output-status-error
            messageText = "Connection failed - check your settings";
            messageStyle = "color: #E85E75; font-size: 14px; background: transparent;"; // .stream-output-message-error
            break;
        case OutputState::Online:
            statusText = "ONLINE";
            statusStyle = "color: #fff; background: #25A231;"; // .stream-output-status-online
            messageText = "Streaming - 1080p @ 6000 kbps";
            messageStyle = "color: #59D966; font-size: 14px; background: transparent;"; // .stream-output-message-online
            break;
    }
    
    // Apply status styling with pill shape and 12px font
    statusLabel_->setText(statusText);
    statusLabel_->setStyleSheet(
        "border-radius: 12px; " // Pill shape
        "padding: 4px 8px; "
        "font-size: 12px; " // 12px font for status
        "line-height: 12px; "
        "text-transform: uppercase; " +
        statusStyle
    );

    // Apply message styling
    messageLabel_->setText(messageText);
    messageLabel_->setStyleSheet(messageStyle);
    
    // Show/hide message based on content
    messageLabel_->setVisible(!messageText.isEmpty());

    // Update button states with direct styling (14px font size)
    bool canStart = (state_ == OutputState::Offline || state_ == OutputState::Error);
    bool canStop = (state_ == OutputState::Online || state_ == OutputState::Connecting);
    bool configAlwaysEnabled = true;
    
    // Define button styles with 14px font
    QString enabledButtonStyle = 
        "QPushButton {"
        "  height: 32px; width: 32px; "
        "  border: 1px solid #fff; "
        "  background: transparent; "
        "  color: #fff; "
        "  font-size: 14px; " // 14px font for buttons
        "  text-align: center; "
        "  font-weight: bold; "
        "} "
        "QPushButton:hover {"
        "  background: rgba(255, 255, 255, 0.1); "
        "}";
    
    QString disabledButtonStyle = 
        "QPushButton {"
        "  height: 32px; width: 32px; "
        "  border: 1px solid #999999; "
        "  background: transparent; "
        "  color: #999999; "
        "  font-size: 14px; " // 14px font for buttons
        "  text-align: center; "
        "  font-weight: bold; "
        "}";
    
    // Apply button styles directly
    startButton_->setEnabled(canStart);
    startButton_->setStyleSheet(canStart ? enabledButtonStyle : disabledButtonStyle);
    
    stopButton_->setEnabled(canStop);
    stopButton_->setStyleSheet(canStop ? enabledButtonStyle : disabledButtonStyle);
    
    configButton_->setEnabled(configAlwaysEnabled);
    configButton_->setStyleSheet(configAlwaysEnabled ? enabledButtonStyle : disabledButtonStyle);
}

void StreamOutput::updateButtonStates()
{
    if (!startButton_ || !stopButton_ || !configButton_) return;

    QString enabledButtonStyle = 
        "QPushButton { "
        "  padding: 6px; "
        "  border-radius: 3px; "
        "  border: 1px solid white; "
        "  background: transparent; "
        "  color: white; "
        "  font-size: 12px; "
        "} "
        "QPushButton:hover { background: rgba(255,255,255,0.1); }";
    
    QString disabledButtonStyle = 
        "QPushButton { "
        "  padding: 6px; "
        "  border-radius: 3px; "
        "  border: 1px solid #999999; "
        "  background: transparent; "
        "  color: #5C5C5C; "
        "  font-size: 12px; "
        "}";

    switch (state_) {
        case OutputState::Unused:
            // Only config enabled
            startButton_->setEnabled(false);
            stopButton_->setEnabled(false);
            configButton_->setEnabled(true);
            startButton_->setStyleSheet(disabledButtonStyle);
            stopButton_->setStyleSheet(disabledButtonStyle);
            configButton_->setStyleSheet(enabledButtonStyle);
            break;
            
        case OutputState::Offline:
            // Start and config enabled
            startButton_->setEnabled(true);
            stopButton_->setEnabled(false);
            configButton_->setEnabled(true);
            startButton_->setStyleSheet(enabledButtonStyle);
            stopButton_->setStyleSheet(disabledButtonStyle);
            configButton_->setStyleSheet(enabledButtonStyle);
            break;
            
        case OutputState::Connecting:
            // Only stop and config enabled
            startButton_->setEnabled(false);
            stopButton_->setEnabled(true);
            configButton_->setEnabled(true);
            startButton_->setStyleSheet(disabledButtonStyle);
            stopButton_->setStyleSheet(enabledButtonStyle);
            configButton_->setStyleSheet(enabledButtonStyle);
            break;
            
        case OutputState::Online:
            // Only stop and config enabled  
            startButton_->setEnabled(false);
            stopButton_->setEnabled(true);
            configButton_->setEnabled(true);
            startButton_->setStyleSheet(disabledButtonStyle);
            stopButton_->setStyleSheet(enabledButtonStyle);
            configButton_->setStyleSheet(enabledButtonStyle);
            break;
            
        case OutputState::Error:
            // Start and config enabled
            startButton_->setEnabled(true);
            stopButton_->setEnabled(false);
            configButton_->setEnabled(true);
            startButton_->setStyleSheet(enabledButtonStyle);
            stopButton_->setStyleSheet(disabledButtonStyle);
            configButton_->setStyleSheet(enabledButtonStyle);
            break;
    }
}

QString StreamOutput::stateToString(OutputState state)
{
    switch (state) {
        case OutputState::Unused:     return tr("UNUSED");
        case OutputState::Offline:    return tr("OFFLINE");
        case OutputState::Connecting: return tr("CONNECTING");
        case OutputState::Online:     return tr("ONLINE");
        case OutputState::Error:      return tr("ERROR");
    }
    return QString();
}

QString StreamOutput::stateToStyleClass(OutputState state)
{
    switch (state) {
        case OutputState::Unused:     return "state-unused";
        case OutputState::Offline:    return "state-offline";
        case OutputState::Connecting: return "state-connecting";
        case OutputState::Online:     return "state-online";
        case OutputState::Error:      return "state-error";
    }
    return QString();
}

void StreamOutput::showConfig()
{
    // Default implementation - could show a generic config dialog
    obs_log(LOG_INFO, "[StreamOutput] Config requested for %s", serviceName_.toUtf8().constData());
}

void StreamOutput::loadConfig()
{
    if (!cfg_) {
        obs_log(LOG_WARNING, "[StreamOutput] No config helper available for %s", serviceName_.toUtf8().constData());
        return;
    }
    
    QString prefix = serviceName_.toLower() + "_";
    // Load basic config - can be overridden in derived classes
    obs_log(LOG_INFO, "[StreamOutput] Loading config for %s", serviceName_.toUtf8().constData());
}

void StreamOutput::saveConfig()
{
    if (!cfg_) {
        obs_log(LOG_WARNING, "[StreamOutput] No config helper available for %s", serviceName_.toUtf8().constData());
        return;
    }
    
    QString prefix = serviceName_.toLower() + "_";
    // Save basic config - can be overridden in derived classes
    obs_log(LOG_INFO, "[StreamOutput] Saving config for %s", serviceName_.toUtf8().constData());
}

bool StreamOutput::validateConfig() const
{
    // Default implementation - always valid
    return true;
}

void StreamOutput::startStream()
{
    obs_log(LOG_INFO, "[StreamOutput] Starting stream for %s", serviceName_.toUtf8().constData());
    if (!validateConfig()) {
        setState(OutputState::Error);
        return;
    }
    setState(OutputState::Connecting);
    // TODO: Implement actual streaming logic
}

void StreamOutput::stopStream()
{
    obs_log(LOG_INFO, "[StreamOutput] Stopping stream for %s", serviceName_.toUtf8().constData());
    setState(OutputState::Offline);
    // TODO: Implement actual stop logic
} 
