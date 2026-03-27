# 控制流

控制流语句用于控制程序的执行顺序。Vix 提供了丰富的控制流结构。

## 目录

- [条件语句](#条件语句)
- [while 循环](#while-循环)
- [for 循环](#for-循环)
- [break 和 continue](#break-和-continue)
- [match 语句](#match-语句)
- [逻辑运算](#逻辑运算)

---

## 条件语句

### if 语句

```vix
let x = 10

if (x > 5) {
    print("x is greater than 5")
}
```

### if-else 语句

```vix
let x = 10

if (x > 15) {
    print("x is greater than 15")
} else {
    print("x is 15 or less")
}
```

### if-elif-else 语句

```vix
let score = 85

if (score >= 90) {
    print("Grade: A")
} elif (score >= 80) {
    print("Grade: B")
} elif (score >= 70) {
    print("Grade: C")
} elif (score >= 60) {
    print("Grade: D")
} else {
    print("Grade: F")
}
```

### 嵌套 if 语句

```vix
let age = 25
let hasLicense = true

if (age >= 18) {
    if (hasLicense) {
        print("You can drive")
    } else {
        print("You need a license to drive")
    }
} else {
    print("You are too young to drive")
}
```

### 条件表达式

条件可以是任意返回布尔值的表达式：

```vix
let a = 10
let b = 20

if (a < b and b < 30) {
    print("Both conditions are true")
}

if (a == 10 or b == 10) {
    print("At least one condition is true")
}

if (!(a > b)) {
    print("a is not greater than b")
}
```

---

## while 循环

while 循环在条件为真时重复执行代码块。

### 基本用法

```vix
let i = 0
while (i < 5) {
    print(i)
    i += 1
}
// 输出: 0 1 2 3 4
```

### 无限循环

```vix
while (true) {
    // 无限循环
    // 使用 break 退出
}
```

### 带条件的无限循环

```vix
let running = true
while (running) {
    let input = getInput()
    if (input == "quit") {
        running = false
    }
}
```

### while 与用户交互

```vix
let guess = 0
let target = 42

while (guess != target) {
    guess = toint(input("Guess a number: "))
    if (guess < target) {
        print("Too low!")
    } elif (guess > target) {
        print("Too high!")
    }
}
print("Correct!")
```

---

## for 循环

for 循环用于遍历范围或集合。

### 范围循环

```vix
// 遍历 1 到 9（不包括 10）
for (i in 1 .. 10) {
    print(i)
}
// 输出: 1 2 3 4 5 6 7 8 9
```

### 从零开始的循环

```vix
for (i in 0 .. 5) {
    print(i)
}
// 输出: 0 1 2 3 4
```

### 遍历数组

```vix
let arr = [10, 20, 30, 40, 50]

for (i in 0 .. arr.length) {
    print("arr[", i, "] = ", arr[i])
}
```

### 嵌套循环

```vix
// 打印乘法表
for (i in 1 .. 10) {
    for (j in 1 .. 10) {
        printf("%4d", i * j)
    }
    print("")
}
```

### 循环变量修改

```vix
for (i in 0 .. 10) {
    if (i % 2 == 0) {
        print(i)  // 只打印偶数
    }
}
```

---

## break 和 continue

### break 语句

`break` 用于立即退出循环：

```vix
let i = 0
while (true) {
    print(i)
    i += 1
    if (i >= 5) {
        break  // 退出循环
    }
}
// 输出: 0 1 2 3 4
```

在 for 循环中使用：

```vix
for (i in 1 .. 100) {
    if (i == 5) {
        break  // 在 i == 5 时退出
    }
    print(i)
}
// 输出: 1 2 3 4
```

查找元素：

```vix
let arr = [3, 7, 2, 9, 5]
let target = 9
let found = -1

for (i in 0 .. arr.length) {
    if (arr[i] == target) {
        found = i
        break  // 找到后立即退出
    }
}

if (found >= 0) {
    print("Found at index: ", found)
} else {
    print("Not found")
}
```

### continue 语句

`continue` 用于跳过当前迭代，继续下一次循环：

```vix
// 只打印奇数
for (i in 1 .. 10) {
    if (i % 2 == 0) {
        continue  // 跳过偶数
    }
    print(i)
}
// 输出: 1 3 5 7 9
```

过滤数据：

```vix
let numbers = [1, -2, 3, -4, 5, -6]

for (i in 0 .. numbers.length) {
    if (numbers[i] < 0) {
        continue  // 跳过负数
    }
    print(numbers[i])
}
// 输出: 1 3 5
```

### 组合使用

```vix
for (i in 0 .. 100) {
    if (i % 3 == 0) {
        continue  // 跳过 3 的倍数
    }
    if (i > 20) {
        break  // 超过 20 就停止
    }
    print(i)
}
```

---

## match 语句

match 语句用于模式匹配：

### 基本用法

```vix
let value = 10

match value {
    5 -> {
        print("value is 5")
    }
    10 -> {
        print("value is 10")
    }
    15 -> {
        print("value is 15")
    }
    _ -> {
        print("value is something else")
    }
}
```

### 匹配枚举值

```vix
type State = Running | Stopped | Paused

let state = Running

match state {
    Running -> {
        print("System is running")
    }
    Stopped -> {
        print("System is stopped")
    }
    Paused -> {
        print("System is paused")
    }
}
```

### 带返回值的 match

```vix
fn describe(n: i32) -> string {
    match n {
        0 -> {
            return "zero"
        }
        1 -> {
            return "one"
        }
        _ -> {
            return "many"
        }
    }
}
```

---

## 逻辑运算

### and 运算符

两个条件都为真时返回真：

```vix
let age = 25
let hasID = true

if (age >= 18 and hasID) {
    print("You can enter")
}
```

短路求值：

```vix
let x = 0
if (x != 0 and 10 / x > 1) {
    // 如果 x != 0 为假，后面的表达式不会求值
    // 避免除零错误
}
```

### or 运算符

至少一个条件为真时返回真：

```vix
let day = "Saturday"

if (day == "Saturday" or day == "Sunday") {
    print("It's the weekend!")
}
```

### not 运算符

取反布尔值：

```vix
let isEmpty = false

if (!isEmpty) {
    print("There are items")
}
```

### 复合逻辑表达式

```vix
let a = 10
let b = 20
let c = 30

if ((a < b and b < c) or a == 0) {
    print("Complex condition is true")
}
```

---

## 控制流最佳实践

### 1. 避免过深的嵌套

```vix
// 不推荐
if (condition1) {
    if (condition2) {
        if (condition3) {
            // 太深了
        }
    }
}

// 推荐：使用提前返回
if (!condition1) {
    return
}
if (!condition2) {
    return
}
if (!condition3) {
    return
}
// 主要逻辑
```

### 2. 使用有意义的条件变量

```vix
// 不推荐
if (age >= 18 and hasLicense and !isSuspended) {
    // ...
}

// 推荐
let canDrive = age >= 18 and hasLicense and !isSuspended
if (canDrive) {
    // ...
}
```

### 3. 选择合适的循环类型

- 使用 `for` 循环遍历已知范围
- 使用 `while` 循环处理不确定的迭代次数

### 4. 避免无限循环

```vix
// 危险：没有退出条件
while (true) {
    // 忘记 break
}

// 安全：明确的退出条件
let running = true
while (running) {
    // ...
    if (shouldStop) {
        running = false
    }
}
```

---

## 示例

### 二分查找

```vix
fn binarySearch(arr: [i32], target: i32, low: i32, high: i32) -> i32 {
    while (low <= high) {
        let mid = (low + high) / 2
        if (arr[mid] == target) {
            return mid
        } elif (arr[mid] < target) {
            low = mid + 1
        } else {
            high = mid - 1
        }
    }
    return -1
}
```

### 冒泡排序

```vix
fn bubbleSort(arr: [i32], size: i32) {
    for (i in 0 .. size - 1) {
        for (j in 0 .. size - i - 1) {
            if (arr[j] > arr[j + 1]) {
                let temp = arr[j]
                arr[j] = arr[j + 1]
                arr[j + 1] = temp
            }
        }
    }
}
```

### 猜数字游戏

```vix
fn game() -> i32 {
    let target = 42
    let guesses = 0
    
    while (true) {
        let guess = toint(input("Guess: "))
        guesses += 1
        
        if (guess == target) {
            print("Correct! You got it in ", guesses, " guesses!")
            return 0
        } elif (guess < target) {
            print("Too low!")
        } else {
            print("Too high!")
        }
    }
}
```

---

## 下一步

- [函数](functions.md) - 函数定义和调用
- [数组与列表](syntax.md#数组与列表) - 数据结构
- [指针](pointers.md) - 指针操作
