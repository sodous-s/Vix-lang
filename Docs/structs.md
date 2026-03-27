# 结构体

结构体（Struct）是 Vix 中用于组织相关数据的复合类型。它允许你将多个不同类型的值组合成一个有意义的整体。

## 目录

- [定义结构体](#定义结构体)
- [创建实例](#创建实例)
- [访问字段](#访问字段)
- [修改字段](#修改字段)
- [嵌套结构体](#嵌套结构体)
- [结构体方法](#结构体方法)
- [泛型结构体](#泛型结构体)
- [结构体作为参数](#结构体作为参数)
- [最佳实践](#最佳实践)

---

## 定义结构体

### 基本语法

```vix
struct StructName {
    field1: type1,
    field2: type2,
    field3: type3
}
```

### 示例

```vix
struct Person {
    name: string,
    age: i32,
    height: f64
}

struct Book {
    title: string,
    author: string,
    year: i32,
    pages: i32
}

struct Point {
    x: f64,
    y: f64
}
```

### 公开结构体

使用 `pub` 关键字使结构体可导出：

```vix
pub struct Person {
    name: string,
    age: i32
}
```

---

## 创建实例

### 使用初始化列表

```vix
let person = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}

let book = Book {
    title: "The Vix Guide",
    author: "Vix Team",
    year: 2024,
    pages: 350
}
```

### 逐字段赋值

```vix
let person: Person {}
person.name = "Bob"
person.age = 30
person.height = 6.0
```

### 使用变量初始化

```vix
let name = "Charlie"
let age = 28
let height = 5.9

let person = Person {
    name: name,
    age: age,
    height: height
}
```

---

## 访问字段

使用 `.` 运算符访问结构体字段：

```vix
let person = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}

print(person.name)     // "Alice"
print(person.age)      // 25
print(person.height)   // 5.7
```

### 链式访问

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

let person = Person {
    name: "Alice",
    address: Address {
        street: "123 Main St",
        city: "Anytown",
        zip: "12345"
    }
}

print(person.address.city)    // "Anytown"
print(person.address.street)  // "123 Main St"
```

---

## 修改字段

### 可变实例

使用 `mut` 声明可修改的结构体实例：

```vix
mut person = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}

person.age = 26           // 修改年龄
person.height = 5.8       // 修改身高

print(person.age)         // 26
print(person.height)      // 5.8
```

### 计算并修改字段

```vix
mut rect = Rectangle {
    width: 10.0,
    height: 5.0
}

// 修改字段值
rect.width = rect.width * 2
rect.height = rect.height + 2.0

print(rect.width)   // 20.0
print(rect.height)  // 7.0
```

---

## 嵌套结构体

结构体可以包含其他结构体作为字段：

```vix
struct Date {
    year: i32,
    month: i32,
    day: i32
}

struct Event {
    name: string,
    date: Date,
    location: string
}

let event = Event {
    name: "Vix Conference",
    date: Date {
        year: 2024,
        month: 6,
        day: 15
    },
    location: "San Francisco"
}

print(event.name)           // "Vix Conference"
print(event.date.year)      // 2024
print(event.date.month)     // 6
print(event.location)       // "San Francisco"
```

### 深层嵌套

```vix
struct Country {
    name: string,
    code: string
}

struct City {
    name: string,
    country: Country
}

struct Address {
    street: string,
    city: City
}

struct Person {
    name: string,
    address: Address
}

let person = Person {
    name: "John",
    address: Address {
        street: "123 Main St",
        city: City {
            name: "New York",
            country: Country {
                name: "United States",
                code: "US"
            }
        }
    }
}

print(person.address.city.country.name)  // "United States"
```

---

## 结构体方法

Vix 通过函数操作结构体：

### 基本方法模式

```vix
struct Rectangle {
    width: f64,
    height: f64
}

// 计算面积
fn area(rect: Rectangle) -> f64 {
    return rect.width * rect.height
}

// 计算周长
fn perimeter(rect: Rectangle) -> f64 {
    return 2.0 * (rect.width + rect.height)
}

// 使用
let rect = Rectangle { width: 10.0, height: 5.0 }
print(area(rect))       // 50.0
print(perimeter(rect))  // 30.0
```

### 修改结构体的方法

```vix
struct Counter {
    value: i32
}

fn increment(counter: &Counter) {
    @counter.value = @counter.value + 1
}

fn reset(counter: &Counter) {
    @counter.value = 0
}

// 使用
mut counter = Counter { value: 0 }
increment(&counter)
print(counter.value)  // 1
reset(&counter)
print(counter.value)  // 0
```

### 工厂函数

```vix
struct Point {
    x: f64,
    y: f64
}

fn newPoint(x: f64, y: f64) -> Point {
    return Point { x: x, y: y }
}

fn origin() -> Point {
    return Point { x: 0.0, y: 0.0 }
}

// 使用
let p1 = newPoint(3.0, 4.0)
let p2 = origin()
```

---

## 泛型结构体

结构体可以使用泛型参数：

### 定义泛型结构体

```vix
struct Box:[T] {
    value: T
}

struct Pair:[T, U] {
    first: T,
    second: U
}
```

### 实例化泛型结构体

```vix
// 指定具体类型
let intBox = Box:[i32]{ value: 42 }
let floatBox = Box:[f64]{ value: 3.14 }
let strBox = Box:[string]{ value: "Hello" }

// 多个泛型参数
let pair = Pair:[i32, string]{
    first: 1,
    second: "one"
}
```

### 泛型结构体方法

```vix
struct Box:[T] {
    value: T
}

fn get:[T](box: Box:[T]) -> T {
    return box.value
}

fn set:[T](box: &Box:[T], newValue: T) {
    @box.value = newValue
}

// 使用
let box = Box:[i32]{ value: 42 }
print(get:[i32](box))  // 42
```

---

## 结构体作为参数

### 值传递

```vix
fn printPerson(p: Person) {
    print("Name: ", p.name)
    print("Age: ", p.age)
}

let person = Person { name: "Alice", age: 25 }
printPerson(person)
```

### 指针传递

```vix
fn updateAge(p: &Person, newAge: i32) {
    @p.age = newAge
}

mut person = Person { name: "Alice", age: 25 }
updateAge(&person, 26)
print(person.age)  // 26
```

### 返回结构体

```vix
fn createPerson(name: string, age: i32) -> Person {
    return Person {
        name: name,
        age: age,
        height: 5.7
    }
}

let person = createPerson("Bob", 30)
```

---

## 最佳实践

### 1. 有意义的命名

```vix
// 推荐：描述性命名
struct UserProfile {
    username: string,
    email: string,
    createdAt: i64
}

// 不推荐：缩写或不清晰的命名
struct UP {
    u: string,
    e: string,
    c: i64
}
```

### 2. 相关字段组织

```vix
// 推荐：相关字段组织在一起
struct Address {
    street: string,
    city: string,
    state: string,
    zip: string,
    country: string
}

struct User {
    name: string,
    email: string,
    address: Address  // 使用嵌套结构体
}
```

### 3. 使用工厂函数

```vix
// 推荐：提供创建实例的便捷方法
fn newUser(name: string, email: string) -> User {
    return User {
        name: name,
        email: email,
        active: true
    }
}
```

### 4. 不可变优先

```vix
// 默认使用不可变
let person = Person { name: "Alice", age: 25 }

// 只在需要修改时使用 mut
mut counter = Counter { value: 0 }
```

---

## 示例

### 图形库

```vix
struct Point {
    x: f64,
    y: f64
}

struct Size {
    width: f64,
    height: f64
}

struct Rectangle {
    origin: Point,
    size: Size
}

fn area(rect: Rectangle) -> f64 {
    return rect.size.width * rect.size.height
}

fn containsPoint(rect: Rectangle, p: Point) -> bool {
    return p.x >= rect.origin.x and
           p.x <= rect.origin.x + rect.size.width and
           p.y >= rect.origin.y and
           p.y <= rect.origin.y + rect.size.height
}
```

### 链表节点

```vix
struct Node:[T] {
    value: T,
    next: &Node:[T]
}

fn createNode:[T](value: T) -> Node:[T] {
    return Node:[T]{
        value: value,
        next: nil
    }
}
```

---

## 下一步

- [函数](functions.md) - 函数定义
- [泛型](types.md#泛型类型) - 泛型类型
- [指针](pointers.md) - 指针操作
