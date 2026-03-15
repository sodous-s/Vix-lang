![Vix logo](images/README/1770378110202.png)

# Vix 编程语言

[![自举进度](https://img.shields.io/badge/自举-90%25-orange)]()
[![后端](https://img.shields.io/badge/后端-LLVM%20%7C%20QBE%20%7C%20C++-brightgreen)]()
[![许可证](https://img.shields.io/badge/许可证-MIT-green)]()

Vix 是一种轻量级编译型语言，旨在提供**接近原生 C++ 的执行速度**，同时保持脚本语言的简洁性和易用性。

[[English](README-en.md) ][🌟 快速开始](#快速开始) | [📖 文档](Docs/README.md) | [🧩 VS Code扩展](#) | [🤝 参与贡献](#参与贡献)


### 🌐 跨平台 + 多架构

- **操作系统**：Windows、Linux、macOS
- **处理器架构**：x86、ARM、RISC-V

## 🚀 快速开始


### 1. 编译 Vix

```shell
cd src && make
```

### 2. 验证安装

```shell
vixc -v
```

### 3. 第一个 Vix 程序

创建一个 `hello.vix` 文件：

```vix
fn main() -> i32 {
    print("Hello, Vix!")
}
```

编译并运行：

```shell
vixc hello.vix -o hello
./hello
```

## 📚 示例预览

这里有几个简单的例子，让你快速感受 Vix 的语法：

### 斐波那契数列

```vix
fn fib(n: i32) -> i32 {
    if (n <= 1) {
        return n
    }
    return fib(n-1) + fib(n-2)
}

fn main() -> i32 {
    print(fib(10))
}
```

### 变量和循环

```vix
fn main() -> i32 {
    sum = 0
    for (i in 1 .. 100) {
        sum = sum + i
    }
    print("1 to 100 ：", sum)
}
```

更多示例请查看 [examples](examples) 目录。

## 📖 文档

- [语言参考](Docs/README.md) - 语法、类型系统、内置函数等
- [编译器使用指南](Docs/compiler.md) - 命令行选项、配置等
- [后端对比](Docs/backends.md) - LLVM/QBE/C++ 后端的详细对比
- [常见问题](Docs/faq.md)

## 🤝 参与贡献

我们欢迎各种形式的贡献！包括但不限于：

- 💡 提出语法建议
- 📝 撰写文档
- 🐛 报告 bug
- 🔧 提交代码
- 📦 开发包管理工具（VPM）
- 📚 完善标准库

请阅读[贡献指南](Docs/CONTRIBUTING.md)开始。

## 🗺️ 项目生态

Vix 正在逐步构建自己的生态：


| 项目             | 描述                            | 状态             |
| ---------------- | ------------------------------- | ---------------- |
| **Vix 编译器**   | 核心编译器（LLVM/QBE/C++ 后端） | 开发中，即将自举 |
| **VPM**          | Vix 包管理器                    | 社区贡献中       |
| **标准库**       | 常用数据结构和函数              | 社区贡献中       |
| **VS Code 扩展** | 编辑器支持                      | 已发布           |

## 📄 许可证

本项目基于 MIT 许可证开源 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 📬 联系方式

- 邮箱：[popolk1871@outlook.com](mailto:popolk1871@outlook.com)
- GitHub Issues：直接在本仓库提交
- 💬 [QQ群：130577506](https://qm.qq.com/cgi-bin/qm/qr?k=130577506)（加入一起聊 Vix）

**如果你对 Vix 感兴趣，欢迎 star、fork、提 issue，或者直接上手试试！**

> 别犹豫！就现在!
