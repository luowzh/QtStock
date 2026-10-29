// ============================================================
// QtStock
// 主程序入口
// ============================================================

#include <QApplication>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <QMessageLogContext>
#include <QDebug>
#include <QIODevice>
#include "view/mainwindow.h"

// 全局日志文件指针，供消息处理器使用
static QFile* g_logFile = nullptr;                            // 全局日志文件指针

// 自定义Qt消息处理器：将所有Qt消息（含插件加载警告）写入日志文件
// 作用：GUI程序（WIN32_EXECUTABLE）无控制台输出，崩溃时无法看到Qt警告，
//       通过日志文件记录，便于定位启动失败原因（如platforms插件缺失）
static void myMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if (!g_logFile || !g_logFile->isOpen()) return;           // 日志文件未就绪则跳过
    QString level;                                            // 日志级别字符串
    switch (type) {
    case QtDebugMsg:    level = "DEBUG";   break;            // 调试信息
    case QtInfoMsg:     level = "INFO";    break;            // 一般信息
    case QtWarningMsg:  level = "WARN";    break;            // 警告信息
    case QtCriticalMsg: level = "CRIT";    break;            // 严重错误
    case QtFatalMsg:    level = "FATAL";   break;            // 致命错误
    }
    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"); // 时间戳
    QTextStream out(g_logFile);                               // 绑定日志文件的输出流
    out << QString("[%1] [%2] %3\n").arg(ts).arg(level).arg(msg); // 写入一行日志
    out.flush();                                              // 立即刷新到磁盘
}

int main(int argc, char* argv[]) {
    // 第一步：在QApplication之前打开日志文件
    // 用argv[0]定位exe所在目录（此时QCoreApplication尚未创建，applicationDirPath不可用）
    QString exeDir = QFileInfo(QString::fromLocal8Bit(argv[0])).absolutePath(); // exe绝对路径目录
    g_logFile = new QFile(exeDir + "/qtstock.log");           // 创建日志文件对象(qtstock.log)
    g_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text); // 追加模式打开

    // 第二步：在QApplication之前安装消息处理器
    // 这样可以捕获QApplication构造时platforms插件加载失败的警告
    qInstallMessageHandler(myMessageHandler);                 // 安装自定义消息处理器

    qInfo() << "=== QtStock 启动 ===";                       // 记录启动标记
    qInfo() << "exeDir:" << exeDir;                           // 记录exe目录路径

    // 第三步：创建Qt应用（此处可能触发platforms插件加载）
    QApplication app(argc, argv);                            // 创建Qt应用
    qInfo() << "QApplication created, Qt version:" << qVersion(); // 记录Qt版本

    // 设置应用信息
    app.setApplicationName("QtStock");                        // 应用名称
    app.setApplicationVersion("1.0");                         // 版本号
    app.setOrganizationName("QtStock");                       // 组织名

    // 设置默认字体（中文显示更清晰）
    QFont font("Microsoft YaHei", 9);                         // 微软雅黑9pt
    app.setFont(font);                                        // 设置全局字体

    // 第四步：创建并显示主窗口
    qInfo() << "Creating MainWindow...";                      // 记录开始创建窗口
    MainWindow window;                                        // 创建主窗口
    window.show();                                            // 显示窗口
    qInfo() << "MainWindow shown, entering event loop...";    // 记录窗口已显示

    // 第五步：进入事件循环
    int ret = app.exec();                                     // 进入Qt事件循环
    qInfo() << "=== QtStock 退出, code:" << ret << "===";    // 记录退出码

    // 关闭并释放日志文件
    if (g_logFile) {                                          // 日志文件存在
        g_logFile->close();                                   // 关闭日志文件
        delete g_logFile;                                     // 释放日志文件对象
        g_logFile = nullptr;                                  // 置空指针
    }
    return ret;                                               // 返回退出码
}
