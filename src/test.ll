; ModuleID = 'VixModule'
source_filename = "VixModule"
target triple = "x86_64-pc-linux-gnu"

declare i32 @printf(ptr, ...)

declare i64 @strlen(ptr)

define i32 @socket(i32 %domain, i32 %type, i32 %protocol) {
entry:
  %domain1 = alloca i32, align 4
  store i32 %domain, ptr %domain1, align 4
  %type2 = alloca i32, align 4
  store i32 %type, ptr %type2, align 4
  %protocol3 = alloca i32, align 4
  store i32 %protocol, ptr %protocol3, align 4
  %domain4 = load i32, ptr %domain1, align 4
  %type5 = load i32, ptr %type2, align 4
  %protocol6 = load i32, ptr %protocol3, align 4
  %calltmp = call i32 @socket(i32 %domain4, i32 %type5, i32 %protocol6)
  ret i32 %calltmp

func_end:                                         ; No predecessors!
  ret i32 0
}

define i32 @listen(i32 %sockfd, i32 %backlog) {
entry:
  %sockfd1 = alloca i32, align 4
  store i32 %sockfd, ptr %sockfd1, align 4
  %backlog2 = alloca i32, align 4
  store i32 %backlog, ptr %backlog2, align 4
  %sockfd3 = load i32, ptr %sockfd1, align 4
  %backlog4 = load i32, ptr %backlog2, align 4
  %calltmp = call i32 @listen(i32 %sockfd3, i32 %backlog4)
  ret i32 %calltmp

func_end:                                         ; No predecessors!
  ret i32 0
}

define void @accept(i32 %sockfd, i32 %addr, i32 %addrlen) {
entry:
  %sockfd1 = alloca i32, align 4
  store i32 %sockfd, ptr %sockfd1, align 4
  %addr2 = alloca i32, align 4
  store i32 %addr, ptr %addr2, align 4
  %addrlen3 = alloca i32, align 4
  store i32 %addrlen, ptr %addrlen3, align 4
  %sockfd4 = load i32, ptr %sockfd1, align 4
  %addr5 = load i32, ptr %addr2, align 4
  %addrlen6 = load i32, ptr %addrlen3, align 4
  call void @accept(i32 %sockfd4, i32 %addr5, i32 %addrlen6)
  ret void <badref>

func_end:                                         ; No predecessors!
  ret void
}

define i32 @send(i32 %sockfd, ptr %buf, i32 %len, i32 %flags) {
entry:
  %sockfd1 = alloca i32, align 4
  store i32 %sockfd, ptr %sockfd1, align 4
  %buf2 = alloca ptr, align 8
  store ptr %buf, ptr %buf2, align 8
  %len3 = alloca i32, align 4
  store i32 %len, ptr %len3, align 4
  %flags4 = alloca i32, align 4
  store i32 %flags, ptr %flags4, align 4
  %sockfd5 = load i32, ptr %sockfd1, align 4
  %buf6 = load ptr, ptr %buf2, align 8
  %len7 = load i32, ptr %len3, align 4
  %flags8 = load i32, ptr %flags4, align 4
  %calltmp = call i32 @send(i32 %sockfd5, ptr %buf6, i32 %len7, i32 %flags8)
  ret i32 %calltmp

func_end:                                         ; No predecessors!
  ret i32 0
}

define i32 @close(i32 %fd) {
entry:
  %fd1 = alloca i32, align 4
  store i32 %fd, ptr %fd1, align 4
  %fd2 = load i32, ptr %fd1, align 4
  %calltmp = call i32 @close(i32 %fd2)
  ret i32 %calltmp

func_end:                                         ; No predecessors!
  ret i32 0
}

define void @main() {
entry:
  ret i32 0

func_end:                                         ; No predecessors!
  ret void
}
