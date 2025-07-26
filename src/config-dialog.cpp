// ─────────── config-dialog.cpp ───────────
#include "config-dialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "toast-helper.h"

ConfigDialog::ConfigDialog(OBSConfigHelper *cfg, QWidget *parent)
    : QDialog(parent)
    , cfg_(cfg)
{
    setWindowTitle("PlayFame – Config");
    setModal(true);
    resize(300, 180);

    auto *lay = new QVBoxLayout(this);

    txt_ = new QLineEdit(this);
    txt_->setPlaceholderText("Your text…");
    num_ = new QSpinBox(this);
    num_->setRange(0, 9999);

    opt_ = new QComboBox(this);
    for (int i = 1; i <= 5; ++i)
        opt_->addItem(QString("Option %1").arg(i), i);

    lay->addWidget(txt_);
    lay->addWidget(num_);
    lay->addWidget(opt_);

    auto *btnBox = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    auto *loadBtn = new QPushButton("Load", this);
    btnBox->addButton(loadBtn, QDialogButtonBox::ActionRole);

    lay->addWidget(btnBox);
    setLayout(lay);

    connect(btnBox->button(QDialogButtonBox::Cancel),
            &QPushButton::clicked, this, &ConfigDialog::reject);
    connect(btnBox->button(QDialogButtonBox::Save),
            &QPushButton::clicked, this, &ConfigDialog::accept);
    connect(loadBtn, &QPushButton::clicked, this, &ConfigDialog::onLoad);

    loadFromCfg();
}

void ConfigDialog::loadFromCfg()
{
    txt_->setText(cfg_->getValue(kSec, "text", "hello").toString());
    num_->setValue(cfg_->getValue(kSec, "number", 0).toInt());
    int idx = cfg_->getValue(kSec, "option", 1).toInt() - 1;
    opt_->setCurrentIndex(std::clamp(idx, 0, 4));
}

void ConfigDialog::saveToCfg()
{
    // Use the simple setValue overload (void return type) by calling it directly
    auto setValue = static_cast<void(OBSConfigHelper::*)(const QString&, const QString&, const QVariant&)>
        (&OBSConfigHelper::setValue);
    (cfg_->*setValue)(kSec, "text", txt_->text());
    (cfg_->*setValue)(kSec, "number", num_->value());
    (cfg_->*setValue)(kSec, "option", opt_->currentData().toInt());
}

void ConfigDialog::onLoad()
{
    loadFromCfg();
    showToast(this, tr("Config loaded"));    
}

void ConfigDialog::accept()
{
    saveToCfg();
    QDialog::accept();
}

void ConfigDialog::reject()
{
    loadFromCfg();
    QDialog::reject();
}

