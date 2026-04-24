# GlobalFixedTimeStep 插件用户手册

**版本：** 1.0  
**引擎：** Unreal Engine 5  
**作者：** DGOne

---

## 目录

1. [概述](#1-概述)
2. [安装](#2-安装)
3. [项目设置](#3-项目设置)
4. [Blueprint 使用](#4-blueprint-使用)
5. [C++ 使用](#5-c-使用)
6. [API 参考](#6-api-参考)
7. [工作原理](#7-工作原理)
8. [常见场景](#8-常见场景)
9. [注意事项](#9-注意事项)

---

## 1. 概述

GlobalFixedTimeStep 插件为 Unreal Engine 5 提供一个**全局固定时间步长更新系统**，类似 Unity 的 `FixedUpdate`。

**核心特性：**

- 以固定频率（默认 60Hz）触发 `OnFixedTick` 事件，与帧率无关
- 经典累加器模式（Accumulator Pattern），物理模拟精度稳定
- 每帧最大子步数限制，防止"螺旋死亡"（Spiral of Death）
- 提供插值 Alpha（0~1），用于渲染层在两次固定步之间平滑插值
- 无需手动放置 Actor，每个 World 自动创建子系统实例
- 完整 Blueprint 支持：事件绑定、运行时配置、只读查询

---

## 2. 安装

1. 将 `GlobalFixedTimeStep` 文件夹复制到项目的 `Plugins/` 目录下
2. 重新生成项目的 Visual Studio 解决方案文件
3. 编译项目
4. 在编辑器中打开 **Edit → Plugins**，确认插件已启用

目录结构：

```
YourProject/
└── Plugins/
    └── GlobalFixedTimeStep/
        ├── GlobalFixedTimeStep.uplugin
        └── Source/
            └── GlobalFixedTimeStep/
```

---

## 3. 项目设置

打开 **Edit → Project Settings → Plugins → Global Fixed TimeStep**，可配置以下默认参数：

| 参数 | 类型 | 默认值 | 范围 | 说明 |
|------|------|--------|------|------|
| Fixed Time Step | float | 0.01667s（1/60） | 0.0001 ~ 1.0 | 每次固定 Tick 的时间间隔（秒） |
| Max Sub Steps Per Frame | int32 | 8 | 1 ~ 64 | 每帧最多触发的子步数 |
| Enable Fixed Tick | bool | true | — | 全局开关，关闭后系统停止 Tick |

这些设置保存在 `DefaultGame.ini` 中，作为运行时的初始值。运行时可通过子系统 API 动态覆盖。

---

## 4. Blueprint 使用

### 4.1 获取子系统

在任意 Blueprint 中，使用 **Get World Subsystem** 节点获取子系统实例：

```
Get World Subsystem → Class: GlobalFixedTimeStepSubsystem → 返回 UGlobalFixedTimeStepSubsystem
```

推荐在 `Event BeginPlay` 中缓存引用：

```
Event BeginPlay
  → Get World Subsystem (GlobalFixedTimeStepSubsystem)
  → Set Variable [FixedTimeStepSubsystem]
```

### 4.2 绑定固定 Tick 事件

```
Event BeginPlay
  → Get World Subsystem (GlobalFixedTimeStepSubsystem)
  → Assign On Fixed Tick
      → [自定义事件节点，输入: Fixed Delta Time (float)]
          → [你的物理/游戏逻辑]
```

`Fixed Delta Time` 引脚的值等于设置中的 Fixed Time Step，每次固定步均相同。

### 4.3 绑定帧结束事件

```
Get World Subsystem (GlobalFixedTimeStepSubsystem)
  → Assign On Fixed Tick Finished
      → [自定义事件节点]
          → [插值/渲染更新逻辑]
```

`On Fixed Tick Finished` 在当帧所有子步完成后触发一次（若当帧无子步则不触发）。

### 4.4 使用插值 Alpha 平滑渲染

```
Event Tick (Delta Seconds)
  → Get World Subsystem (GlobalFixedTimeStepSubsystem)
  → Get Interpolation Alpha  →  [0.0 ~ 1.0]
  → Lerp (A=上一帧物理位置, B=当前物理位置, Alpha=插值Alpha)
  → Set Actor Location
```

### 4.5 运行时修改配置

```
[按键事件]
  → Get World Subsystem (GlobalFixedTimeStepSubsystem)
  → Set Fixed Time Step (New Fixed Time Step = 0.02)   ← 切换到 50Hz
```

### 4.6 加载关卡后重置累加器

```
Event Begin Play (新关卡)
  → Get World Subsystem (GlobalFixedTimeStepSubsystem)
  → Reset Accumulator
```

---

## 5. C++ 使用

### 5.1 获取子系统

```cpp
#include "GlobalFixedTimeStepSubsystem.h"

UGlobalFixedTimeStepSubsystem* FixedStepSys =
    GetWorld()->GetSubsystem<UGlobalFixedTimeStepSubsystem>();
```

### 5.2 绑定事件

```cpp
// 在 BeginPlay 或 Initialize 中
if (UGlobalFixedTimeStepSubsystem* Sys = GetWorld()->GetSubsystem<UGlobalFixedTimeStepSubsystem>())
{
    Sys->OnFixedTick.AddDynamic(this, &UMyComponent::OnFixedTick);
    Sys->OnFixedTickFinished.AddDynamic(this, &UMyComponent::OnFixedTickFinished);
}

// 回调函数
void UMyComponent::OnFixedTick(float FixedDeltaTime)
{
    // 物理模拟、游戏逻辑
}

void UMyComponent::OnFixedTickFinished()
{
    // 插值、渲染更新
}
```

### 5.3 解绑事件

```cpp
// 在 EndPlay 或 BeginDestroy 中
if (UGlobalFixedTimeStepSubsystem* Sys = GetWorld()->GetSubsystem<UGlobalFixedTimeStepSubsystem>())
{
    Sys->OnFixedTick.RemoveDynamic(this, &UMyComponent::OnFixedTick);
    Sys->OnFixedTickFinished.RemoveDynamic(this, &UMyComponent::OnFixedTickFinished);
}
```

### 5.4 运行时配置

```cpp
UGlobalFixedTimeStepSubsystem* Sys = GetWorld()->GetSubsystem<UGlobalFixedTimeStepSubsystem>();
if (Sys)
{
    Sys->SetFixedTimeStep(1.0f / 30.0f);  // 切换到 30Hz
    Sys->SetMaxSubStepsPerFrame(4);
    Sys->SetEnabled(false);               // 暂停固定 Tick
    Sys->ResetAccumulator();              // 清空累加器
}
```

### 5.5 读取状态

```cpp
float Alpha = Sys->GetInterpolationAlpha();   // 0~1，用于渲染插值
int64 TotalTicks = Sys->GetFixedTickCount();  // 自 World 启动以来的总固定 Tick 数
int32 StepsThisFrame = Sys->GetSubStepsThisFrame(); // 本帧触发的子步数
```

---

## 6. API 参考

### UGlobalFixedTimeStepSubsystem

#### 事件（BlueprintAssignable）

| 名称 | 签名 | 触发时机 |
|------|------|----------|
| `OnFixedTick` | `(float FixedDeltaTime)` | 每个固定子步触发一次，每帧可触发 N 次 |
| `OnFixedTickFinished` | `()` | 当帧所有子步完成后触发一次（无子步时不触发） |

#### 运行时配置（BlueprintCallable）

| 函数 | 参数 | 说明 |
|------|------|------|
| `SetFixedTimeStep` | `float NewFixedTimeStep` | 设置固定步长（秒），自动钳制到 [0.0001, 1.0]，**重置累加器** |
| `SetMaxSubStepsPerFrame` | `int32 NewMaxSubSteps` | 设置每帧最大子步数，钳制到 [1, 64] |
| `SetEnabled` | `bool bNewEnabled` | 启用/禁用系统，从禁用切换到启用时**自动重置累加器** |
| `ResetAccumulator` | — | 清零累加器和插值 Alpha |

#### 只读查询（BlueprintPure）

| 函数 | 返回类型 | 说明 |
|------|----------|------|
| `GetFixedTimeStep` | `float` | 当前固定步长（秒） |
| `GetMaxSubStepsPerFrame` | `int32` | 当前每帧最大子步数 |
| `IsEnabled` | `bool` | 系统是否启用 |
| `GetInterpolationAlpha` | `float` | 渲染插值 Alpha（0~1） |
| `GetFixedTickCount` | `int64` | 自 World 启动以来的总固定 Tick 数 |
| `GetSubStepsThisFrame` | `int32` | 本帧触发的子步数 |

### UGlobalFixedTimeStepSettings

| 属性 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `FixedTimeStep` | `float` | `1/60` | 固定步长（秒） |
| `MaxSubStepsPerFrame` | `int32` | `8` | 每帧最大子步数 |
| `bEnableFixedTick` | `bool` | `true` | 全局启用开关 |

---

## 7. 工作原理

### 累加器模式

每帧 `Tick(DeltaTime)` 执行以下逻辑：

```
Accumulator += DeltaTime
Accumulator = min(Accumulator, FixedTimeStep × MaxSubSteps)  // 防止螺旋死亡

while Accumulator >= FixedTimeStep:
    广播 OnFixedTick(FixedTimeStep)
    Accumulator -= FixedTimeStep

InterpolationAlpha = Accumulator / FixedTimeStep  // 0~1
if 本帧有子步:
    广播 OnFixedTickFinished()
```

### 螺旋死亡保护

当游戏出现严重卡顿（如加载资源、GC），`DeltaTime` 可能远大于 `FixedTimeStep`。若不加限制，累加器会触发大量子步，导致下一帧更慢，形成正反馈循环。

插件通过将累加器上限钳制为 `FixedTimeStep × MaxSubStepsPerFrame` 来打破这一循环。默认配置（60Hz，最大 8 步）意味着单帧最多处理 133ms 的固定步，超出部分直接丢弃。

### 插值 Alpha

`GetInterpolationAlpha()` 返回当前帧累加器中剩余时间占一个固定步的比例（0~1）。用于在两次固定步之间对渲染位置进行线性插值，消除固定步带来的视觉抖动：

```
渲染位置 = Lerp(上一固定步位置, 当前固定步位置, InterpolationAlpha)
```

### 精度

累加器使用 `double` 精度，避免长时间运行（数小时）后 `float` 精度不足导致的时间漂移。

---

## 8. 常见场景

### 场景 A：物理模拟与渲染解耦

```
OnFixedTick:
  → 更新物理状态（位置、速度）
  → 保存 PreviousPosition = CurrentPosition

Event Tick:
  → Alpha = GetInterpolationAlpha()
  → RenderPosition = Lerp(PreviousPosition, CurrentPosition, Alpha)
  → SetActorLocation(RenderPosition)
```

### 场景 B：网络同步

```
OnFixedTick:
  → 执行输入预测
  → 发送状态快照到服务器（使用 GetFixedTickCount() 作为帧序号）
```

### 场景 C：关卡切换后防止 Tick 爆发

```
Event Level Loaded:
  → GetWorldSubsystem(GlobalFixedTimeStepSubsystem)
  → ResetAccumulator()
```

### 场景 D：动态切换 Tick 频率

```
// 进入战斗时提高精度
OnCombatStart:
  → SetFixedTimeStep(1.0 / 120.0)  // 120Hz

// 退出战斗时降低开销
OnCombatEnd:
  → SetFixedTimeStep(1.0 / 60.0)   // 60Hz
```

---

## 9. 注意事项

**不要在 OnFixedTick 中执行渲染操作**  
固定 Tick 用于逻辑/物理更新，渲染更新应在 `Event Tick` 中结合 `GetInterpolationAlpha()` 完成。

**OnFixedTick 每帧可能触发多次**  
当帧率低于固定频率时（如 30fps 运行 60Hz 固定步），每帧会触发 2 次 `OnFixedTick`。逻辑必须能正确处理多次调用。

**OnFixedTickFinished 不保证每帧触发**  
若当帧累加器未达到一个固定步（如帧率远高于固定频率），`OnFixedTickFinished` 不会触发。

**SetFixedTimeStep 会重置累加器**  
运行时修改步长会清零累加器，避免步长变化导致的异常子步爆发。

**编辑器中不运行**  
子系统的 `IsTickableInEditor()` 返回 `false`，固定 Tick 仅在 PIE 和打包游戏中生效。

**多 World 隔离**  
每个 `UWorld`（包括 PIE 多窗口、服务器/客户端）拥有独立的子系统实例和累加器，互不干扰。
