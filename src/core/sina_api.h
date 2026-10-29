#ifndef SINA_API_H
#define SINA_API_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QByteArray>
#include "stockdata.h"

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

// ============================================================
// 新浪行情数据接口
// 通过新浪财经HTTP接口获取实时行情和历史K线数据
// ============================================================

class SinaAPI : public QObject {
    Q_OBJECT

public:
    explicit SinaAPI(QObject* parent = nullptr);
    ~SinaAPI();

    // 请求实时行情（批量）
    void requestQuotes(const QStringList& codes);            // 请求多只股票实时行情

    // 请求历史K线数据
    void requestKLines(const QString& code,                  // 股票代码
                        const QString& period = "day",       // 周期: day/week/month/60/30/15/5
                        int count = 200);                    // 数量

    // 请求分时数据
    void requestTimeSharing(const QString& code);            // 请求当日分时图数据

signals:
    // 行情数据到达信号
    void quotesReceived(const QList<StockQuote>& quotes);    // 批量行情数据
    void klinesReceived(const QString& code,                 // K线数据
                        const QList<KLine>& klines);
    void timeSharingReceived(const QString& code,            // 分时数据
                             const QList<TimeSharing>& data);
    void errorOccurred(const QString& msg);                  // 错误信号

private slots:
    void onQuoteReply(QNetworkReply* reply);                 // 行情回复处理
    void onKLineReply(QNetworkReply* reply);                 // K线回复处理
    void onTimeSharingReply(QNetworkReply* reply);           // 分时回复处理

private:
    QNetworkAccessManager* m_nam;                            // 网络管理器
    QString m_pendingCode;                                   // 当前请求的代码（K线用）

    // 解析新浪行情数据格式
    QList<StockQuote> parseQuoteData(const QByteArray& data); // 解析实时行情
    QList<KLine> parseKLineData(const QByteArray& data);     // 解析K线数据
    QList<TimeSharing> parseTimeSharingData(const QByteArray& data); // 解析分时数据

    // �0C构造新浪行情URL
    QString buildQuoteUrl(const QStringList& codes);         // 构造行情请求URL
    QString buildKLineUrl(const QString& code,              // 构造K线请求URL
                          const QString& period, int count);
};

#endif // SINA_API_H
