#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include "obs-config-helper.h"

class ConfigDialog : public QDialog {
    Q_OBJECT
public:
    ConfigDialog(OBSConfigHelper *cfg, QWidget *parent = nullptr);

private:
    OBSConfigHelper *cfg_;
    QLineEdit  *txt_;
    QSpinBox   *num_;
    QComboBox  *opt_;
    void loadFromCfg();
    void saveToCfg();

private slots:
    void onLoad();
    void onSave();
};
