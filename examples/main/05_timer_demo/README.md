# 05_timer_demo

该示例展示如何在 `tbox::main::Module` 中使用事件循环定时触发任务。模块会定期打印一条可配置的日志，用于演示：

- 如何填充默认配置 (`onFillDefaultConfig`)
- 如何在初始化阶段解析配置并创建 `TimerEvent`
- 如何在 `onStart`/`onStop` 生命周期中启用与停用事件

## 构建

```bash
make
```

## 运行

直接运行可使用默认配置，每秒输出一次 `Tick from cpp-tbox`：

```bash
./05_timer_demo
```

也可以通过命令行覆盖配置项，例如修改定时间隔与输出内容：

```bash
./05_timer_demo -s 'timer_demo.interval_ms=500' -s 'timer_demo.message="Hello cpp-tbox"'
```
