#include "preferences.h"
#include "ui_preferences.h"

Preferences::Preferences(pref_t* pPref, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);

    if (pPref)
        setPref(*pPref);
    else {
        m_pref.iBlockSize = 8;
        m_pref.iSensorGain = 2;
        m_pref.defaultSavePath = "~/Documents/GoogleArm/Data/";
        m_pref.iRecordingLength = 60;
    }

    ui->le_gain->setText(QString::number(m_pref.iSensorGain));
    ui->le_blockSize->setText(QString::number(m_pref.iBlockSize));
    ui->le_savePath->setText(m_pref.defaultSavePath);
    ui->le_recordingLength->setText(QString::number(m_pref.iRecordingLength));
}

Preferences::~Preferences() {
    delete ui;
}

Preferences::pref_t Preferences::getPref() {
    return m_pref;
}

void Preferences::setPref(pref_t pref) {
    m_pref = pref;
}

void Preferences::setBlockSize(uint16_t blockSize) {
    m_pref.iBlockSize = blockSize;
}

void Preferences::setSensorGain(uint16_t sensorGain) {
    m_pref.iSensorGain = sensorGain;
}

void Preferences::setDefaultSavePath(QString path) {
    m_pref.defaultSavePath = path;
}

void Preferences::setRecordingLength(uint16_t recLength) {
    m_pref.iRecordingLength = recLength;
}

void Preferences::on_buttonBox_clicked(QAbstractButton *button)
{
    m_pref.iSensorGain      = ui->le_gain->text().toUInt();
    m_pref.iBlockSize       = ui->le_blockSize->text().toUInt();
    m_pref.defaultSavePath  = ui->le_savePath->text();
    m_pref.iRecordingLength = ui->le_recordingLength->text().toUInt();
}
