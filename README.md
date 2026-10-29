# QtStock - 开源版看盘软件

> 基于Qt 5.14.2开发的开源股票看盘软件。

## 功能规划

| Phase | 功能 | 状态 |
|-------|------|------|
| Phase 1 | 行情显示（自选股列表+实时行情+K线图） | ✅ 已完成 |
| Phase 2 | 技术指标（MA/MACD/KDJ/RSI/BOLL等） | ✅ 已完成 |

## Phase 1 已实现功能
- ✅ 自选股列表（15只默认股票）
- ✅ 实时行情显示（3秒自动刷新）
- ✅ 涨红跌绿颜色显示
- ✅ K线图（双击股票加载日K线）

## 构建方法
双击 `build.bat` 即可自动编译。

环境要求：
- Qt 5.14.2 (mingw73_32)
- MinGW 7.3 32bit
- CMake 3.14+

## 项目结构
```
QtStock/
├── CMakeLists.txt          # CMake构建文件
├── build.bat               # Windows构建脚本
├── src/
│   ├── main.cpp            # 程序入口
│   ├── core/               # 核心数据层
│   │   ├── stockdata.h     # 数据结构定义
│   │   ├── sina_api.h/cpp  # 新浪行情接口
│   │   └── dataengine.h/cpp # 数据引擎
│   ├── view/               # UI层
│   │   ├── mainwindow.h/cpp # 主窗口
│   │   ├── stock_list.h/cpp # 自选股列表
│   │   └── kline_view.h/cpp # K线图视图
│   ├── indicator/          # 技术指标(Phase 2)
│   └── formula/            # 公式引擎(Phase 3)
└── data/                   # 数据目录
```

## 技术栈
- C++17 + Qt 5.14.2
- Qt Charts (K线图绘制)
- Qt Network (行情数据请求)
- 新浪财经HTTP接口 (数据源)