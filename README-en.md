# Vix Programming Language

[![Self-hosted](https://img.shields.io/badge/self--hosted-90%25-orange)]()
[![Backends](https://img.shields.io/badge/backends-LLVM%20%7C%20QBE%20%7C%20C++-brightgreen)]()
[![VS Code Extension](https://img.shields.io/badge/VS%20Code-extension-purple)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

Vix is a lightweight compiled language designed to deliver **near-native C++ performance** while maintaining the simplicity and ease of use of a scripting language.

[🌟 Quick Start](#quick-start) | [📖 Documentation](Docs/README.md) | [🧩 VS Code Extension](#) | [🤝 Contribute](#contributing)

## ✨ Key Highlights

### ⚙️ Three-Backend Architecture (Rare in any language)

Vix is one of the very few compiled languages that **supports three backends simultaneously**, giving you the freedom to choose based on your scenario:

| Backend | Characteristics | Best For |
|---------|-----------------|----------|
| **LLVM** | Industrial-grade optimization, high-quality code | Production, performance-sensitive applications |
| **QBE** | Minimalist, lightweight, fast compilation | Development, debugging, learning |
| **C++** | Generates readable C++ code | Manual intervention, porting to special environments |

> Implementing three backends (LLVM, QBE, and C++) is one of Vix's most hardcore technical features.

### 🚀 Approaching Self-Hosting
The next version of the compiler will be able to compile itself — a major milestone in any language's journey to maturity.

### 🧩 VS Code Extension
Provides syntax highlighting and error reporting, ready to use out of the box for a smoother development experience.

### 🌐 Cross-Platform + Multi-Architecture
- **OS Support**: Windows, Linux, macOS
- **Architecture Support**: x86, ARM, RISC-V

## 🚀 Quick Start

### 1. Install Dependencies

```shell
apt install gcc g++ flex bison llvm clang-18 # Ubuntu/Debian
yum install gcc gcc-c++ flex bison llvm clang-18 # CentOS/RHEL
brew install flex bison llvm clang-18 # macOS
pacman -S flex bison g++ gcc llvm clang-18 # Arch Linux
```

### 2. Build Vix

```shell
make
```

### 3. Verify Installation

```shell
vixc -v
```

### 4. Your First Vix Program

Create a `hello.vix` file:

```vix
fn main() -> i32 {
    print("Hello, Vix!")
}
```

Compile and run:

```shell
vixc hello.vix -o hello
./hello
```

## 📚 Example Preview

Here are a few simple examples to give you a feel for Vix's syntax:

### Fibonacci Sequence

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

### Variables and Loops

```vix
fn main() -> i32 {
    sum = 0
    for (i in 1 .. 100) {
        sum = sum + i
    }
    print("Sum from 1 to 100:", sum)
}
```

Check out the [examples](examples) directory for more.

## 📖 Documentation

- [Language Reference](Docs/README.md) - Syntax, type system, built-in functions, etc.
- [Compiler Guide](Docs/compiler.md) - Command-line options, configuration, etc.
- [Backend Comparison](Docs/backends.md) - Detailed comparison of LLVM/QBE/C++ backends
- [FAQ](Docs/faq.md)

## 🧩 VS Code Extension

To make Vix development more enjoyable, we provide an official VS Code extension:

- ✅ Syntax highlighting
- ✅ Error reporting
- ✅ Code snippets
- ✅ Build task integration

## 🤝 Contributing

We welcome all forms of contribution, including but not limited to:

- 💡 Syntax suggestions
- 📝 Documentation writing
- 🐛 Bug reports
- 🔧 Code contributions
- 📦 Package manager development (VPM)
- 📚 Standard library improvements

Please read our [Contributing Guide](Docs/CONTRIBUTING.md) to get started.

## 🗺️ Project Ecosystem

Vix is gradually building its ecosystem:

| Project | Description | Status |
|---------|-------------|--------|
| **Vix Compiler** | Core compiler (LLVM/QBE/C++ backends) | In development, approaching self-hosting |
| **VPM** | Vix Package Manager | Community contributions welcome |
| **Standard Library** | Common data structures and functions | Community contributions welcome |
| **VS Code Extension** | Editor support | Released |

## 📄 License

This project is open-sourced under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📬 Contact

- Email: [popolk1871@outlook.com](mailto:popolk1871@outlook.com)
- GitHub Issues: Open an issue directly in this repository
- 💬 [QQ Group: 130577506](https://qm.qq.com/cgi-bin/qm/qr?k=130577506) (Join the conversation — Chinese-speaking community)

**If you're interested in Vix, feel free to star, fork, open issues, or just give it a try!**

> Don't hesitate. Start now!