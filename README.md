用于操作系统监控的agent
## 1. 修改Makefile
### 1.1 操作系统类型

+ linux系统:
```
LDFLAGS = -g -DLINUX -lpthread -DDEBUG
```
+ MAC系统：

```
LDFLAGS = -g -DMAC -lpthread
```

### 1.2 修改本机监听IP及port

```
./$(SERVER) -h <侦听IP地址> -p <端口> -b 100

```

## 2. 编译

```
make
```

## 3. 运行

```
make run
```


----
## 4. client
java client协议，参数Client.java
