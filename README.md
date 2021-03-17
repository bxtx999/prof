# Prof

Self-contained C/C++ profiler library for Linux

## 准备

- 安装`perf`相关内容。

    ```text
    $ sudo apt install linux-tools-common linux-tools-generic linux-tools-`uname -r`
    ```

- 设置`kernel`中的`perf_event_paranoid`：

    ```text
    echo 1 | sudo tee /proc/sys/kernel/perf_event_paranoid
    ```
  
## Prof API

1. `PROF_START()`

    重置计数器并且开始（重新开始）事件计数。
   
    在包含这个文件之前，通过设置`PROF_EVENT_LIST`宏来指定要监控的事件，使其成为`PROF_EVENT_*`调用的列表；默认为计算CPU时钟周期数。
     
    如果在包含这个文件之前定义了`PROF_USER_EVENTS_ONLY`宏，那么内核和hypervisor事件将被排除在计数之外。

2. `PROF_EVENT(type, config)`

    指定一个要监控的事件，类型和配置在 `perf_event_open` 系统调用的文档中定义。

3. `PROF_EVENT_HW(config)`

    与`PROF_EVENT`相同，但适用于硬件事件；`config`中必须省略`PERF_COUNT_HW_`前缀。
   
4. `PROF_EVENT_SW(config)`

    与`PROF_EVENT`相同，但适用于硬件事件；`config`中必须省略`PERF_COUNT_SW_`前缀。

5. `PROF_EVENT_CACHE(cache, op, result)`

    与`PROF_EVENT`相同，但用于缓存事件。
    前缀`PERF_COUNT_HW_CACHE_`、`PERF_COUNT_HW_CACHE_OP_`和`PERF_COUNT_HW_CACHE_RESULT_`必须分别省略`cache`、`OP`和`result`。
    同样，`cache`、`op`和`result`在`perf_event_open`系统调用的文档中定义。

6. `PROF_STOP()`

    停止对事件的计数。然后可以使用`PROF_COUNTERS`访问计数器数组。

7. `PROF_COUNTERS`

    访问计数器数组。计数器的顺序与`PROF_EVENT_LIST`中定义的事件相同。这个数组的元素是64位无符号整数。

8. `PROF_DO(block)`

    停止对事件的计数，对每个事件执行`block`提供的代码。
    
    在代码中：
    
    - `index`指的是`PROF_COUNTERS`定义的计数器数组中的事件位置索引。
    - `counter`是计数器的实际值。
    - `index`是一个64位无符号整数。

9. `PROF_CALL(callback)`

    与`PROF_DO`相同，只是`callback`是一个可调用对象的名称(例如一个函数)。
    
    对于每一个事件，它的两个参数`index`和`counter`被调用。

10. `PROF_FILE(file)`

    停止收集对事件计数，并将与`PROF_EVENT_LIST`中事件相同的行数写入写入`file`(stdio.h 的 `FILE *`类型)。
    
    每一行都包含 `index` 和计数器（由`PROF_DO`定义），并由制表符分隔。如果只有一个事件，那么index会省略。

11. `PROF_STDOUT()`

    与 `PROF_FILE` 一样，不过`file`是`stdout`类型。

12 `PROF_STDERR()`

    与 `PROF_FILE` 一样，不过`file`是`stderr`类型。

## 使用

1. Test:

   ```shell
   $ sudo ./header_test

   行优先耗时：0.008733 ms
   列优先耗时：0.011547 ms

   20106146  # CPU cycles

   26545355  # CPU cycles

   ```

2. 测试其他事件

   ```shell
   $ sudo ./header_test_custom
   行优先耗时：0.005951 ms
   列优先耗时：0.007836 ms
   Total L1 faults_slow: R = 33403890; W = 0
   Total L1 faults_fast: R = 49791176; W = 0
   ```

## 其他

1. `perf_event_paranoid`

   Controls use of the performance events system by unprivileged users (without `CAP_SYS_ADMIN`)

   - `-1`: Allow use of (almost) all events by all users Ignore mlock limit after `perf_event_mlock_kb` without `CAP_IPC_LOCK`

   - `>=0`：Disallow ftrace function tracepoint by users without `CAP_SYS_ADMIN` Disallow raw tracepoint access by users without `CAP_SYS_ADMIN`

   - `>=1`：Disallow CPU event access by users without `CAP_SYS_ADMIN`

   - `>=2`：Disallow kernel profiling by users without `CAP_SYS_ADMIN`


## 参考

1. https://github.com/cyrus-and/prof

2. http://www.brendangregg.com/perf.html

3. https://perf.wiki.kernel.org/index.php/Main_Page

4. https://perf.wiki.kernel.org/index.php/Tutorial

5. https://man7.org/linux/man-pages/man2/perf_event_open.2.html
