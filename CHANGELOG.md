### 26/1/24 更新日志

1. 更新了错误提示
2. 更新了qbe ir的生成，从vic ir到qbe ir ，而不是从bytcode到qbe ir
3. 更新const关键字（包括错误提示）
4. 修复了toint/tofloat函数的cpp代码生成

- will do
  修复input的seg bug

### 26/1/25 更新日志

1. 更新列表

- will do
  指针

### 26/1/26 更新日志

1. 更新列表操作(add,remove,pop,push)

- will do
  指针和结构体

### 26/1/27 更新日志

1. 修复对于pop操作的bug
2. 更新结构体语法
3. 更新函数参数类型后置语法
4. 更新结构体的错误提示

- will do
  指针

### 26/2/5 更新日志
1. 添加指针语法
2. 添加指针错误提示
3. 基本支持qbe后端
4. qbe后端的opt未完善，可能有优化问题

- will do
  完善qbe后端

### 26/2/6 更新日志
1. 实现嵌套结构体的cpp代码生成

- will do
  完善qbe后端

### 26/2/7 更新日志
1. 修改struct的初始化语法 ' : '变为  ' = '
2. 更新for循环的语法 可以 for(i ; 1 .. 10)或for(i in 1 .. 10)

- will do
  完善qbe后端

## 26/2/8 更新日志
1. 支持列表类型和相关操作
2. 实现列表作为函数参数和返回值的语法支持，包括列表的基本操作如列表的索引、添加、删除等。同时新增了字节码指令来处理列表相关的操作
   并更新了AST节点类型以支持列表类型定义

- will do
  完善qbe后端

## 25/2/12 更新日志
1. 添加对于空指针的支持

- will do
  完善llvm后端

## 25/2/17 更新日志
1. 移除emit.c文件中的VIC IR到LLVM IR转换实现
2. 由于项目架构调整，删除了原来的emit.c文件中完整的vic ir到llvm ir转换功能实现。该文件包含了从vic中间表示解析并生成llvm ir的完整逻辑

## 25/2/19 更新日志
1. [Update]: 添加import和pub关键字支持添加IMPORT和PUB token定义到词法分析器中，并在语法分析器中实现相应的语法规则，支持导入模块和声明公共函数的功能。

2. [Update]: 实现import节点和公共函数相关AST节点

3. [Fix]: 实现模块导入功能在main.c中集成模块导入处理，在AST中添加import节点的解析、打印和释放功能，以及inline_imports函数来内联导的模块内容。

4. [Update]: 添加import语义检查功能实现对import语句的语义检查，包括模块文件存在性验证和从被导入模块中提取公共函数符号到符号表中的功能

5. [Fix]: 改进unaryop操作符显示修复print_ast函数中的unaryop操作符显示逻辑

6. [Fix]: 清除LLVM构建器插入点在函数完成后清除LLVM构建器的插入点，防止后续代码在已完成的函数基本块中继续生成代码。

7. [Update]: 更新相应的import测试文件 如[examples/import_test.vix]