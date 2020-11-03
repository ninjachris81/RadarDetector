#include "dataanalyzer.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

QString DataAnalyzer::DB_HOST = "rpi-server.fritz.box";
QString DataAnalyzer::DATABASE_NAME = "radar_data";
QString DataAnalyzer::TABLE_NAME = "radar_data";
QString DataAnalyzer::DATABASE_USERNAME = "radar";
QString DataAnalyzer::DATABASE_PASSWORD = "radar";

DataAnalyzer::DataAnalyzer(QHttpServer *httpServer, QObject *parent) : QObject(parent), m_httpServer(httpServer)
{
    qDebug() << Q_FUNC_INFO;

    m_mqttClient.setHostname("rpi-server.fritz.box");
    m_mqttClient.setPort(1883);

    connect(&m_mqttClient, &QMqttClient::errorChanged, this, &DataAnalyzer::_onMqttError);
    connect(&m_mqttClient, &QMqttClient::stateChanged, this, &DataAnalyzer::_onMqttStateChanged);

    connect(&m_mqttClient, &QMqttClient::connected, this, &DataAnalyzer::_onMqttConnected);
    connect(&m_mqttClient, &QMqttClient::disconnected, this, &DataAnalyzer::_onMqttDisconnected);

#ifdef WIN32
    m_db = QSqlDatabase::addDatabase("QODBC");
    m_db.setDatabaseName("DRIVER={PostgreSQL ODBC Driver(UNICODE)};DATABASE=" + DATABASE_NAME + ";SERVER=" + DB_HOST + ";PORT=5432");
#else
    m_db = QSqlDatabase::addDatabase("QPSQL");
    m_db.setDatabaseName(DATABASE_NAME);
#endif

    m_db.setHostName(DB_HOST);
    m_db.setPort(5432);
    m_db.setUserName(DATABASE_USERNAME);
    m_db.setPassword(DATABASE_PASSWORD);

    QSqlQuery query = QSqlQuery( m_db );
    if (m_db.open()) {
        query.exec( "CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " (ts DATETIME, val INTEGER)" );
        m_mqttClient.connectToHost();
    } else {
        qWarning() << query.lastError();
    }

    httpServer->route("/data/", [this](const int limit) {
        return getContent(limit);
    });

    const auto port = httpServer->listen(QHostAddress::Any, 9090);
    if (!port) {
        qDebug() << "Server started";
    }
}

void DataAnalyzer::_onMqttStateChanged(QMqttClient::ClientState state) {
    qDebug() << Q_FUNC_INFO << state;
}

void DataAnalyzer::_onMqttError(QMqttClient::ClientError error) {
    qWarning() << Q_FUNC_INFO << error;
}

void DataAnalyzer::_onMqttConnected() {
    qDebug() << Q_FUNC_INFO;

    QString s = buildPath(QStringList() << "radar", MQTT_MODE_VAL, 0).join(MQTT_PATH_SEP);
    qDebug() << s;

    m_sub = m_mqttClient.subscribe(QMqttTopicFilter(s));
    connect(m_sub, &QMqttSubscription::messageReceived, this, &DataAnalyzer::_onMqttValueReceived);

}

void DataAnalyzer::_onMqttDisconnected() {
    qDebug() << Q_FUNC_INFO;

}

QStringList DataAnalyzer::buildPath(QStringList paths, MQTT_MODE mode, int index, bool addWildcard) {
    //qDebug() << Q_FUNC_INFO << paths;

    paths.prepend(MQTT_BASE_PATH);
    switch(mode) {
    case MQTT_MODE_VAL:
        paths.append(MQTT_VAL);
        break;
    case MQTT_MODE_SET:
        paths.append(MQTT_SET);
        break;
    case MQTT_MODE_TRE:
        paths.append(MQTT_TRE);
        break;
    default:
        // nothing
        break;
    }

    if (addWildcard) {
        paths.append(MQTT_WILDCARD);
    } else if (index>-1){
        paths.append(QString::number(index));
    }
    return paths;
}

void DataAnalyzer::_onMqttValueReceived(QMqttMessage msg) {
    qDebug() << Q_FUNC_INFO << msg.topic() << msg.payload();

    int val = DataAnalyzer::parsePayload(msg.payload()).toInt();

    qDebug() << val;
    QSqlQuery query = QSqlQuery( m_db );
    query.prepare("INSERT INTO " + TABLE_NAME + " (ts, val) VALUES (:ts, :val)");

    query.bindValue(":ts", QDateTime::currentMSecsSinceEpoch());
    query.bindValue(":val", val);

    query.exec();
}

QVariant DataAnalyzer::parsePayload(QByteArray payload) {
    if (payload.count()>=MQTT_MIN_MSG_SIZE) {
        switch(payload.at(0)) {
        case MQTT_ID_DOUBLE:
            return QByteArray(payload.mid(1)).toDouble();
        case MQTT_ID_INTEGER:
            return QByteArray(payload.mid(1)).toInt();
        case MQTT_ID_STRING:
            return QString(QByteArray((payload.mid(1))));
        case MQTT_ID_BOOL:
            return QByteArray(payload.mid(1)).toInt()==1;
        }
    }

    return QVariant();
}

QString DataAnalyzer::getContent(int limitSec) {
    QJsonDocument doc;
    QJsonObject obj;

    QString timeLimit = " ts >= " + QString::number(QDateTime::currentMSecsSinceEpoch() - (limitSec * 1000));
    qDebug() << timeLimit << limitSec;

    QSqlQuery query1 = QSqlQuery (m_db);
    query1.exec("SELECT AVG(val) FROM " + TABLE_NAME + " WHERE " + timeLimit);

    if (query1.first()) {
        obj.insert("avg", query1.value(0).toDouble());
    }

    QSqlQuery query2 = QSqlQuery (m_db);
    query2.exec("SELECT ts, val FROM " + TABLE_NAME + " ORDER BY ts DESC LIMIT 1");

    if (query2.first()) {
        obj.insert("ts", query2.value(0).toLongLong());
        obj.insert("last", query2.value(1).toInt());
    }

    QSqlQuery query3 = QSqlQuery (m_db);
    query3.exec("SELECT MAX(val) FROM " + TABLE_NAME + " WHERE " + timeLimit);

    if (query3.first()) {
        obj.insert("max", query3.value(0).toInt());
    }

    QSqlQuery query4 = QSqlQuery (m_db);
    query4.exec("SELECT MIN(val) FROM " + TABLE_NAME + " WHERE " + timeLimit);

    if (query4.first()) {
        obj.insert("min", query4.value(0).toInt());
    }

    doc.setObject(obj);
    return doc.toJson();
}
