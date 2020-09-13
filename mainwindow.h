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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Axis {
        XAxis,
        YAxis
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateStatusBarMessage(QString message);
    void updatePlots();
    void nSamplesRecorded(qulonglong nSamplesRecorded);

    void on_recordBtn_clicked();
    void on_actionAmplitude_triggered(bool checked);
    void on_connectBtn_clicked();

    void on_actionSave_As_triggered();

    void on_actionPreferences_triggered(bool);

    void on_actionSave_triggered();

    void currentIndexChanged(int index);

    void on_actionTime_triggered(bool checked);

private:
    void initAxes();
    void initGraphs();
    void toggleAxisVisibilty(Axis axis, bool show);
    void connectYAxesRange();
    void clearGraphs(int idx);

    void closeEvent (QCloseEvent *event);

    void setRecording(bool record);

    void displayFPS(double etime);

    // Combo Box
    void updateComboBox(bool init = false);
    //JSON
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
