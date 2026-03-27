# 模块系统

Vix 的模块系统允许你将代码组织成可复用的模块。通过 `import` 和 `pub` 关键字，你可以轻松地共享和重用代码。

## 目录

- [导入模块](#导入模块)
- [导出函数](#导出函数)
- [标准库模块](#标准库模块)
- [创建自定义模块](#创建自定义模块)
- [模块组织](#模块组织)
- [最佳实践](#最佳实践)

---

## 导入模块

### 基本语法

使用 `import` 关键字导入模块：

```vix
import "path/to/module.vix"
```

### 导入标准库

```vix
import "std/io.vix"
import "std/arr.vix"
import "std/strings.vix"
import "std/os.vix"
import "std/mem.vix"
import "std/rand.vix"
```

### 使用导入的函数

```vix
import "std/io.vix"
import "std/strings.vix"

fn main() -> i32 {
    let s = "Hello, Vix!"
    
    // 使用导入的函数
    puts(s)
    
    let cmp = strcmp("abc", "def")
    print(cmp)
    
    return 0
}
```

---

## 导出函数

### pub 关键字

使用 `pub` 关键字标记需要导出的函数：

```vix
// math_utils.vix

// 公开函数 - 可以被其他模块导入使用
pub fn square(n: i32) -> i32 {
    return n * n
}

pub fn cube(n: i32) -> i32 {
    return n * n * n
}

pub fn power(base: i32, exp: i32) -> i32 {
    let result = 1
    for (i in 0 .. exp) {
        result *= base
    }
    return result
}

// 私有函数 - 仅在本模块内使用
fn helper(x: i32) -> i32 {
    return x * 2
}
```

### 导入自定义模块

```vix
// main.vix
import "math_utils.vix"

fn main() -> i32 {
    let sq = square(5)    // 25
    let cb = cube(3)      // 27
    let pw = power(2, 10) // 1024
    
    print(sq)
    print(cb)
    print(pw)
    
    // helper(10)  // 错误：helper 不是公开函数
    
    return 0
}
```

---

## 标准库模块

### std/io - 输入输出

```vix
import "std/io.vix"

fn main() -> i32 {
    // 输出
    puts("Hello, World!")
    
    // 文件操作
    let file = fopen("test.txt", "w")
    if (file == nil) {
        puts("Failed to open file")
        return 1
    }
    
    let data = "Hello, Vix!"
    fwrite(data, 1, data.length, file)
    fclose(file)
    
    return 0
}
```

**可用函数：**
| 函数 | 描述 |
|------|------|
| `puts(s)` | 打印字符串并换行 |
| `fopen(filename, mode)` | 打开文件 |
| `fclose(file)` | 关闭文件 |
| `fread(ptr, size, count, file)` | 读取文件 |
| `fwrite(ptr, size, count, file)` | 写入文件 |
| `panic(msg)` | 触发错误并停止程序 |

### std/arr - 数组操作

```vix
import "std/arr.vix"

fn main() -> i32 {
    let arr = [5, 2, 8, 1, 9, 10]
    
    // 排序数组
    sort(arr, arr.length)
    
    // 打印排序后的数组
    for (i in 0 .. arr.length) {
        printf("%d ", arr[i])
    }
    printf("\n")
    
    return 0
}
```

**可用函数：**
| 函数 | 描述 |
|------|------|
| `sort(arr, size)` | 冒泡排序数组 |

### std/strings - 字符串操作

```vix
import "std/strings.vix"

fn main() -> i32 {
    let str1 = "Hello"
    let str2 = "World"
    
    // 字符串比较
    let cmp = strcmp(str1, str2)
    if (cmp == 0) {
        print("Strings are equal")
    } elif (cmp < 0) {
        print("str1 < str2")
    } else {
        print("str1 > str2")
    }
    
    return 0
}
```

**可用函数：**
| 函数 | 描述 |
|------|------|
| `strcmp(a, b)` | 比较两个字符串 |

### std/os - 操作系统接口

```vix
import "std/os.vix"

fn main() -> i32 {
    // 执行系统命令
    system("echo Hello from shell")
    
    // 目录操作
    let dir = opendir(".")
    // ... 读取目录
    closedir(dir)
    
    return 0
}
```

**可用函数：**
| 函数 | 描述 |
|------|------|
| `system(cmd)` | 执行系统命令 |
| `exit(status)` | 退出程序 |
| `opendir(path)` | 打开目录 |
| `readdir(dir)` | 读取目录项 |
| `closedir(dir)` | 关闭目录 |

### std/mem - 内存管理

```vix
import "std/mem.vix"

fn main() -> i32 {
    // 分配内存
    let buffer = malloc(100)
    
    // 设置内存
    memset(buffer, 0, 100)
    
    // 使用内存...
    buffer[0] = 42
    
    // 释放内存
    free(buffer)
    
    return 0
}
```

**可用函数：**
| 函数 | 描述 |
|------|------|
| `malloc(size)` | 分配内存 |
| `free(ptr)` | 释放内存 |
| `memcpy(dest, src, n)` | 复制内存 |
| `memset(s, c, n)` | 设置内存 |
| `memcmp(s1, s2, n)` | 比较内存 |

### std/rand - 随机数

```vix
import "std/rand.vix"

fn main() -> i32 {
    // 生成 1 到 100 之间的随机数
    let r = rand(1, 100)
    print(r)
    
    return 0
}
```

**可用函数：**
| 函数 | 描述 |
|------|------|
| `rand(min, max)` | 生成指定范围内的随机数 |

### std/net - 网络编程

```vix
import "std/net/net.vix"

fn main() -> i32 {
    // 创建 socket
    let sock = socket(2, 1, 0)
    
    if (sock < 0) {
        printf("socket failed\n")
        return 1
    }
    
    // 监听连接
    if (listen(sock, 5) < 0) {
        printf("listen failed\n")
        return 1
    }
    
    printf("server running\n")
    
    // 处理客户端
    let response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Hello from Vix!</h1>"
    
    while (1) {
        let client = accept(sock, 0, 0)
        if (client >= 0) {
            printf("client connected\n")
            send(client, response, 100, 0)
            close(client)
        }
    }
    
    close(sock)
    return 0
}
```

---

## 创建自定义模块

### 模块文件结构

```
myproject/
├── main.vix
├── utils.vix
└── math/
    ├── basic.vix
    └── advanced.vix
```

### 创建工具模块

```vix
// utils.vix

pub fn clamp(value: i32, min: i32, max: i32) -> i32 {
    if (value < min) {
        return min
    }
    if (value > max) {
        return max
    }
    return value
}

pub fn abs(n: i32) -> i32 {
    if (n < 0) {
        return -n
    }
    return n
}

pub fn sign(n: i32) -> i32 {
    if (n > 0) {
        return 1
    }
    if (n < 0) {
        return -1
    }
    return 0
}
```

### 创建数学模块

```vix
// math/basic.vix

pub fn add(a: i32, b: i32) -> i32 {
    return a + b
}

pub fn subtract(a: i32, b: i32) -> i32 {
    return a - b
}

pub fn multiply(a: i32, b: i32) -> i32 {
    return a * b
}

pub fn divide(a: i32, b: i32) -> i32 {
    if (b == 0) {
        return 0
    }
    return a / b
}
```

```vix
// math/advanced.vix

pub fn power(base: i32, exp: i32) -> i32 {
    let result = 1
    for (i in 0 .. exp) {
        result *= base
    }
    return result
}

pub fn factorial(n: i32) -> i32 {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

pub fn fibonacci(n: i32) -> i32 {
    if (n <= 1) {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}
```

### 使用自定义模块

```vix
// main.vix
import "utils.vix"
import "math/basic.vix"
import "math/advanced.vix"

fn main() -> i32 {
    // 使用 utils
    let clamped = clamp(150, 0, 100)
    print(clamped)  // 100
    
    // 使用 math/basic
    let sum = add(10, 20)
    print(sum)      // 30
    
    // 使用 math/advanced
    let fact = factorial(5)
    print(fact)     // 120
    
    return 0
}
```

---

## 模块组织

### 项目结构建议

```
project/
├── src/
│   ├── main.vix       # 主程序入口
│   ├── app.vix        # 应用逻辑
│   └── modules/
│       ├── config.vix # 配置模块
│       ├── utils.vix  # 工具函数
│       └── types.vix  # 类型定义
├── lib/
│   └── mylib.vix      # 外部库
└── std/
    ├── io.vix
    └── strings.vix
```

### 模块命名规范

- 使用小写字母
- 使用下划线分隔单词
- 名称应有意义且简洁

```
string_utils.vix   # 好
StringUtils.vix    # 不推荐
su.vix             # 太短，不清晰
```

---

## 最佳实践

### 1. 单一职责

每个模块应该只负责一个功能领域：

```vix
// 好：专注于字符串操作
// string_utils.vix
pub fn trim(s: string) -> string { }
pub fn split(s: string, delim: string) -> [string] { }
pub fn join(arr: [string], delim: string) -> string { }

// 不好：混合多种不相关的功能
// utils.vix
pub fn trim(s: string) { }
pub fn sortArray(arr: [i32]) { }
pub fn connectToDatabase() { }
```

### 2. 清晰的公开接口

只导出必要的函数：

```vix
// math.vix

// 公开接口
pub fn calculate(x: i32, y: i32) -> i32 {
    return internalHelper(x) + internalHelper(y)
}

// 私有实现细节
fn internalHelper(n: i32) -> i32 {
    return n * n + 1
}
```

### 3. 避免循环依赖

```
// 错误：循环依赖
// a.vix
import "b.vix"

// b.vix
import "a.vix"  // 循环！

// 正确：重构代码，提取公共部分
// common.vix
pub fn sharedFunction() { }

// a.vix
import "common.vix"

// b.vix
import "common.vix"
```

### 4. 合理的导入顺序

```vix
// 标准库
import "std/io.vix"
import "std/strings.vix"

// 第三方库
import "lib/http.vix"

// 本地模块
import "utils.vix"
import "config.vix"
```

---

## 下一步

- [标准库](stdlib.md) - 标准库详细文档
- [函数](functions.md) - 函数定义和导出
- [编译器](compiler.md) - 编译多文件项目
