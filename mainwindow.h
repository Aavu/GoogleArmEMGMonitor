#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <fstream>

#include <QMainWindow>
#include <QFileDialog>
#include <QDir>
#include <QCloseEvent>
#include <QComboBox>
#include "qcustomplot.h"
#include "emgdatasource.h"
#include "SshSession.h"
#include "MyTime.h"
#include "TempFile.h"

#include "preferences.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define APP_NAME "Google Arm EMG Monitor"

/**
 * @brief The Main Window class to display on screen
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Axis {
        XAxis,
        YAxis
    };

    /**
     * @brief Main Window constructor
     * @param parent : Parent widget if any
     */
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:

    /**
     * @brief Slot to update Status Bar Message
     * @param message : the msg string
     */
    void updateStatusBarMessage(QString message);

    /**
     * @brief slot to update the Plots
     */
    void updatePlots();

    /**
     * @brief slot for number of Samples Recorded
     * @param nSamplesRecorded : How many
     */
    void nSamplesRecorded(qulonglong nSamplesRecorded);

    /**
     * @brief callback when recordBtn is clicked
     */
    void on_recordBtn_clicked();

    /**
     * @brief callback when actionAmplitude is triggered
     * @param checked : Whether checked or not
     */
    void on_actionAmplitude_triggered(bool checked);

    /**
     * @brief callback when connectBtn is clicked
     */
    void on_connectBtn_clicked();

    /**
     * @brief callback when Save_As is triggered
     */
    void on_actionSave_As_triggered();

    /**
     * @brief callback when Preferences is triggered
     */
    void on_actionPreferences_triggered(bool);

    /**
     * @brief callback when Save is triggered
     */
    void on_actionSave_triggered();

    /**
     * @brief slot for current Index Changed
     * @param index : The index
     */
    void currentIndexChanged(int index);

    /**
     * @brief callback when Time is triggered
     * @param checked : Whether it is checked
     */
    void on_actionTime_triggered(bool checked);

private:
    /**
     * @brief Initialize the Axes
     */
    void initAxes();

    /**
     * @brief Initialize Graphs
     */
    void initGraphs();

    /**
     * @brief Toggle Axis Visibilty
     * @param axis : The axis to affect
     * @param show : Whether to show or hide
     */
    void toggleAxisVisibilty(Axis axis, bool show);

    /**
     * @brief Connect Y Axes Range
     */
    void connectYAxesRange();

    /**
     * @brief Clear Graphs
     * @param idx : the index to clear
     */
    void clearGraphs(int idx);

    /**
     * @brief Close Event
     * @param event : Pointer to the event details
     */
    void closeEvent (QCloseEvent *event);

    /**
     * @brief set Recording to enable or disable
     * @param record : booleon to set
     */
    void setRecording(bool record);

    /**
     * @brief Update Combo Box
     * @param init : Whether to initialize of not
     */
    void updateComboBox(bool init = false);

    /**
     * @brief parse the Annotation Map JSON file
     * @return Error_t
     */
    Error_t parseAnnotationMap();

    Ui::MainWindow *ui;
    QCustomPlot* m_plotWidget;
    QList<QCPAxis*> m_yAxes;
    QList<QCPAxis*> m_xAxes;
    QList<QCPGraph*> m_graph;

    int xAxisIdx = 0;

    QTimer m_dataTimer;
    size_t m_iRefreshTime = 1; /* ms */

    QString m_host = "googlearm.local";
    uint16_t m_iPort = 8080;

    uint16_t* m_pData = nullptr;
    quint8 m_iTotalChannels;
    quint8 m_iNumEMGChannels;
    quint16 m_iTimeSize;
    int m_iParentAxisID;
    bool m_bShowYAxis = false;
    bool m_bShowXAxis = true;

    QString m_defaultSavePath = QDir::homePath() + "/Documents/GoogleArm/Data/";

    bool m_bIsConnected = false;

    EMGDataSource* m_pEMGDataSrc;

    TempFile m_tempFile;

    static constexpr Qt::GlobalColor colors[] = {Qt::gray, Qt::cyan, Qt::red, Qt::white, Qt::yellow, Qt::green, Qt::blue, Qt::magenta, Qt::black};

    QStringList m_gestureNameList {};
    int m_iCurrentSessionGesture;

    // Annotation Map - JSON
    QString m_annotationMapJSONFilePath = QDir::homePath() + "/Documents/GoogleArm/annotation_map.json";

    static inline const QString INACTIVITY = "inactivity";

};
#endif // MAINWINDOW_H
