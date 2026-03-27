# 标准库

Vix 提供了一套标准库，包含常用的数据结构和函数。本文档详细介绍标准库的使用方法。

## 目录

- [std/io - 输入输出](#stdio---输入输出)
- [std/arr - 数组操作](#stdarr---数组操作)
- [std/strings - 字符串操作](#stdstrings---字符串操作)
- [std/os - 操作系统接口](#stdos---操作系统接口)
- [std/mem - 内存管理](#stdmem---内存管理)
- [std/rand - 随机数](#stdrand---随机数)
- [std/net - 网络编程](#stdnet---网络编程)

---

## std/io - 输入输出

提供基本的输入输出和文件操作功能。

### 导入

```vix
import "std/io.vix"
```

### 函数列表

| 函数 | 签名 | 描述 |
|------|------|------|
| `puts` | `fn puts(s: ptr) -> i32` | 打印字符串并换行 |
| `fopen` | `fn fopen(filename: ptr, mode: ptr) -> ptr` | 打开文件 |
| `fclose` | `fn fclose(file: ptr) -> i32` | 关闭文件 |
| `fread` | `fn fread(ptr: ptr, size: usize, count: usize, file: ptr) -> usize` | 读取文件 |
| `fwrite` | `fn fwrite(ptr: ptr, size: usize, count: usize, file: ptr) -> usize` | 写入文件 |
| `panic` | `fn panic(msg: ptr) -> i32` | 打印错误信息并挂起程序 |

### 使用示例

#### 控制台输出

```vix
import "std/io.vix"

fn main() -> i32 {
    puts("Hello, Vix!")
    return 0
}
```

#### 文件写入

```vix
import "std/io.vix"

fn main() -> i32 {
    let file = fopen("output.txt", "w")
    if (file == nil) {
        puts("Failed to open file for writing")
        return 1
    }
    
    let data = "Hello, this is written by Vix!"
    fwrite(data, 1, data.length, file)
    fclose(file)
    
    puts("File written successfully")
    return 0
}
```

#### 文件读取

```vix
import "std/io.vix"

extern "C" {
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
}

fn main() -> i32 {
    let file = fopen("input.txt", "r")
    if (file == nil) {
        puts("Failed to open file for reading")
        return 1
    }
    
    // 分配缓冲区
    let buffer = malloc(1024)
    let bytesRead = fread(buffer, 1, 1024, file)
    
    // 处理数据...
    printf("Read %d bytes\n", bytesRead)
    
    fclose(file)
    free(buffer)
    return 0
}
```

#### 错误处理

```vix
import "std/io.vix"

fn main() -> i32 {
    let file = fopen("nonexistent.txt", "r")
    if (file == nil) {
        panic("Cannot open required file")
    }
    
    // 不会执行到这里
    fclose(file)
    return 0
}
```

---

## std/arr - 数组操作

提供数组排序和操作功能。

### 导入

```vix
import "std/arr.vix"
```

### 函数列表

| 函数 | 签名 | 描述 |
|------|------|------|
| `sort` | `fn sort(nums: [i32], size: i32)` | 冒泡排序数组 |

### 使用示例

#### 数组排序

```vix
import "std/arr.vix"

fn main() -> i32 {
    let arr = [64, 34, 25, 12, 22, 11, 90]
    
    puts("Before sorting:")
    for (i in 0 .. arr.length) {
        printf("%d ", arr[i])
    }
    printf("\n")
    
    sort(arr, arr.length)
    
    puts("After sorting:")
    for (i in 0 .. arr.length) {
        printf("%d ", arr[i])
    }
    printf("\n")
    
    return 0
}
```

输出：
```
Before sorting:
64 34 25 12 22 11 90 
After sorting:
11 12 22 25 34 64 90 
```

---

## std/strings - 字符串操作

提供字符串比较和处理功能。

### 导入

```vix
import "std/strings.vix"
```

### 函数列表

| 函数 | 签名 | 描述 |
|------|------|------|
| `strcmp` | `fn strcmp(a: ptr, b: ptr) -> i32` | 比较两个字符串 |

### 使用示例

#### 字符串比较

```vix
import "std/strings.vix"

fn main() -> i32 {
    let str1 = "apple"
    let str2 = "banana"
    let str3 = "apple"
    
    // 比较不同的字符串
    let cmp1 = strcmp(str1, str2)
    if (cmp1 < 0) {
        printf("%s < %s\n", str1, str2)  // apple < banana
    }
    
    // 比较相同的字符串
    let cmp2 = strcmp(str1, str3)
    if (cmp2 == 0) {
        printf("%s == %s\n", str1, str3)  // apple == apple
    }
    
    return 0
}
```

#### 命令行参数解析

```vix
import "std/strings.vix"

fn main(argc: i32, argv: ptr) -> i32 {
    for (i in 1 .. argc) {
        if (strcmp(argv[i], "--help") == 0) {
            puts("Usage: program [options]")
            puts("Options:")
            puts("  --help    Show this help message")
            puts("  --version Show version")
            return 0
        }
        if (strcmp(argv[i], "--version") == 0) {
            puts("Version 1.0.0")
            return 0
        }
    }
    
    puts("Running program...")
    return 0
}
```

---

## std/os - 操作系统接口

提供与操作系统交互的功能。

### 导入

```vix
import "std/os.vix"
```

### 函数列表

| 函数 | 签名 | 描述 |
|------|------|------|
| `system` | `fn system(cmd: ptr) -> i32` | 执行系统命令 |
| `exit` | `fn exit(status: i32) -> void` | 退出程序 |
| `opendir` | `fn opendir(path: ptr) -> ptr` | 打开目录 |
| `readdir` | `fn readdir(dir: ptr) -> ptr` | 读取目录项 |
| `closedir` | `fn closedir(dir: ptr) -> i32` | 关闭目录 |

### 使用示例

#### 执行系统命令

```vix
import "std/os.vix"

fn main() -> i32 {
    // 列出当前目录
    system("ls -la")
    
    // 创建目录
    system("mkdir -p output")
    
    // 复制文件
    system("cp input.txt output/")
    
    return 0
}
```

#### 程序退出

```vix
import "std/os.vix"

fn main() -> i32 {
    let success = doSomething()
    
    if (!success) {
        puts("Operation failed")
        exit(1)
    }
    
    puts("Success!")
    exit(0)
}

fn doSomething() -> bool {
    // 执行操作
    return true
}
```

#### 目录操作

```vix
import "std/os.vix"

fn main() -> i32 {
    let dir = opendir(".")
    if (dir == nil) {
        puts("Failed to open directory")
        return 1
    }
    
    // 读取目录内容
    let entry = readdir(dir)
    while (entry != nil) {
        // 处理目录项
        entry = readdir(dir)
    }
    
    closedir(dir)
    return 0
}
```

---

## std/mem - 内存管理

提供动态内存分配和管理功能。

### 导入

```vix
import "std/mem.vix"
```

### 函数列表

| 函数 | 签名 | 描述 |
|------|------|------|
| `malloc` | `fn malloc(size: i32) -> ptr` | 分配内存 |
| `free` | `fn free(ptr: ptr)` | 释放内存 |
| `memcpy` | `fn memcpy(dest: ptr, src: ptr, n: i32) -> ptr` | 复制内存 |
| `memset` | `fn memset(s: ptr, c: i32, n: i32) -> ptr` | 设置内存 |
| `memcmp` | `fn memcmp(s1: ptr, s2: ptr, n: i32) -> i32` | 比较内存 |

### 使用示例

#### 动态数组

```vix
import "std/mem.vix"

fn main() -> i32 {
    // 分配存储 10 个整数的空间
    let arr = malloc(40)  // 10 * 4 bytes
    
    // 初始化数组
    for (i in 0 .. 10) {
        arr[i] = i * i
    }
    
    // 打印数组
    for (i in 0 .. 10) {
        printf("%d ", arr[i])
    }
    printf("\n")
    
    // 释放内存
    free(arr)
    return 0
}
```

#### 内存复制

```vix
import "std/mem.vix"

fn main() -> i32 {
    let src = malloc(100)
    let dst = malloc(100)
    
    // 初始化源数据
    for (i in 0 .. 25) {
        src[i] = i
    }
    
    // 复制数据
    memcpy(dst, src, 100)
    
    // 验证复制
    for (i in 0 .. 25) {
        printf("%d ", dst[i])
    }
    printf("\n")
    
    free(src)
    free(dst)
    return 0
}
```

#### 内存清零

```vix
import "std/mem.vix"

fn main() -> i32 {
    let buffer = malloc(1024)
    
    // 清零缓冲区
    memset(buffer, 0, 1024)
    
    // 使用缓冲区...
    
    free(buffer)
    return 0
}
```

---

## std/rand - 随机数

提供随机数生成功能。

### 导入

```vix
import "std/rand.vix"
```

### 函数列表

| 函数 | 签名 | 描述 |
|------|------|------|
| `rand` | `fn rand(mini: i32, maxi: i32) -> i32` | 生成指定范围内的随机数 |

### 使用示例

#### 生成随机数

```vix
import "std/rand.vix"

fn main() -> i32 {
    // 生成 1 到 100 的随机数
    let r1 = rand(1, 100)
    printf("Random number (1-100): %d\n", r1)
    
    // 生成 0 到 9 的随机数
    let r2 = rand(0, 9)
    printf("Random digit: %d\n", r2)
    
    // 模拟骰子
    let dice = rand(1, 6)
    printf("Dice roll: %d\n", dice)
    
    return 0
}
```

#### 猜数字游戏

```vix
import "std/rand.vix"

fn main() -> i32 {
    let target = rand(1, 100)
    let guesses = 0
    
    puts("Guess a number between 1 and 100!")
    
    while (true) {
        let guess = toint(input("Your guess: "))
        guesses += 1
        
        if (guess == target) {
            printf("Correct! You got it in %d guesses!\n", guesses)
            return 0
        } elif (guess < target) {
            puts("Too low!")
        } else {
            puts("Too high!")
        }
    }
}
```

---

## std/net - 网络编程

提供网络编程功能。

### 导入

```vix
import "std/net/net.vix"
```

### 函数列表

| 函数 | 描述 |
|------|------|
| `socket(domain, type, protocol)` | 创建套接字 |
| `listen(sock, backlog)` | 监听连接 |
| `accept(sock, addr, addrlen)` | 接受连接 |
| `send(sock, buf, len, flags)` | 发送数据 |
| `close(sock)` | 关闭套接字 |

### 使用示例

#### HTTP 服务器

```vix
import "std/net/net.vix"

fn main() -> i32 {
    // 创建 TCP socket
    let sock = socket(2, 1, 0)  // AF_INET, SOCK_STREAM
    if (sock < 0) {
        printf("Failed to create socket\n")
        return 1
    }
    
    // 开始监听
    if (listen(sock, 5) < 0) {
        printf("Failed to listen\n")
        return 1
    }
    
    printf("Server running on port 80...\n")
    
    // HTTP 响应
    let response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Hello from Vix!</h1>"
    
    // 处理请求
    while (1) {
        let client = accept(sock, 0, 0)
        if (client >= 0) {
            printf("Client connected\n")
            send(client, response, 100, 0)
            close(client)
        }
    }
    
    close(sock)
    return 0
}
```

---

## 内置函数

除了标准库模块，Vix 还提供了一些内置函数，无需导入即可使用。

### print

打印输出：

```vix
print("Hello, World!")
print("Value:", 42)
print("Name:", name, "Age:", age)
```

### input

读取用户输入：

```vix
let name = input("Enter your name: ")
print("Hello, " + name)
```

### toint

字符串转整数：

```vix
let num = toint("123")
print(num + 1)  // 124
```

### tofloat

字符串转浮点数：

```vix
let f = tofloat("3.14")
print(f * 2)  // 6.28
```

---

## 下一步

- [模块系统](modules.md) - 如何创建和使用模块
- [指针](pointers.md) - 内存操作
- [编译器](compiler.md) - 编译选项
