#include "csv_loader.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QIODevice>

// ============================================================
// CSV文件加载器实现
// ============================================================

// 从CSV文件加载股票代码
QStringList CsvLoader::loadStocks(const QString& filePath) {
    QStringList codes;                                         // 结果列表

    if (!exists(filePath)) {                                   // 文件不存在
        return codes;                                          // 返回空列表
    }

    QFile file(filePath);                                      // 打开文件
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {  // 只读文本模式
        return codes;                                          // 打开失败返回空
    }

    QTextStream in(&file);                                     // 文本流
    in.setCodec("UTF-8");                                      // UTF-8编码

    while (!in.atEnd()) {                                      // 逐行读取
        QString line = in.readLine().trimmed();                // 读取一行并去空白
        if (line.isEmpty()) continue;                          // 空行跳过
        if (line.startsWith("#")) continue;                    // 注释行跳过
        if (line.startsWith("//")) continue;                   // 注释行跳过
        codes.append(line);                                    // 添加到列表
    }

    file.close();                                              // 关闭文件
    return codes;                                              // 返回代码列表
}

// 保存股票代码列表到CSV文件
bool CsvLoader::saveStocks(const QString& filePath,
                           const QStringList& codes) {
    QFile file(filePath);                                      // 打开文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) { // 只写文本模式
        return false;                                          // 打开失败
    }

    QTextStream out(&file);                                    // 文本流
    out.setCodec("UTF-8");                                     // UTF-8编码
    out << "# QtStock 自选股列表" << endl;                      // 写入注释头
    out << "# 每行一个股票代码，支持以下格式:" << endl;          // 说明
    out << "#   sh600519 (沪市，带前缀)" << endl;               // 说明
    out << "#   sz000001 (深市，带前缀)" << endl;               // 说明
    out << "#   600036   (沪市，不带前缀，自动识别)" << endl;    // 说明
    out << "#   000001   (深市，不带前缀，自动识别)" << endl;    // 说明
    out << endl;                                               // 空行

    for (const QString& code : codes) {                        // 遍历代码列表
        out << code << endl;                                   // 每行写一个代码
    }

    file.close();                                              // 关闭文件
    return true;                                               // 保存成功
}

// 检查CSV文件是否存在
bool CsvLoader::exists(const QString& filePath) {
    return QFileInfo::exists(filePath);                        // 检查文件是否存在
}
