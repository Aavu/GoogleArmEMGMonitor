#include "preferences.h"
#include "ui_preferences.h"
#include "QAbstractButton"
#include "QFileDialog"
#include <QStandardPaths>

Preferences::Preferences(pref_t* pPref, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);

    if (pPref)
        setPref(*pPref);
    else {
        m_pref.host = "googlearm.local";
        m_pref.iPort = 8080;
        m_pref.iBlockSize = 8;
        m_pref.iSensorGain = 2;
        m_pref.defaultSavePath = QDir::homePath() + "/Documents/GoogleArm/Data/";
        m_pref.iRecordingLength = 60;
        m_pref.serverPath = "/home/pi/EMGDataServer/";
        m_pref.annotationMapJSONFilePath = QDir::homePath() + "/Documents/GoogleArm/annotation_map.json";
    }

    ui->le_host->setText(m_pref.host);
    ui->le_port->setText(QString::number(m_pref.iPort));
    ui->le_gain->setText(QString::number(m_pref.iSensorGain));
    ui->le_blockSize->setText(QString::number(m_pref.iBlockSize));
    ui->le_savePath->setText(m_pref.defaultSavePath);
    ui->le_recordingLength->setText(QString::number(m_pref.iRecordingLength));
    ui->le_serverPath->setText(m_pref.serverPath);
    ui->annotationMapFilePath->setText(m_pref.annotationMapJSONFilePath);
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
    if (button->text() == "Cancel") {
        return;
    }

    m_pref.host                         = ui->le_host->text();
    m_pref.iPort                        = ui->le_port->text().toUInt();
    m_pref.iSensorGain                  = ui->le_gain->text().toUInt();
    m_pref.iBlockSize                   = ui->le_blockSize->text().toUInt();
    m_pref.defaultSavePath              = ui->le_savePath->text();
    m_pref.iRecordingLength             = ui->le_recordingLength->text().toUInt();
    m_pref.serverPath                   = ui->le_serverPath->text();
    m_pref.annotationMapJSONFilePath    = ui->annotationMapFilePath->text();
}

void Preferences::on_btn_annotationMapFilePath_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose annotation_map"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    tr("Annotation Map (*.json)"));

    if (fileName.isEmpty())
        return;

    m_pref.annotationMapJSONFilePath = fileName;
    ui->annotationMapFilePath->setText(fileName);

}
