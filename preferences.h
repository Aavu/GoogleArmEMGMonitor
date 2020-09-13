#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>


namespace Ui {
class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT

public:
    struct pref_t {
        QString host;
        uint16_t iPort;
        uint16_t iBlockSize;
        uint16_t iSensorGain;
        QString defaultSavePath;
        uint16_t iRecordingLength;
        QString serverPath;
        QString annotationMapJSONFilePath;
    };

    explicit Preferences(pref_t* pPref = nullptr, QWidget *parent = nullptr);
    ~Preferences();

    pref_t getPref();
    void setPref(pref_t pref);

    void setBlockSize(uint16_t blockSize);
    void setSensorGain(uint16_t sensorGain);
    void setDefaultSavePath(QString path);
    void setRecordingLength(uint16_t recLength);

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

    void on_btn_annotationMapFilePath_clicked();

private:
    Ui::Preferences *ui;
    pref_t m_pref;
};

#endif // PREFERENCES_H
