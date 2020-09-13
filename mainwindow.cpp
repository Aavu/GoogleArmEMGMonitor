#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_iTotalChannels(EMGDataSource::iTotalChannels)
    , m_iNumEMGChannels(EMGDataSource::iNumEMGChannels)
    , m_iTimeSize(4)
    , m_iParentAxisID(m_iNumEMGChannels - 1)
{
    ui->setupUi(this);

    ui->actionSave->setEnabled(false);
    ui->actionSave_As->setEnabled(false);

    m_plotWidget = ui->plotWidget;

    m_dataTimer.setInterval(m_iRefreshTime);

    initAxes();
    initGraphs();

    updateStatusBarMessage("Disconnected...");

    m_pEMGDataSrc = new EMGDataSource(m_iRefreshTime);
    connect(m_pEMGDataSrc, SIGNAL(sig_updateUI(QString)), this, SLOT(updateStatusBarMessage(QString)));
    connect(m_pEMGDataSrc, SIGNAL(sig_nSamplesRecorded(qulonglong)), this, SLOT(nSamplesRecorded(qulonglong)));

    m_pEMGDataSrc->init(&m_tempFile);

    m_pData = new uint16_t[m_iRefreshTime*m_iTotalChannels];

    parseAnnotationMap();

    // Combo box setup
    updateComboBox(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pEMGDataSrc;
    delete [] m_pData;
}

void MainWindow::initAxes() {
//    m_plotWidget->setOpenGl(true, 16);
    m_plotWidget->plotLayout()->clear();
    m_plotWidget->setBackground(this->palette().color(QPalette::ColorRole::Window));

    // init axes excluding the ground truth
    for(quint8 i=0; i< m_iNumEMGChannels; i++) {
        QCPAxisRect* axisRect = new QCPAxisRect(m_plotWidget);
        m_plotWidget->plotLayout()->addElement(i, 0, axisRect);

        QList<QCPAxis*> axes = axisRect->axes();
        foreach (auto* axis, axes) {
            axis->setLayer("axes");
            axis->setTickLabelColor(Qt::darkGray);

            auto aType = axis->axisType();
            if (aType == QCPAxis::atLeft) {
                m_yAxes << axis;
                axis->setRange(-1.1, 1.1);
            }
            else if (aType == QCPAxis::atBottom) {
                axis->setTicks(false);
                axis->setBasePen(QPen(Qt::PenStyle::NoPen));
                m_xAxes << axis;
            }

            axis->grid()->setLayer("grid");
            axis->grid()->setSubGridVisible(false);

        }
    }

    toggleAxisVisibilty(XAxis, true);
    toggleAxisVisibilty(YAxis, false);
    connectYAxesRange();

    connect(&m_dataTimer, SIGNAL(timeout()), this, SLOT(updatePlots()));
    m_dataTimer.start();

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s:%z");
    m_xAxes[7]->setTicker(timeTicker);

    m_plotWidget->rescaleAxes();
}

void MainWindow::toggleAxisVisibilty(Axis axis, bool show) {
    switch (axis) {
    case XAxis:
        m_xAxes[m_iParentAxisID]->setTicks(show);
        break;
    case YAxis:
        foreach(auto* axis, m_yAxes) {
            axis->setTicks(show);
            if (show)
                axis->grid()->setPen(QPen(Qt::darkGray));
            else
                axis->grid()->setPen(QPen(Qt::PenStyle::NoPen));
        }
        break;
    }
    m_plotWidget->replot();
}

void MainWindow::connectYAxesRange() {
    for(int i=0; i<m_iParentAxisID; i++) {
        connect(m_xAxes[m_iParentAxisID], SIGNAL(rangeChanged(QCPRange)), m_xAxes[i], SLOT(setRange(QCPRange)));
    }
}

void MainWindow::initGraphs() {
    for (int i=0; i < m_iNumEMGChannels; i++) {
        auto* graph = new QCPGraph(m_xAxes[i], m_yAxes[i]);
        graph->setName("Ch " + QString(i));
        graph->setPen(QPen(colors[i]));
        graph->setBrush(QBrush(colors[i]));
        graph->setAntialiased(true);
        m_graph << graph;
    }
}

void MainWindow::updatePlots() {
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0;
    static double prevKey = 0;
    if (!m_bIsConnected)
        return;

    auto* pBuf = m_pEMGDataSrc->getDataBufferPtr();
    pBuf->pop(m_pData, m_iRefreshTime);

    for (size_t i=0; i<m_iRefreshTime; i++) {
        for(int j=0; j<m_iNumEMGChannels; j++) {
            float point = (m_pData[(i*m_iTotalChannels) + (j + 1)] - (1 << 15)) / (float)(1 << 15);
            m_graph[j]->addData(key, point);
//            std::cout << m_pData[(i*m_iTotalChannels) + j]  << "\t";
//            m_graph[j]->addData(key, qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
        }
//        std::cout << std::endl;
    }

    m_xAxes[m_iParentAxisID]->setRange(key, m_iTimeSize, Qt::AlignRight);

    if (key - prevKey > .01) {
        m_plotWidget->replot();
        prevKey = key;
    }
}

void MainWindow::clearGraphs(int idx) {
    m_graph[idx]->data().data()->clear();
}

void MainWindow::updateComboBox(bool init) {
    if (init)
        connect(ui->cb_gestureName, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));

    ui->cb_gestureName->setEnabled(!m_gestureNameList.isEmpty());
    ui->cb_gestureName->clear();
    for (auto gesture : m_gestureNameList) {
        if (gesture == INACTIVITY) {
            ui->cb_gestureName->addItem("Choose a gesture");
        } else
            ui->cb_gestureName->addItem(gesture);
    }
}

void MainWindow::setRecording(bool record) {
    m_pEMGDataSrc->setRecording(record);
    ui->recordBtn->setText(record ? "Stop" : "Record");
    ui->recordBtn->setChecked(record);
    ui->cb_gestureName->setEnabled(!record);
}

void MainWindow::displayFPS(double etime) {
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;

    auto totalDP = 0;

    for (int i=0; i<m_iNumEMGChannels; i++) {
        totalDP += m_graph[i]->data().data()->size();
    }

    if (etime-lastFpsKey > 2) // average fps over 2 seconds
    {
      ui->statusBar->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(etime-lastFpsKey), 0, 'f', 0)
            .arg(totalDP)
            , 0);
      lastFpsKey = etime;
      frameCount = 0;
    }
}

void MainWindow::updateStatusBarMessage(QString message) {
    ui->statusBar->showMessage(message, 0);
    m_bIsConnected = (message == "connected...");

    if (m_bIsConnected)
        ui->lbl_status->setText("Connected");
    else
        ui->lbl_status->setText("Disconnected");

    ui->plotWidget->setEnabled(m_bIsConnected);

    ui->connectBtn->setEnabled(!m_bIsConnected);
    ui->recordBtn->setEnabled(m_bIsConnected && m_iCurrentSessionGesture > 0);
}

void MainWindow::nSamplesRecorded(qulonglong nSamplesRecorded) {
    ui->nSamplesRecordedLCD->display((int)nSamplesRecorded);

    if (EMGDataSource::iNumSamplesToRecord != 0)
        if ((nSamplesRecorded > EMGDataSource::iNumSamplesToRecord))
            setRecording(false);
}

Error_t MainWindow::parseAnnotationMap() {
    QFile file(m_annotationMapJSONFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        updateStatusBarMessage("Failed to open " + m_annotationMapJSONFilePath);
        qDebug() << "Failed to open" << m_annotationMapJSONFilePath;
        return kFileOpenError;
    }

    QTextStream txt(&file);
    auto json_string = txt.readAll();
    file.close();

    auto json_bytes = json_string.toLocal8Bit();
    auto json_doc = QJsonDocument::fromJson(json_bytes);

    if (json_doc.isNull()) {
        qDebug() << "Failed to create JSON doc.";
        return kUnknownError;
    }

    if(!json_doc.isObject()){
        qDebug() << "JSON is not an object.";
        return kUnknownError;
    }

    QJsonObject json_obj = json_doc.object();
    if(json_obj.isEmpty()){
        qDebug() << "JSON object is empty.";
        return kUnknownError;
    }

    QVariantMap json_map = json_obj.toVariantMap();
//    qDebug() << json_map[INACTIVITY].toInt();

    QVariantList gesturesList = json_map["gestures"].toList();
    if (gesturesList.empty()) {
        qDebug() << "Gestures list is empty... Recheck json file.";
    }

    for (auto gesture : gesturesList) {
        auto name = gesture.toString();
        m_gestureNameList.append(gesture.toString());
    }

    qDebug() << m_gestureNameList;
    updateComboBox();
    return kNoError;
}


void MainWindow::closeEvent (QCloseEvent *event)
{
    if (m_tempFile.hasUnSavedData()) {
        QMessageBox::StandardButton resBtn = QMessageBox::question( this, APP_NAME,
                                                                    tr("The last session is not saved. Are you sure you want to Quit?\n"),
                                                                    QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::No);
        if (resBtn != QMessageBox::Yes) {
            event->ignore();
        } else {
            m_tempFile.destroy();
            event->accept();
        }
    }
}

void MainWindow::on_recordBtn_clicked()
{
    auto hasUnSavedData = m_tempFile.hasUnSavedData();

    if(ui->recordBtn->isChecked()) {
        if (hasUnSavedData) {
            auto reply = QMessageBox::question(this, "New Session",
                                          "The previous session is not yet saved. Do you still wish to save it before continuing?",
                                          QMessageBox::Save | QMessageBox::No,
                                          QMessageBox::Save);
            if (reply == QMessageBox::Save) {
                on_actionSave_As_triggered();
            } else {
                m_tempFile.destroy();
            }
        }

        m_tempFile.create(EMGDataSource::iBlockLength, EMGDataSource::iTotalChannels);
        setRecording(true);

    } else {
        setRecording(false);
    }

    ui->actionSave->setEnabled(hasUnSavedData);
    ui->actionSave_As->setEnabled(hasUnSavedData);
}

void MainWindow::on_actionAmplitude_triggered(bool checked)
{
    toggleAxisVisibilty(YAxis, checked);
}

void MainWindow::on_actionTime_triggered(bool checked)
{
    toggleAxisVisibilty(XAxis, checked);
}

void MainWindow::on_connectBtn_clicked()
{
    auto err = m_pEMGDataSrc->sshStartServer(m_host.toStdString(), m_iPort);
    if (err != kNoError) {
        updateStatusBarMessage("Couldn't start server. Recheck hostname and try again...");
    }
}

void MainWindow::currentIndexChanged(int index) {
    ui->recordBtn->setEnabled(false);
    if (m_gestureNameList.isEmpty() || index <= 0)
        return;


    ui->recordBtn->setEnabled(m_bIsConnected);
    m_iCurrentSessionGesture = index;
    m_pEMGDataSrc->setCurrentSessionGesture(m_iCurrentSessionGesture);
}

void MainWindow::on_actionSave_triggered()
{
    auto fileName = m_gestureNameList[m_iCurrentSessionGesture] + "_" + QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate) + ".txt";
    qDebug() << m_defaultSavePath + fileName;
    auto err = kNoError;
    if (m_tempFile.isInitialized()) {
        if (!QDir(m_defaultSavePath).exists())
            QDir().mkpath(m_defaultSavePath);
        err = m_tempFile.copyToFile(m_defaultSavePath + fileName);
    }

    if (err == kNoError)
        ui->actionSave->setEnabled(false);
}

void MainWindow::on_actionSave_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Session"), "",
            tr("EMG Data (*.txt);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    auto err = kNoError;
    err = m_tempFile.copyToFile(fileName);
    if (err == kNoError)
        ui->actionSave_As->setEnabled(false);
}

void MainWindow::on_actionPreferences_triggered(bool)
{
    Preferences::pref_t pref;
    pref.host = m_host;
    pref.iPort = m_iPort;
    pref.iBlockSize = m_iTimeSize;
    pref.iSensorGain = m_pEMGDataSrc->getSensorGain();
    pref.defaultSavePath = m_defaultSavePath;
    pref.iRecordingLength = EMGDataSource::iNumSamplesToRecord / EMGDataSource::iSampleRate;
    pref.serverPath = SshSession::SERVER_PATH;
    pref.annotationMapJSONFilePath = m_annotationMapJSONFilePath;

    Preferences prefDialog(&pref);
    prefDialog.setModal(true);
    auto ok = prefDialog.exec();

    if (ok) {
        pref = prefDialog.getPref();
        m_host = pref.host;
        m_iPort = pref.iPort;
        m_iTimeSize = pref.iBlockSize;
        m_defaultSavePath = pref.defaultSavePath;
        if (pref.iSensorGain != m_pEMGDataSrc->getSensorGain())
            m_pEMGDataSrc->setSensorGain(pref.iSensorGain);
        EMGDataSource::iNumSamplesToRecord = pref.iRecordingLength * EMGDataSource::iSampleRate;
        SshSession::SERVER_PATH = pref.serverPath;

        if (m_annotationMapJSONFilePath != pref.annotationMapJSONFilePath) {
            m_annotationMapJSONFilePath = pref.annotationMapJSONFilePath;
            parseAnnotationMap();
        }
    }
}
