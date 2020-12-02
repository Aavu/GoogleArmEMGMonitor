#include "pch.h"
#include "Server.h"
#include "CoAmp.h"
#include "KeyLogger.h"

#define APP "EMGDataServer"

int main() {
    std::string lockFile = std::string("/tmp/") + APP + "_lockFile";
    int pid_file = open(lockFile.c_str(), O_CREAT | O_RDWR, 0666);
    auto rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc != 0) {
        std::cerr << "Server already running...\n";
        return 1;
    }

    auto pid = getpid();
    write(pid_file, &pid, sizeof(pid_t));

    Logger::init(Logger::info);

    // Key logger
    auto* pKeyLogger = new KeyLogger();

    // EMG Sensors
    CoAmp::SensorParam_t sensorParam;
    sensorParam.iWindowLength = 256;
    sensorParam.iHopLength = 256;
    sensorParam.iGain = 8;
    CoAmp* pSensor = CoAmp::getInstance();
    auto err = pSensor->init(sensorParam, pKeyLogger);
    if (err != kNoError) {
        delete pSensor;
        flock(pid_file, LOCK_UN);
        close(pid_file);
        return 1;
    }

    Server* pServer = Server::getInstance();
    pServer->init(pSensor, 8080, 128);

    pServer->run(-1);

    delete pServer;
    delete pSensor;
    delete pKeyLogger;

    flock(pid_file, LOCK_UN);
    close(pid_file);
    return 0;
}
