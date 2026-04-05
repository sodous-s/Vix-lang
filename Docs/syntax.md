# Vix 语言语法参考

本文档提供 Vix 编程语言的完整语法参考。

## 目录

- [程序结构](#程序结构)
- [注释](#注释)
- [关键字](#关键字)
- [标识符](#标识符)
- [变量](#变量)
- [数据类型](#数据类型)
- [运算符](#运算符)
- [控制流](#控制流)
- [函数](#函数)
- [结构体](#结构体)
- [数组与列表](#数组与列表)
- [指针](#指针)
- [模块系统](#模块系统)
- [错误处理](#错误处理)

---

## 程序结构

### 入口函数

每个 Vix 程序都从 `main` 函数开始执行：

```vix
fn main(): i32 {
    // 程序代码
    return 0
}
```

带命令行参数的入口函数：

```vix
fn main(argc: i32, argv: ptr): i32 {
    for (i in 0 .. argc) {
        print(argv[i])
    }
    return 0
}
```

### 程序组成

一个 Vix 程序可以包含：

- 全局变量声明
- 结构体定义
- 函数定义
- 导入语句
- 外部函数声明

```vix
// 导入模块
import "std/io.vix"

// 全局变量
global counter = 42

// 结构体定义
struct Point {
    x: f64,
    y: f64
}

// 外部函数声明
extern "C" {
    fn printf(format: ptr, ...) -> i32
}

// 函数定义
fn main(): i32 {
    print("Hello, Vix!")
    return 0
}
```

---

## 注释

### 单行注释

使用 `//` 开始，到行尾结束：

```vix
// 这是一个单行注释
let x = 10  // 行末注释
```

### 多行注释

使用 `/* */` 包裹：

```vix
/*
 * 这是一个
 * 多行注释
 */
let x = 10
```

---

## 关键字

Vix 的保留关键字：

| 关键字 | 用途 |
|--------|------|
| `fn` | 函数定义 |
| `let` | 不可变变量声明 |
| `mut` | 可变变量声明 |
| `if` | 条件判断 |
| `elif` | 否则如果 |
| `else` | 否则 |
| `while` | while 循环 |
| `for` | for 循环 |
| `in` | 循环范围 |
| `return` | 返回语句 |
| `struct` | 结构体定义 |
| `import` | 模块导入 |
| `pub` | 公开声明 |
| `extern` | 外部函数声明 |
| `global` | 全局变量声明 |
| `nil` | 空指针 |
| `and` | 逻辑与 |
| `or` | 逻辑或 |
| `match` | 模式匹配 |

---

## 标识符

标识符用于命名变量、函数、结构体等。

### 命名规则

- 必须以字母或下划线开头
- 可以包含字母、数字和下划线
- 区分大小写

```vix
// 合法标识符
myVariable
_myVar
MyFunction
value1

// 非法标识符
1value      // 不能以数字开头
my-var      // 不能包含连字符
```

### 命名约定

- **变量**：小驼峰命名（`myVariable`）
- **函数**：小驼峰命名（`calculateSum`）
- **结构体**：大驼峰命名（`MyStruct`）
- **常量**：全大写下划线分隔（`MAX_SIZE`）

---

## 变量

### 变量声明

#### 变量

使用 `let` 关键字声明变量：

```vix
let x = 10
let name = "Vix"
let pi = 3.14159
```


### 类型标注

可以在声明时显式指定类型：

```vix
let a: i32 = 10
let b: f64 = 3.14
let c: string = "Hello"
let flag: bool = true
```

### 变量赋值

```vix
mut x = 10
x = 20        // 简单赋值
x += 5        // 复合赋值
x -= 3
x *= 2
x /= 4
```

---

## 数据类型

### 基本类型

| 类型 | 描述 | 大小 |
|------|------|------|
| `i8` | 8位有符号整数 | 1 byte |
| `i32` | 32位有符号整数 | 4 bytes |
| `i64` | 64位有符号整数 | 8 bytes |
| `f32` | 32位浮点数 | 4 bytes |
| `f64` | 64位浮点数 | 8 bytes |
| `bool` | 布尔类型 | 1 byte |
| `string` | 字符串类型 | - |
| `void` | 空类型 | - |
| `ptr` | 通用指针类型 | - |
| `usize` | 无符号整数（指针大小） | - |

### 类型标注语法

```vix
let num: i32 = 42
let float: f64 = 3.14
let text: string = "Hello"
let flag: bool = true
let ptr: &i32 = &num// 指针类型
let arr: [i32 * 5] = [1, 2, 3, 4, 5]// 数组
```

---

## 运算符

### 算术运算符

| 运算符 | 描述 | 示例 |
|--------|------|------|
| `+` | 加法 | `a + b` |
| `-` | 减法 | `a - b` |
| `*` | 乘法 | `a * b` |
| `/` | 除法 | `a / b` |
| `%` | 取模 | `a % b` |
| `**` | 幂运算 | `a ** b` |

### 比较运算符

| 运算符 | 描述 | 示例 |
|--------|------|------|
| `==` | 等于 | `a == b` |
| `!=` | 不等于 | `a != b` |
| `<` | 小于 | `a < b` |
| `<=` | 小于等于 | `a <= b` |
| `>` | 大于 | `a > b` |
| `>=` | 大于等于 | `a >= b` |

### 逻辑运算符

| 运算符 | 描述 | 示例 |
|--------|------|------|
| `and` | 逻辑与 | `a and b` |
| `or` | 逻辑或 | `a or b` |
| `!` | 逻辑非 | `!a` |

### 位运算符

| 运算符 | 描述 | 示例 |
|--------|------|------|
| `&` | 按位与 | `a & b` |
| `\|` | 按位或 | `a \| b` |
| `^` | 按位异或 | `a ^ b` |
| `~` | 按位取反 | `~a` |
| `<<` | 左移 | `a << n` |
| `>>` | 右移 | `a >> n` |

### 指针运算符

| 运算符 | 描述 | 示例 |
|--------|------|------|
| `&` | 取地址 | `&x` |
| `@` | 解引用 | `@ptr` |

### 运算符优先级

从高到低：

1. `()` `[]` `.` `@`
2. `!` `~` `-`（一元）`&`
3. `**`
4. `*` `/` `%`
5. `+` `-`
6. `<<` `>>`
7. `<` `<=` `>` `>=`
8. `==` `!=`
9. `&`（位运算）
10. `^`
11. `|`
12. `and`
13. `or`
14. `=` `+=` `-=` `*=` `/=`

---

## 控制流

### if 语句

```vix
if (condition) {
    // 代码块
}

// 带 elif 和 else
if (x > 10) {
    print("x is greater than 10")
} elif (x > 5) {
    print("x is greater than 5")
} else {
    print("x is 5 or less")
}
```

### while 循环

```vix
let i = 0
while (i < 10) {
    print(i)
    i += 1
}
```

### for 循环

#### 范围循环

```vix
// 从 1 到 9（不包括 10）
for (i in 1 .. 10) {
    print(i)
}
```

#### 嵌套循环

```vix
for (x in 1 .. 3) {
    for (y in 1 .. 3) {
        print("x:", x, "y:", y)
    }
}
```

### break 和 continue

```vix
// break - 跳出循环
let i = 0
while (true) {
    if (i >= 10) {
        break
    }
    print(i)
    i += 1
}

// continue - 跳过当前迭代
for (i in 1 .. 10) {
    if (i % 2 == 0) {
        continue
    }
    print(i)  // 只打印奇数
}
```

### match 语句

```vix
match value {
    10 -> {
        print("value is 10")
    }
    20 -> {
        print("value is 20")
    }
    _ -> {
        print("value is something else")
    }
}
```

---

## 函数

### 函数定义

```vix
fn function_name(param1: type1, param2: type2): return_type {
    // 函数体
    return value
}
```

### 基本示例

```vix
fn add(a: i32, b: i32): i32 {
    return a + b
}

fn greet(name: string) {
    print("Hello, " + name + "!")
}
```

### 公开函数

使用 `pub` 关键字声明可导出的函数：

```vix
pub fn publicFunction(x: i32): i32 {
    return x * 2
}
```

### 外部函数

```vix
extern "C" {
    fn printf(format: ptr, ...): i32
    fn malloc(size: i32): ptr
    fn free(ptr: ptr): void
}
```

### 泛型函数

```vix
fn id:[T](value: T): T {
    return value
}

fn add:[T](a: T, b: T): T {
    return a + b
}

// 使用
let x = id:[i32](42)
let y = add:[f64](2.5, 3.5)
```

---

## 结构体

### 定义结构体

```vix
struct Person {
    name: string,
    age: i32,
    height: f64
}
```

### 创建实例

```vix
// 使用初始化列表
let p1 = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}

// 逐字段赋值
let p2: Person {}
p2.name = "Bob"
p2.age = 30
p2.height = 6.0
```

### 访问字段

```vix
print(p1.name)    // "Alice"
print(p1.age)     // 25
print(p1.height)  // 5.7
```

### 泛型结构体

```vix
struct Box:[T] {
    value: T
}

let intBox = Box:[i32]{ value: 42 }
let floatBox = Box:[f64]{ value: 3.14 }
```

---

## 数组与列表

### 固定大小数组

```vix
// 声明
let arr: [i32 * 5] = [1, 2, 3, 4, 5]

// 访问元素
print(arr[0])      // 1
print(arr[2])      // 3

// 修改元素
arr[1] = 10

// 数组长度
print(arr.length)  // 5
```

### 动态列表

```vix
fn map:[T, U](list: [T], f: fn(T): U): [U]
{
    let out = []
    for (item in list)
    {
        out.push(f(item))
    }
    return out
}

fn main(): i32
{
    let nums = [1, 2, 3]
    let doubled = map:[i32, i32](nums, fn(x: i32): i32 { x * 2 })
    print(doubled[0])
    print(doubled[1])
    print(doubled[2])
    return 0
}

```

### 数组作为函数参数

```vix
fn printArray(arr: [i32], len: i32) {
    for (i in 0 .. len) {
        print(arr[i])
    }
}
```

---

## 指针

### 取地址与解引用

```vix
let x = 10
let mut ptr = &x        // 获取 x 的地址

let value = @ptr    // 解引用：获取 ptr 指向的值
@ptr = 20           // 通过指针修改值
```

### 指针运算

```vix
let arr = [1, 2, 3, 4, 5]
let p = &arr[0]
let second = @(p + 1)  // 获取 arr[1] = 2
print(second)
```

### 空指针

```vix
let p: &i32 = nil

if (p == nil) {
    print("Pointer is null")
}
```

---

## 模块系统

### 导入模块

```vix
// 导入标准库模块
import "std/io.vix"
import "std/arr.vix"
import "std/strings.vix"
```

### 导出函数

使用 `pub` 关键字标记需要导出的函数：

```vix
// mymodule.vix
pub fn myPublicFunction(x: i32): i32 {
    return x * 2
}

fn privateHelper() {
    // 这个函数不会被导出
}
```

### 使用导入的函数

```vix
import "mymodule.vix"

fn main(): i32 {
    let result = myPublicFunction(5)
    print(result)
    return 0
}
```

---

## 错误处理

---

## 语法扩展（2026-03）

以下语法已在 `src/test.vix` 场景中完成编译链路验证（`vixc -> out.ll -> clang -> run`）。

### 泛型声明与调用

支持两种泛型写法（保持兼容）：

```vix
fn new_table:[T](): SymbolTable:[T] { ... }
let tab = new_table[Type]()

struct SymbolTable:[T] {
    scopes: [ [ (string, T) ] ]
}
```

### match 与构造器模式

支持 `match` 分支中的构造器模式与绑定：

```vix
match t {
    Some(v) -> print(v.name)
    None -> print("not found")
}

match ok {
    Ok(v) -> print(v)
    Err(e) -> print(e)
}
```

### 联合类型（ADT）

支持联合类型定义形式：

```vix
type Result:[T, E] = Ok(T) | Err(E)
```

### 其他兼容语法

- 可选类型：`?T`
- 函数类型：`fn(T): U`
- 元组样式参数：`(a, b)`（例如 `push((name, val))`）
- 元组样式索引字段：`pair.0`、`pair.1`
- `for (item in iterable)` 形式
- 行注释：`// comment`

### 当前实现备注

- 泛型函数调用在未显式写出类型参数时，会走后端默认实例化兜底路径。
- 联合类型与构造器目前为可用实现，复杂场景（深层嵌套、完整代数数据类型优化）仍在持续演进。


---

## 输入输出

### 输出

```vix
// 打印字符串
print("Hello, World!")

// 打印多个值
print("Name:", name, "Age:", age)

// 格式化输出
printf("Value: %d, Text: %s\n", 42, "test")

```
notes:printf需要extern "C" {fn printf(fmt:ptr,...): i32}
---

## 全局变量

使用 `global` 关键字声明全局变量：

```vix
global counter = 42
global message: string = "Hello World"
global pi: f64 = 3.14159

fn main() -> i32 {
    print(counter)
    print(message)
    print(pi)
    return 0
}
```

---

## 下一步

- [类型系统](types.md) - 深入了解类型系统
- [函数详解](functions.md) - 更多函数特性
- [标准库](stdlib.md) - 内置功能参考
