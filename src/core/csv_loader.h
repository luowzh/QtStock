#ifndef CSV_LOADER_H
#define CSV_LOADER_H

#include <QString>
#include <QStringList>

// ============================================================
// CSV文件加载器
// 从CSV文件读取股票代码列表
// 文件格式: 每行一个股票代码，如:
//   sh600519
//   sz000001
//   600036
// ============================================================

class CsvLoader {
public:
    // 从CSV文件加载股票代码列表
    // filePath: CSV文件路径
    // 返回: 股票代码列表，文件不存在或为空则返回空列表
    static QStringList loadStocks(const QString& filePath);   // 加载股票代码

    // 保存股票代码列表到CSV文件
    // filePath: CSV文件路径
    // codes: 股票代码列表
    // 返回: 保存成功返回true
    static bool saveStocks(const QString& filePath,           // 保存股票代码
                           const QStringList& codes);

    // 检查CSV文件是否存在
    static bool exists(const QString& filePath);              // 检查文件存在
};

#endif // CSV_LOADER_H
