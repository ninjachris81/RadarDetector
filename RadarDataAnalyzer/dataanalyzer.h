#ifndef DATAANALYZER_H
#define DATAANALYZER_H

#include <QObject>
#include <QMqttClient>
#include <QSqlDatabase>
#include <QtHttpServer>

#define MQTT_BASE_PATH "radar"


// TYPES
#define MQTT_ID_DOUBLE 'd'
#define MQTT_ID_INTEGER 'i'
#define MQTT_ID_STRING 's'
#define MQTT_ID_BOOL 'b'

#define MQTT_MIN_MSG_SIZE 2

#define MQTT_PATH_SEP "/"
#define MQTT_WILDCARD "#"
#define MQTT_VAL "val"
#define MQTT_SET "set"
#define MQTT_TRE "tre"
#define MQTT_LOG_SOURCE_DIV "> "


class DataAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit DataAnalyzer(QHttpServer *httpServer, QObject *parent = nullptr);

    static QString DB_HOST;
    static QString DATABASE_NAME;
    static QString TABLE_NAME;
    static QString DATABASE_USERNAME;
    static QString DATABASE_PASSWORD;

    enum MQTT_MODE {
        MQTT_MODE_NONE,
        MQTT_MODE_VAL,
        MQTT_MODE_SET,
        MQTT_MODE_TRE
    };

    static QVariant parsePayload(QByteArray payload);

private slots:
    void _onMqttConnected();
    void _onMqttDisconnected();

    void _onMqttStateChanged(QMqttClient::ClientState state);
    void _onMqttError(QMqttClient::ClientError error);

    void _onMqttValueReceived(QMqttMessage msg);

private:
    QHttpServer *m_httpServer;

    QMqttClient m_mqttClient;

    QMqttSubscription* m_sub;

    QSqlDatabase m_db;

    static QStringList buildPath(QStringList paths, MQTT_MODE mode = MQTT_MODE_NONE, int index = -1, bool addWildcard = false);

    QString getContent(int limit);

signals:

public slots:
};

#endif // DATAANALYZER_H
