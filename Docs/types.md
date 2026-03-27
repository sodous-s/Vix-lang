# 类型系统

Vix 采用静态类型系统，在编译时进行类型检查。本文档详细介绍 Vix 的类型系统。

## 目录

- [基本类型](#基本类型)
- [整数类型](#整数类型)
- [浮点类型](#浮点类型)
- [布尔类型](#布尔类型)
- [字符串类型](#字符串类型)
- [指针类型](#指针类型)
- [数组类型](#数组类型)
- [结构体类型](#结构体类型)
- [泛型类型](#泛型类型)
- [类型推断](#类型推断)
- [类型转换](#类型转换)

---

## 基本类型

Vix 提供以下基本类型：

| 类型 | 描述 | 大小 | 默认值 |
|------|------|------|--------|
| `i8` | 8位有符号整数 | 1 byte | 0 |
| `i32` | 32位有符号整数 | 4 bytes | 0 |
| `i64` | 64位有符号整数 | 8 bytes | 0 |
| `f32` | 32位浮点数 | 4 bytes | 0.0 |
| `f64` | 64位浮点数 | 8 bytes | 0.0 |
| `bool` | 布尔类型 | 1 byte | false |
| `string` | 字符串类型 | - | "" |
| `void` | 空类型 | 0 | - |
| `ptr` | 通用指针 | 8 bytes | nil |
| `usize` | 无符号整数 | 8 bytes | 0 |

---

## 整数类型

### i8 - 8位整数

用于存储小范围整数：

```vix
let small: i8 = 127    // 最大值
let tiny: i8 = -128    // 最小值
```

范围：-128 到 127

### i32 - 32位整数

最常用的整数类型：

```vix
let count: i32 = 1000000
let index: i32 = 0
let result: i32 = 42
```

范围：-2,147,483,648 到 2,147,483,647

### i64 - 64位整数

用于需要更大范围的场景：

```vix
let bigNumber: i64 = 9223372036854775807
let timestamp: i64 = 1704067200000
```

范围：-9,223,372,036,854,775,808 到 9,223,372,036,854,775,807

### 整数字面量

```vix
let decimal = 42        // 十进制
let hex = 0x2A          // 十六进制
let binary = 0b101010   // 二进制
let octal = 0o52        // 八进制
```

---

## 浮点类型

### f32 - 单精度浮点数

适用于精度要求不高的场景：

```vix
let temperature: f32 = 36.5
let ratio: f32 = 0.618
```

精度：约 6-7 位有效数字

### f64 - 双精度浮点数

默认浮点类型，精度更高：

```vix
let pi: f64 = 3.14159265358979
let e: f64 = 2.71828182845904
```

精度：约 15-16 位有效数字

### 浮点字面量

```vix
let a = 3.14
let b = 2.0
let c = 1.5e10      // 科学计数法
let d = 2.5e-3      // 0.0025
```

---

## 布尔类型

`bool` 类型只有两个值：`true` 和 `false`

```vix
let isActive: bool = true
let isEmpty: bool = false
let result = 10 > 5  // result 是 bool 类型，值为 true
```

### 布尔运算

```vix
let a = true
let b = false

let andResult = a and b    // false
let orResult = a or b      // true
let notResult = !a         // false
```

---

## 字符串类型

`string` 类型用于存储文本数据：

```vix
let greeting: string = "Hello, Vix!"
let name = "World"
let empty = ""
```

### 字符串操作

```vix
let s = "Hello"

// 字符串长度
print(s.length)      // 5

// 字符串拼接
let full = s + " World"  // "Hello World"

// 字符串比较
if (strcmp(s, "Hello") == 0) {
    print("Strings are equal")
}
```

### 字符串字面量

```vix
let simple = "Hello"
let withEscape = "Line1\nLine2"
let withTab = "Column1\tColumn2"
let withQuote = "She said \"Hi\""
```

### 转义字符

| 转义序列 | 描述 |
|----------|------|
| `\n` | 换行 |
| `\t` | 制表符 |
| `\r` | 回车 |
| `\\` | 反斜杠 |
| `\"` | 双引号 |
| `\'` | 单引号 |

---

## 指针类型

指针存储内存地址，用于直接操作内存。

### 声明指针

```vix
// 指向特定类型的指针
let intPtr: &i32
let floatPtr: &f64

// 通用指针
let genericPtr: ptr
```

### 取地址和解引用

```vix
let x = 10
mut ptr = &x        // 取地址：ptr 指向 x

let value = @ptr    // 解引用：获取 ptr 指向的值
@ptr = 20           // 通过指针修改 x 的值
```

### 空指针

```vix
let nullPtr: &i32 = nil

if (nullPtr == nil) {
    print("Pointer is null")
}
```

### 指针运算

```vix
let arr = [1, 2, 3, 4, 5]
let p = &arr[0]
let second = @(p + 1)  // 获取 arr[1] = 2
```

---

## 数组类型

### 固定大小数组

```vix
// 语法：[元素类型 * 大小]
let arr: [i32 * 5] = [1, 2, 3, 4, 5]

// 访问元素
print(arr[0])       // 1

// 修改元素
arr[1] = 10

// 获取长度
print(arr.length)   // 5
```

### 动态列表

```vix
// 语法：[元素类型]
let list: [i32] = [1, 2, 3]

// 或让编译器推断
let list2 = [1, 2, 3, 4, 5]
```

### 多维数组

```vix
// 二维数组
let matrix: [[i32 * 3] * 3] = [
    [1, 2, 3],
    [4, 5, 6],
    [7, 8, 9]
]

print(matrix[0][0])  // 1
print(matrix[1][2])  // 6
```

---

## 结构体类型

结构体是用户定义的复合类型。

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
let p = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}
```

### 访问字段

```vix
print(p.name)    // "Alice"
print(p.age)     // 25
print(p.height)  // 5.7
```

### 嵌套结构体

```vix
struct Address {
    street: string,
    city: string,
    zip: string
}

struct Person {
    name: string,
    address: Address
}

let p = Person {
    name: "Bob",
    address: Address {
        street: "123 Main St",
        city: "Anytown",
        zip: "12345"
    }
}

print(p.address.city)  // "Anytown"
```

---

## 泛型类型

Vix 支持泛型，可以创建参数化的类型和函数。

### 泛型结构体

```vix
struct Box:[T] {
    value: T
}

// 实例化
let intBox = Box:[i32]{ value: 42 }
let floatBox = Box:[f64]{ value: 3.14 }
let strBox = Box:[string]{ value: "Hello" }
```

### 泛型函数

```vix
fn identity:[T](x: T) -> T {
    return x
}

fn add:[T](a: T, b: T) -> T {
    return a + b
}

// 使用
let a = identity:[i32](42)
let b = identity:[string]("Hello")
let c = add:[f64](2.5, 3.5)
```

### 类型约束的联合类型

```vix
type Flag:[T] = On | Off

let state: Flag:[bool] = On

match state {
    On -> {
        print("Flag is on")
    }
    Off -> {
        print("Flag is off")
    }
}
```

---

## 类型推断

Vix 支持类型推断，编译器会根据上下文推断变量的类型。

### 字面量推断

```vix
let a = 42        // 推断为 i32
let b = 3.14      // 推断为 f64
let c = "Hello"   // 推断为 string
let d = true      // 推断为 bool
```

### 表达式推断

```vix
let x = 10        // i32
let y = 20        // i32
let z = x + y     // i32

let f1 = 1.5      // f64
let f2 = f1 * 2   // f64
```

### 函数返回值推断

```vix
fn getValue() -> i32 {
    return 42
}

let result = getValue()  // result 推断为 i32
```

---

## 类型转换

Vix 提供内置函数进行类型转换。

### 字符串转数字

```vix
// 字符串转整数
let num = toint("123")      // 123 (i32)

// 字符串转浮点数
let float = tofloat("3.14") // 3.14 (f64)
```

### 数字类型转换

数字类型之间的转换会自动进行或需要显式标注：

```vix
let a: i32 = 10
let b: i64 = a       // 自动扩展
let c: f64 = a       // 整数转浮点
```

### 指针类型转换

```vix
let ptr: ptr = malloc(100)
let intPtr: &i32 = ptr  // 通用指针转特定类型指针
```

---

## 类型别名

使用 `type` 关键字定义类型别名：

```vix
type Age = i32
type Name = string
type Point = [f64 * 2]

let age: Age = 25
let name: Name = "Alice"
let point: Point = [1.0, 2.0]
```

---

## 类型兼容性

### 赋值兼容

```vix
let a: i32 = 10
let b: i64 = a    // i32 可以赋值给 i64

let x: f32 = 1.5
let y: f64 = x    // f32 可以赋值给 f64
```

### 函数参数兼容

```vix
fn process(n: i64) {
    print(n)
}

let x: i32 = 42
process(x)  // i32 可以传递给 i64 参数
```

---

## 最佳实践

1. **优先使用类型推断**：让编译器推断简单变量的类型
2. **复杂类型使用显式标注**：对于函数参数、返回值等使用显式类型
3. **选择合适的整数大小**：根据数值范围选择 i8、i32 或 i64
4. **默认使用 f64**：除非有特殊需求，否则使用 f64 作为浮点类型
5. **使用泛型提高代码复用**：对于通用数据结构使用泛型

---

## 下一步

- [函数详解](functions.md) - 了解函数和类型的关系
- [结构体](structs.md) - 深入学习结构体类型
- [泛型](#泛型类型) - 更多泛型使用示例
