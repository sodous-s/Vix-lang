# 指针

指针是 Vix 中用于直接操作内存的特性。通过指针，你可以访问和修改内存地址中的数据。

## 目录

- [指针基础](#指针基础)
- [取地址和解引用](#取地址和解引用)
- [指针类型](#指针类型)
- [指针运算](#指针运算)
- [空指针](#空指针)
- [指针与数组](#指针与数组)
- [指针与函数](#指针与函数)
- [内存管理](#内存管理)
- [安全性](#安全性)

---

## 指针基础

指针是一个存储内存地址的变量。通过指针，你可以：

- 获取变量的内存地址
- 通过地址访问和修改数据
- 实现高效的数据传递
- 进行动态内存分配

### 基本语法

```vix
// 声明指针变量
let ptr: &i32      // 指向 i32 的指针
let fptr: &f64     // 指向 f64 的指针
let gptr: ptr      // 通用指针
```

---

## 取地址和解引用

### 取地址运算符 `&`

使用 `&` 获取变量的内存地址：

```vix
let x = 10
mut ptr = &x        // ptr 现在指向 x 的地址

print(ptr)          // 打印地址值
```

### 解引用运算符 `@`

使用 `@` 获取指针指向的值：

```vix
let x = 10
mut ptr = &x

let value = @ptr    // value = 10
print(value)        // 10
```

### 通过指针修改值

```vix
let x = 10
mut ptr = &x

@ptr = 20           // 通过指针修改 x 的值
print(x)            // 20
```

### 完整示例

```vix
fn main() -> i32 {
    let x = 10
    
    // 取地址
    mut ptr = &x
    print("x =", x)           // x = 10
    print("*ptr =", @ptr)     // *ptr = 10
    
    // 通过指针修改
    @ptr = 20
    print("x =", x)           // x = 20
    print("*ptr =", @ptr)     // *ptr = 20
    
    return 0
}
```

---

## 指针类型

### 类型化指针

Vix 支持指向特定类型的指针：

```vix
let intPtr: &i32
let floatPtr: &f64
let strPtr: &string
```

### 通用指针

`ptr` 类型是通用指针，可以指向任何类型：

```vix
let genericPtr: ptr

let x = 10
genericPtr = &x
```

### 指针的指针

```vix
let x = 10
mut ptr = &x
mut ptrToPtr = &ptr

print(@(@ptrToPtr))  // 10
```

---

## 指针运算

### 指针算术

指针可以进行加减运算：

```vix
let arr = [1, 2, 3, 4, 5]
let p = &arr[0]

// 指针加法
let second = @(p + 1)    // arr[1] = 2
let third = @(p + 2)     // arr[2] = 3

print(second)            // 2
print(third)             // 3
```

### 遍历数组

```vix
let arr = [10, 20, 30, 40, 50]
let p = &arr[0]

for (i in 0 .. 5) {
    print(@(p + i))
}
```

### 指针与偏移

```vix
let data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
let base = &data[0]

// 跳过前 5 个元素
let offset = base + 5
print(@offset)           // 6

// 访问更多元素
print(@(offset + 1))     // 7
print(@(offset + 2))     // 8
```

---

## 空指针

### nil 关键字

`nil` 表示空指针，不指向任何有效地址：

```vix
let ptr: &i32 = nil

if (ptr == nil) {
    print("Pointer is null")
}
```

### 检查空指针

```vix
fn safeAccess(ptr: &i32) -> i32 {
    if (ptr == nil) {
        print("Error: null pointer")
        return 0
    }
    return @ptr
}

let validPtr: &i32 = nil
let result = safeAccess(validPtr)  // Error: null pointer
```

### 初始化指针

```vix
let ptr: &i32 = nil  // 初始化为空

let x = 10
ptr = &x             // 稍后赋值

print(@ptr)          // 10
```

---

## 指针与数组

### 数组名作为指针

数组名可以作为指向第一个元素的指针：

```vix
let arr = [1, 2, 3, 4, 5]
let p = &arr[0]

// p 指向数组的第一个元素
print(@p)            // 1
```

### 指针遍历数组

```vix
fn printArray(arr: [i32], len: i32) {
    let p = &arr[0]
    for (i in 0 .. len) {
        printf("%d ", @(p + i))
    }
    printf("\n")
}
```

### 修改数组元素

```vix
fn doubleElements(arr: [i32], len: i32) {
    let p = &arr[0]
    for (i in 0 .. len) {
        @(p + i) = @(p + i) * 2
    }
}

let arr = [1, 2, 3, 4, 5]
doubleElements(arr, 5)
// arr = [2, 4, 6, 8, 10]
```

---

## 指针与函数

### 传递指针参数

通过指针可以修改函数外部的变量：

```vix
fn swap(a: &i32, b: &i32) {
    let temp = @a
    @a = @b
    @b = temp
}

mut x = 10
mut y = 20
swap(&x, &y)
print("x =", x, "y =", y)  // x = 20, y = 10
```

### 返回指针

```vix
extern "C" {
    fn malloc(size: i32) -> ptr
}

fn createInt(value: i32) -> &i32 {
    let ptr = malloc(4)
    @ptr = value
    return ptr
}

let p = createInt(42)
print(@p)  // 42
```

### 函数指针

```vix
fn add(a: i32, b: i32) -> i32 {
    return a + b
}

fn multiply(a: i32, b: i32) -> i32 {
    return a * b
}

fn calculate(a: i32, b: i32, op: fn(i32, i32) -> i32) -> i32 {
    return op(a, b)
}

let result1 = calculate(3, 4, add)       // 7
let result2 = calculate(3, 4, multiply)  // 12
```

---

## 内存管理

### 动态内存分配

使用外部函数进行内存管理：

```vix
extern "C" {
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
    fn realloc(ptr: ptr, size: i32) -> ptr
}
```

### 分配和使用内存

```vix
extern "C" {
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
    
    // 读取数据
    for (i in 0 .. 10) {
        printf("%d ", buffer[i])
    }
    printf("\n")
    
    // 释放内存
    free(buffer)
    return 0
}
```

### 内存复制

```vix
extern "C" {
    fn memcpy(dest: ptr, src: ptr, n: i32) -> ptr
    fn memset(s: ptr, c: i32, n: i32) -> ptr
}

fn clearBuffer(buf: ptr, size: i32) {
    memset(buf, 0, size)
}
```

---

## 安全性

### 避免悬空指针

```vix
// 危险：返回局部变量的指针
fn badFunction() -> &i32 {
    let local = 10
    return &local  // 错误！local 在函数返回后失效
}

// 安全：使用动态分配
extern "C" {
    fn malloc(size: i32) -> ptr
}

fn goodFunction() -> &i32 {
    let ptr = malloc(4)
    @ptr = 10
    return ptr
}
```

### 检查指针有效性

```vix
fn safeRead(ptr: &i32) -> i32 {
    if (ptr == nil) {
        print("Warning: null pointer access")
        return 0
    }
    return @ptr
}
```

### 内存泄漏预防

```vix
extern "C" {
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
}

fn example() -> i32 {
    let buffer = malloc(1000)
    
    // 使用 buffer...
    
    // 确保释放
    free(buffer)
    return 0
}
```

---

## 最佳实践

1. **始终初始化指针**：要么指向有效地址，要么设为 `nil`
2. **检查空指针**：解引用前检查指针是否为 `nil`
3. **配对分配和释放**：每个 `malloc` 都要有对应的 `free`
4. **避免悬空指针**：释放后将指针设为 `nil`
5. **优先使用值传递**：除非需要修改原值或性能敏感

---

## 示例

### 链表实现

```vix
extern "C" {
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
}

struct Node {
    value: i32,
    next: &Node
}

fn createNode(value: i32) -> &Node {
    let node = malloc(16)  // 假设结构体大小为 16 字节
    @node.value = value
    @node.next = nil
    return node
}

fn append(head: &Node, value: i32) -> &Node {
    let newNode = createNode(value)
    let current = head
    
    while (@current.next != nil) {
        current = @current.next
    }
    @current.next = newNode
    return head
}

fn printList(head: &Node) {
    let current = head
    while (current != nil) {
        printf("%d -> ", @current.value)
        current = @current.next
    }
    printf("nil\n")
}
```

---

## 下一步

- [函数](functions.md) - 函数参数传递
- [结构体](structs.md) - 结构体与指针
- [标准库](stdlib.md) - 内存管理函数
