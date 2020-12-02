#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>


namespace Ui {
class Preferences;
}

/**
 * @brief The Preferences class takes care of the user preferences of the whole app.
 * @note The states do not persist upon restarting the app
 */
class Preferences : public QDialog
{
    Q_OBJECT

public:

    /**
     * @brief The preferences struct
     */
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

    /**
     * @brief get the current preferences
     * @return Reference to preference
     */
    pref_t& getPref();

    /**
     * @brief set the Preferences
     * @param A reference to the preference
     */
    void setPref(const pref_t& pref);

    /**
     * @brief set the Block Size
     * @param blockSize
     */
    void setBlockSize(uint16_t blockSize);

    /**
     * @brief set the Sensor Gain
     * @param sensorGain
     */
    void setSensorGain(uint16_t sensorGain);

    /**
     * @brief set the Default Save Path
     * @param path : the path string
     */
    void setDefaultSavePath(QString path);

    /**
     * @brief set the Recording Length
     * @param recLength : the recording length
     */
    void setRecordingLength(uint16_t recLength);

private slots:
    /**
     * @brief slot when buttonBox clicked
     * @param button : The details of the button
     */
    void on_buttonBox_clicked(QAbstractButton *button);

    /**
     * @brief slot when annotationMapFilePath buttton is clicked
     */
    void on_btn_annotationMapFilePath_clicked();

private:
    Ui::Preferences *ui;
    pref_t m_pref;
};

#endif // PREFERENCES_H
