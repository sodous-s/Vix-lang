# 函数

函数是 Vix 程序的基本构建块。本文档详细介绍 Vix 的函数特性。

## 目录

- [函数定义](#函数定义)
- [参数](#参数)
- [返回值](#返回值)
- [函数调用](#函数调用)
- [作用域](#作用域)
- [递归](#递归)
- [泛型函数](#泛型函数)
- [外部函数](#外部函数)
- [公开函数](#公开函数)

---

## 函数定义

### 基本语法

```vix
fn function_name(parameters) -> return_type {
    // 函数体
    return value
}
```

### 示例

```vix
fn add(a: i32, b: i32) -> i32 {
    return a + b
}

fn greet(name: string) {
    print("Hello, " + name + "!")
}
```

### 主函数

每个 Vix 程序必须有一个 `main` 函数作为入口点：

```vix
fn main() -> i32 {
    // 程序代码
    return 0
}
```

带命令行参数的主函数：

```vix
fn main(argc: i32, argv: ptr) -> i32 {
    for (i in 0 .. argc) {
        print(argv[i])
    }
    return 0
}
```

---

## 参数

### 参数声明

参数使用 `name: type` 的语法：

```vix
fn power(base: i32, exponent: i32) -> i32 {
    let result = 1
    for (i in 0 .. exponent) {
        result *= base
    }
    return result
}
```

### 支持的类型

所有基本类型和复合类型都可以作为参数：

```vix
// 基本类型
fn processInt(n: i32) { }
fn processFloat(f: f64) { }
fn processBool(b: bool) { }
fn processString(s: string) { }

// 指针
fn processPointer(p: &i32) { }
fn processGenericPointer(p: ptr) { }

// 数组
fn processArray(arr: [i32], len: i32) { }
fn processFixedSizeArray(arr: [i32 * 5]) { }

// 结构体
struct Point { x: f64, y: f64 }
fn processPoint(p: Point) { }
```

### 可变参数

使用 `...` 表示可变参数：

```vix
extern "C" {
    fn printf(format: ptr, ...) -> i32
}
```

---

## 返回值

### 返回语句

使用 `return` 关键字返回值：

```vix
fn multiply(a: i32, b: i32) -> i32 {
    return a * b
}
```

### 无返回值

函数可以不返回值（隐式返回 void）：

```vix
fn log(message: string) {
    print("[LOG] " + message)
    // 没有 return 语句
}
```

显式声明 void 返回类型：

```vix
fn doSomething() -> void {
    // 函数体
}
```

### 多个返回路径

```vix
fn absolute(n: i32) -> i32 {
    if (n < 0) {
        return -n
    }
    return n
}
```

---

## 函数调用

### 基本调用

```vix
let result = add(3, 5)
print(result)  // 8
```

### 嵌套调用

```vix
let result = add(multiply(2, 3), 4)
print(result)  // 10
```

### 链式调用

```vix
fn double(n: i32) -> i32 {
    return n * 2
}

let result = double(double(double(2)))
print(result)  // 16
```

---

## 作用域

### 局部变量

函数内部声明的变量是局部的：

```vix
fn example() {
    let x = 10      // x 只在 example 内有效
    print(x)
}

fn another() {
    // print(x)     // 错误：x 未定义
}
```

### 变量遮蔽

内层作用域可以遮蔽外层变量：

```vix
let x = 10

fn example() {
    let x = 20      // 遮蔽外层的 x
    print(x)        // 20
}

print(x)            // 10
```

### 块作用域

```vix
fn example() {
    let a = 1
    {
        let b = 2
        print(a)    // 1 - 可以访问外层变量
        print(b)    // 2
    }
    // print(b)     // 错误：b 超出作用域
    print(a)        // 1
}
```

---

## 递归

函数可以调用自身：

### 基本递归

```vix
fn factorial(n: i32) -> i32 {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

print(factorial(5))  // 120
```

### 斐波那契数列

```vix
fn fib(n: i32) -> i32 {
    if (n <= 1) {
        return n
    }
    return fib(n - 1) + fib(n - 2)
}

print(fib(10))  // 55
```

### 尾递归优化

Vix 编译器会对尾递归进行优化：

```vix
fn fibTail(n: i32, a: i32, b: i32) -> i32 {
    if (n == 0) {
        return a
    }
    return fibTail(n - 1, b, a + b)
}

print(fibTail(40, 0, 1))  // 高效计算
```

---

## 泛型函数

### 定义泛型函数

使用 `[T]` 语法定义泛型：

```vix
fn identity:[T](value: T) -> T {
    return value
}

fn add:[T](a: T, b: T) -> T {
    return a + b
}
```

### 使用泛型函数

```vix
// 显式指定类型
let a = identity:[i32](42)
let b = identity:[string]("Hello")
let c = add:[f64](2.5, 3.5)

// 让编译器推断
let d = identity(42)        // 推断为 i32
let e = identity("Hello")   // 推断为 string
```

### 泛型约束

泛型参数需要支持所需操作：

```vix
// T 必须支持 + 运算
fn sum:[T](a: T, b: T, c: T) -> T {
    return a + b + c
}

let result = sum:[i32](1, 2, 3)  // 6
```

---

## 外部函数

使用 `extern` 声明外部函数：

### C 标准库函数

```vix
extern "C" {
    fn printf(format: ptr, ...) -> i32
    fn puts(s: ptr) -> i32
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
    fn memcpy(dest: ptr, src: ptr, n: i32) -> ptr
    fn memset(s: ptr, c: i32, n: i32) -> ptr
}
```

### 使用外部函数

```vix
extern "C" {
    fn printf(format: ptr, ...) -> i32
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
}

fn main() -> i32 {
    // 分配内存
    let buffer = malloc(100)
    
    // 使用内存
    for (i in 0 .. 10) {
        buffer[i] = i * i
    }
    
    // 打印结果
    for (i in 0 .. 10) {
        printf("%d ", buffer[i])
    }
    printf("\n")
    
    // 释放内存
    free(buffer)
    return 0
}
```

### 系统调用

```vix
extern "C" {
    fn system(cmd: ptr) -> i32
    fn exit(status: i32) -> void
}

fn main() -> i32 {
    system("echo Hello from shell")
    return 0
}
```

---

## 公开函数

使用 `pub` 关键字导出函数：

### 定义公开函数

```vix
// math.vix
pub fn square(n: i32) -> i32 {
    return n * n
}

pub fn cube(n: i32) -> i32 {
    return n * n * n
}

// 内部函数，不会被导出
fn helper() {
    // ...
}
```

### 导入和使用

```vix
import "math.vix"

fn main() -> i32 {
    let sq = square(5)   // 25
    let cb = cube(3)     // 27
    print(sq)
    print(cb)
    return 0
}
```

---

## 函数作为值

Vix 支持函数指针：

```vix
fn apply(a: i32, b: i32, op: fn(i32, i32) -> i32) -> i32 {
    return op(a, b)
}

fn add(a: i32, b: i32) -> i32 {
    return a + b
}

fn multiply(a: i32, b: i32) -> i32 {
    return a * b
}

fn main() -> i32 {
    print(apply(3, 4, add))       // 7
    print(apply(3, 4, multiply))  // 12
    return 0
}
```

---

## 内联函数

编译器会自动内联小型函数以提高性能：

```vix
fn square(n: i32) -> i32 {
    return n * n
}

fn main() -> i32 {
    // square 可能被内联
    let result = square(5)
    print(result)
    return 0
}
```

---

## 最佳实践

1. **单一职责**：每个函数只做一件事
2. **有意义的命名**：函数名应该清楚表达其功能
3. **适当的长度**：函数不宜过长，建议不超过 50 行
4. **减少副作用**：尽量使用纯函数
5. **类型明确**：对于公共函数，使用显式类型标注

---

## 示例

### 数学函数库

```vix
pub fn abs(n: i32) -> i32 {
    if (n < 0) {
        return -n
    }
    return n
}

pub fn max(a: i32, b: i32) -> i32 {
    if (a > b) {
        return a
    }
    return b
}

pub fn min(a: i32, b: i32) -> i32 {
    if (a < b) {
        return a
    }
    return b
}

pub fn clamp(value: i32, low: i32, high: i32) -> i32 {
    return max(low, min(value, high))
}
```

### 字符串处理函数

```vix
pub fn strlen(s: string) -> i32 {
    return s.length
}

pub fn strconcat(a: string, b: string) -> string {
    return a + b
}
```

---

## 下一步

- [控制流](control-flow.md) - 控制流语句
- [结构体](structs.md) - 结构体和方法
- [模块系统](modules.md) - 模块和组织代码
