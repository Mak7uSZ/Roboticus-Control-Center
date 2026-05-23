#include "controllers/AppController.h"

AppController::AppController(QObject *parent) : QObject(parent) {
  m_portManager = new SerialPortManager(this);
  m_parser = new SerialParser(this);
  m_udpConnection = new UDPConnection(this);

  // Route raw data from Serial Port -> Parser
  connect(m_portManager, &SerialPortManager::rawDataReceived, m_parser,
          &SerialParser::onRawDataReady);

  // Route decoded frames from Parser -> Controller
  connect(m_parser, &SerialParser::frameDecoded, this,
          &AppController::onFrameParsed);

  // Clear snapshot list when port connection is started
  connect(m_portManager, &SerialPortManager::connectionChanged, this,
          &AppController::onPortConnectionChanged);

  // Forward errors to UI
  connect(m_portManager, &SerialPortManager::errorOccurred, this,
          &AppController::errorOccurred);

  connect(m_udpConnection, &UDPConnection::errorOccurred, this,
          &AppController::errorOccurred);
}

void AppController::setModels(SensorModel *sensorModel,
                              VectorModel *vectorModel) {
  m_sensorModel = sensorModel;
  m_vectorModel = vectorModel;
}

void AppController::switchToWiredMode() {
  if (m_udpConnection) {
    m_udpConnection->stopListening();
  }

  if (m_parser) {
    m_parser->reset();
  }

  if (m_connectionMode == QStringLiteral("wired")) {
    return;
  }

  m_connectionMode = QStringLiteral("wired");
  emit connectionModeChanged();
}

void AppController::switchToWirelessMode() {
  if (m_portManager && m_portManager->isConnected()) {
    m_portManager->disconnectPort();
  }

  if (m_parser) {
    m_parser->reset();
  }

  if (m_connectionMode == QStringLiteral("wireless")) {
    return;
  }

  m_connectionMode = QStringLiteral("wireless");
  emit connectionModeChanged();
}

bool AppController::startWirelessMonitor(int port) {
  if (port <= 0 || port > 65535) {
    emit errorOccurred("UDP port must be between 1 and 65535.");
    return false;
  }

  if (m_connectionMode != QStringLiteral("wireless")) {
    switchToWirelessMode();
  }

  if (m_portManager && m_portManager->isConnected()) {
    m_portManager->disconnectPort();
  }

  if (m_parser) {
    m_parser->reset();
  }

  if (!m_udpConnection) {
    return false;
  }

  m_udpConnection->clearStatistics();
  return m_udpConnection->startListening(static_cast<quint16>(port));
}

void AppController::stopWirelessMonitor() {
  if (m_udpConnection) {
    m_udpConnection->stopListening();
  }

  if (m_parser) {
    m_parser->reset();
  }
}

void AppController::reportConnectionError(const QString &message) {
  if (!message.isEmpty()) {
    emit errorOccurred(message);
  }
}

void AppController::onFrameParsed(const DecodedFrame &frame) {
  // 1. update UI models
  updateModelsFromFrame(frame);

  // 2. Convert Frame to Snapshot
  FrameSnapshot snapshot;
  snapshot.timestamp = frame.timestamp;
  snapshot.sensors =
      m_sensorModel ? m_sensorModel->getAllSensors() : QList<Sensor>();
  snapshot.vectors =
      m_vectorModel ? m_vectorModel->getAllVectors() : QList<Vector>();

  // 3. Store it
  m_snapshotStore.append(snapshot);
  emit snapshotsChanged();
}

void AppController::updateModelsFromFrame(const DecodedFrame &frame) {
  if (!m_sensorModel || !m_vectorModel)
    return;

  updateSensorsFromFrame(frame.sensors);
  updateVectorsFromFrame(frame.vectors);
}

void AppController::updateSensorsFromFrame(const QVariantList &sensors) {
  if (!m_sensorModel)
    return;

  for (const QVariant &sensorObject : sensors) {
    // [0]=name [1]=input [2]=isTriggered [3]=threshold [4]=layer [5]=x [6]=y
    QVariantList s = sensorObject.toList();
    if (s.size() < 7) { // 7 elements expected for a valid sensor entry
      qDebug() << "Malformed sensor entry, skipping";
      continue;
    }

    QString name = s.at(0).toString();
    double input = roundTo2Decimals(s.at(1).toFloat());
    bool isTriggered = s.at(2).toBool();
    double threshold = roundTo2Decimals(s.at(3).toFloat());
    QString layer = s.at(4).toString();
    double x = roundTo2Decimals(s.at(5).toFloat());
    double y = roundTo2Decimals(s.at(6).toFloat());

    int idx = m_sensorModel->getIndexByName(name);
    if (idx >= 0) {
      // Update existing sensor in the model
      QModelIndex modelIdx = m_sensorModel->index(idx);
      m_sensorModel->setData(modelIdx, input, SensorModel::InputRole);
      m_sensorModel->setData(modelIdx, threshold, SensorModel::ThresholdRole);
      m_sensorModel->setData(modelIdx, isTriggered, SensorModel::TriggerRole);
      m_sensorModel->setData(modelIdx, x, SensorModel::XRole);
      m_sensorModel->setData(modelIdx, y, SensorModel::YRole);
      m_sensorModel->setData(modelIdx, layer, SensorModel::LayerRole);
    } else {
      // Add new sensor to the model
      m_sensorModel->addSensor(name, input, threshold, isTriggered, layer, x,
                               y);
    }
  }
}

void AppController::updateVectorsFromFrame(const QVariantList &vectors) {
  if (!m_vectorModel)
    return;

  for (const QVariant &vectorObject : vectors) {
    // [0]=name [1]=rotation [2]=color [3]=layer [4]=x [5]=y
    QVariantList v = vectorObject.toList();
    if (v.size() < 6) { // 6 elements expected for a valid vector entry
      qDebug() << "Malformed vector entry, skipping";
      continue;
    }

    QString name = v.at(0).toString();
    double rotation = roundTo2Decimals(v.at(1).toFloat());
    QString color = v.at(2).toString();
    QString layer = v.at(3).toString();
    double x = roundTo2Decimals(v.at(4).toFloat());
    double y = roundTo2Decimals(v.at(5).toFloat());

    int idx = m_vectorModel->getIndexByName(name);
    if (idx >= 0) {
      // Update existing vector in the model
      QModelIndex modelIdx = m_vectorModel->index(idx);
      m_vectorModel->setData(modelIdx, rotation, VectorModel::RotationRole);
      m_vectorModel->setData(modelIdx, QColor(color), VectorModel::ColorRole);
      m_vectorModel->setData(modelIdx, layer, VectorModel::LayerRole);
      m_vectorModel->setData(modelIdx, x, VectorModel::XRole);
      m_vectorModel->setData(modelIdx, y, VectorModel::YRole);
    } else {
      // Add new vector to the model
      m_vectorModel->addVector(name, rotation, 1.0, QColor(color), layer, x, y);
    }
  }
}

bool AppController::saveToFile(QUrl filePath) {
  QString errorMessage;
  if (!m_snapshotLoader.save(filePath, m_snapshotStore.all(), &errorMessage)) {
    emit errorOccurred(errorMessage);
    return false;
  }
  return true;
}

bool AppController::loadFromFile(QUrl filePath) {
  QList<FrameSnapshot> loadedSnapshots;
  QString errorMessage;
  if (!m_snapshotLoader.load(filePath, &loadedSnapshots, &errorMessage)) {
    emit errorOccurred(errorMessage);
    return false;
  }

  m_snapshotStore.clear();
  for (const auto &snapshot : loadedSnapshots) {
    m_snapshotStore.append(snapshot);
  }

  emit snapshotsChanged();
  return true;
}

bool AppController::restoreToIndex(int index) {
  if (!m_snapshotStore.isValidIndex(index))
    return false;

  const FrameSnapshot snapshot = m_snapshotStore.at(index);

  if (m_sensorModel) {
    m_sensorModel->clear();
    for (const auto &s : snapshot.sensors) {
      m_sensorModel->addSensor(s.name, s.inputValue, s.threshold, s.isTriggered,
                               s.layer, s.x, s.y);
    }
  }

  if (m_vectorModel) {
    m_vectorModel->clear();
    for (const auto &v : snapshot.vectors) {
      m_vectorModel->addVector(v.name, v.rotation, v.scale, v.color, v.layer,
                               v.x, v.y);
    }
  }

  return true;
}

void AppController::onPortConnectionChanged() {
  if (m_portManager && m_portManager->isConnected()) {
    m_snapshotStore.clear();
    emit snapshotsChanged();
  }
}

QList<qint64> AppController::availableTimestamps() const {
  return m_snapshotStore.availableTimestamps();
}

qint64 AppController::timestampAt(int index) const {
  return m_snapshotStore.timestampAt(index);
}
