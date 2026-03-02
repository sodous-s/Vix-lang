![Vix logo](images/README/1770378110202.png)

# Vix Programming Language

[![Self-hosting Progress](https://img.shields.io/badge/Self--hosting-90%25-orange)]()
[![Backends](https://img.shields.io/badge/Backends-LLVM%20%7C%20QBE%20%7C%20C++-brightgreen)]()
[![License](https://img.shields.io/badge/License-MIT-green)]()

Vix is a lightweight compiled language designed to provide **near-native C++ execution speed** while maintaining the simplicity and ease of use of scripting languages.

[🇨🇳 中文版](README.md) | [🌟 Quick Start](#quick-start) | [📖 Documentation](Docs/README.md) | [🧩 VS Code Extension](#) | [🤝 Contributing](#contributing)

### 🌐 Cross-platform + Multi-architecture

- **Operating Systems**: Windows, Linux, macOS
- **Processor Architectures**: x86, ARM, RISC-V

## 🚀 Quick Start

### Build Vix

```shell
cd src && make
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

Here are a few simple examples to give you a quick feel for Vix's syntax:

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
    print("Sum from 1 to 100: ", sum)
}
```

For more examples, check the [examples](examples) directory.

## 📖 Documentation

- [Language Reference](Docs/README.md) - Syntax, type system, built-in functions, etc.
- [Compiler Guide](Docs/compiler.md) - Command-line options, configuration, etc.
- [Backend Comparison](Docs/backends.md) - Detailed comparison of LLVM/QBE/C++ backends
- [FAQ](Docs/faq.md)

## 🤝 Contributing

We welcome all forms of contribution! Including but not limited to:

- 💡 Suggesting syntax improvements
- 📝 Writing documentation
- 🐛 Reporting bugs
- 🔧 Submitting code
- 📦 Developing the package manager (VPM)
- 📚 Enhancing the standard library

Please read the [Contribution Guidelines](Docs/CONTRIBUTING.md) to get started.

## 🗺️ Project Ecosystem

Vix is gradually building its own ecosystem:

| Project            | Description                         | Status            |
| ------------------ | ----------------------------------- | ----------------- |
| **Vix Compiler**   | Core compiler (LLVM/QBE/C++ backends) | In development, self-hosting soon |
| **VPM**            | Vix Package Manager                 | Community contribution |
| **Standard Library** | Common data structures and functions | Community contribution |
| **VS Code Extension** | Editor support                    | Released          |

## 📄 License

This project is open-sourced under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📬 Contact

- Email: [popolk1871@outlook.com](mailto:popolk1871@outlook.com)
- GitHub Issues: Submit directly in this repository
- 💬 [QQ Group: 130577506](https://qm.qq.com/cgi-bin/qm/qr?k=130577506) (Join us to chat about Vix)

**If you're interested in Vix, feel free to star, fork, open an issue, or try it out right away!**

> Don't hesitate! Do it now!