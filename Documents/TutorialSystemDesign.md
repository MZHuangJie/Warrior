# 🎮 Warrior 引导系统设计方案

---

## 📋 文档信息

| 项目 | 内容 |
|------|------|
| **项目名称** | Warrior - 引导系统 (Tutorial System) |
| **文档版本** | v1.0 |
| **创建日期** | 2026-03-13 |
| **实现方式** | C++ 框架 + 蓝图配置 |
| **引导模式** | 线性引导（按固定顺序完成） |

---

## 🎯 一、系统概述

### 1.1 引导类型

本系统支持**两种引导模式**：

| 类型 | 名称 | 描述 | 应用场景 |
|------|------|------|---------|
| 🎮 **Type A** | **游戏世界引导** | 在3D游戏世界中引导玩家操作 | 移动、攻击、技能等 |
| 🖥️ **Type B** | **UI界面引导** | 在UI界面中引导玩家点击控件 | 菜单、商店、背包等 |

```
┌─────────────────────────────────────────────────────────────────────┐
│                      Tutorial System Types                           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   🎮 Type A: 游戏世界引导              🖥️ Type B: UI界面引导          │
│   ┌─────────────────────┐           ┌─────────────────────┐         │
│   │                     │           │  ┌───┐ ┌───┐ ┌───┐  │         │
│   │    🧑 ←──── 移动    │           │  │ A │ │ B │ │ C │  │         │
│   │        ⚔️ 攻击      │           │  └───┘ └─┬─┘ └───┘  │         │
│   │        🛡️ 翻滚      │           │          ↓          │         │
│   │                     │           │     [点击这里]       │         │
│   └─────────────────────┘           └─────────────────────┘         │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### 1.2 目标

为新玩家提供完整的游戏教学体验，引导玩家逐步学习：
- 基础操作（移动、视角、跳跃）
- 战斗系统（轻攻击、重攻击、连击）
- 装备系统（装备/卸下武器）
- 技能系统（翻滚、特殊技能）

### 1.2 表现形式

| 形式 | 描述 |
|------|------|
| 📝 **屏幕提示** | 显示操作说明文字和按键图标 |
| 💡 **高亮显示** | 高亮当前需要按的按键对应的UI |
| ➡️ **箭头指引** | 3D世界中的箭头指向目标 |
| 🎭 **蒙版遮罩** | 暗化非目标区域，聚焦玩家注意力 |
| 🎯 **讽刺攻击对象** | 标记可攻击的训练假人/敌人 |
| 🔊 **音效提示** | 播放引导相关音效 |

### 1.3 核心特性

- ✅ 线性引导流程，确保玩家按顺序学习
- ✅ 支持引导进度持久化存储
- ✅ 蓝图可配置，策划可自由调整
- ✅ 与现有 GAS 系统深度集成
- ✅ 可扩展的步骤系统

---

## 🏗️ 二、系统架构

### 2.1 整体架构图

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Tutorial System                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌──────────────────┐    ┌──────────────────┐    ┌────────────────┐ │
│  │ UTutorialManager │───▶│ UTutorialStep    │───▶│ UTutorialTask  │ │
│  │   (Subsystem)    │    │   (DataAsset)    │    │  (UObject)     │ │
│  └────────┬─────────┘    └──────────────────┘    └────────────────┘ │
│           │                                                          │
│           ▼                                                          │
│  ┌──────────────────┐    ┌──────────────────┐    ┌────────────────┐ │
│  │ UTutorialSave    │    │ ATutorialVolume  │    │WBP_TutorialHUD │ │
│  │  (SaveGame)      │    │    (Actor)       │    │   (Widget)     │ │
│  └──────────────────┘    └──────────────────┘    └────────────────┘ │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### 2.2 核心类设计

#### 2.2.1 UTutorialManagerSubsystem（引导管理器子系统）

**职责**: 引导系统的核心管理器，负责控制引导流程

**类型**: `UGameInstanceSubsystem`

**核心功能**:
| 功能 | 描述 |
|------|------|
| 初始化引导 | 加载引导配置和存档 |
| 开始引导 | 启动引导流程 |
| 推进步骤 | 完成当前步骤，进入下一步 |
| 跳过引导 | 允许玩家跳过引导 |
| 保存进度 | 持久化存储引导进度 |

**关键接口**:
```
- StartTutorial()               // 开始引导
- CompleteCurrentStep()         // 完成当前步骤
- SkipTutorial()               // 跳过引导
- GetCurrentStepIndex()        // 获取当前步骤索引
- IsTutorialCompleted()        // 是否已完成引导
- SaveProgress()               // 保存进度
- LoadProgress()               // 加载进度
```

---

#### 2.2.2 UTutorialStepDataAsset（引导步骤数据资产）

**职责**: 定义单个引导步骤的配置数据

**类型**: `UPrimaryDataAsset`

**数据结构**:
```
FTutorialStepData
├── StepID (FName)                    // 步骤唯一标识
├── StepName (FText)                  // 步骤名称（本地化）
├── Description (FText)               // 步骤描述（本地化）
├── RequiredInputTag (FGameplayTag)   // 需要触发的输入Tag
├── CompletionCondition (枚举)        // 完成条件类型
│   ├── OnInputPressed                // 按下指定按键
│   ├── OnInputReleased              // 释放指定按键
│   ├── OnAbilityActivated           // 激活指定技能
│   ├── OnEnterVolume                // 进入触发区域
│   ├── OnTargetHit                  // 命中目标
│   ├── OnComboCompleted             // 完成连击
│   └── Custom                       // 自定义条件
├── RequiredCount (int32)             // 需要完成的次数
├── TimeLimit (float)                 // 时间限制（0=无限制）
├── UIConfig (FTutorialUIConfig)      // UI配置
│   ├── PromptText (FText)           // 提示文字
│   ├── KeyIcon (UTexture2D*)        // 按键图标
│   ├── bShowArrow (bool)            // 是否显示箭头
│   ├── ArrowTargetTag (FName)       // 箭头指向的Actor标签
│   ├── bUseMask (bool)              // 是否使用蒙版
│   └── HighlightWidgetName (FName)  // 高亮的Widget名称
├── AudioConfig (FTutorialAudioConfig)// 音效配置
│   ├── OnStartSound (USoundBase*)   // 步骤开始音效
│   └── OnCompleteSound (USoundBase*)// 步骤完成音效
└── Tasks (TArray<UTutorialTask*>)    // 子任务列表
```

---

#### 2.2.3 UTutorialTask（引导任务基类）

**职责**: 定义可执行的引导任务

**类型**: `UObject`（蓝图可继承）

**任务类型**:
| 任务类 | 功能 |
|--------|------|
| `UTutorialTask_WaitInput` | 等待玩家输入 |
| `UTutorialTask_WaitAbility` | 等待技能激活 |
| `UTutorialTask_WaitHit` | 等待命中目标 |
| `UTutorialTask_WaitVolume` | 等待进入区域 |
| `UTutorialTask_SpawnActor` | 生成Actor（如训练假人） |
| `UTutorialTask_PlayAnimation` | 播放动画/蒙太奇 |
| `UTutorialTask_Delay` | 延迟等待 |

**关键接口**:
```
- Execute()                    // 执行任务
- OnTaskCompleted (Delegate)   // 任务完成回调
- OnTaskFailed (Delegate)      // 任务失败回调
```

---

#### 2.2.4 ATutorialVolume（引导触发区域）

**职责**: 场景中的触发区域，用于检测玩家位置

**类型**: `AActor`

**组件**:
- `UBoxComponent` - 碰撞检测
- `UArrowComponent` - 可视化方向（仅编辑器）

---

#### 2.2.5 UTutorialSaveGame（引导存档）

**职责**: 持久化存储引导进度

**类型**: `USaveGame`

**存储数据**:
```
FTutorialSaveData
├── bTutorialCompleted (bool)         // 是否完成全部引导
├── CurrentStepIndex (int32)          // 当前步骤索引
├── CompletedStepIDs (TSet<FName>)    // 已完成的步骤ID
└── SaveTime (FDateTime)              // 存档时间
```

---

#### 2.2.6 WBP_TutorialHUD（引导UI控件）

**职责**: 显示引导相关的UI元素

**类型**: `UWarriorWidgetBase`（蓝图）

**UI元素**:
| 元素 | 功能 |
|------|------|
| `TextBlock_Prompt` | 显示提示文字 |
| `Image_KeyIcon` | 显示按键图标 |
| `WBP_TutorialArrow` | 3D箭头指示器 |
| `Border_Mask` | 蒙版遮罩 |
| `ProgressBar` | 引导进度条 |
| `Button_Skip` | 跳过按钮 |

---

## 📝 三、引导步骤设计

### 3.1 引导流程总览

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Tutorial Flow                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  [Phase 1: 基础操作]                                                  │
│  ┌────────┐   ┌────────┐   ┌────────┐                               │
│  │ Step 1 │──▶│ Step 2 │──▶│ Step 3 │                               │
│  │  移动   │   │  视角   │   │  跳跃   │                               │
│  └────────┘   └────────┘   └────────┘                               │
│                                  │                                   │
│                                  ▼                                   │
│  [Phase 2: 装备系统]                                                  │
│  ┌────────┐   ┌────────┐                                            │
│  │ Step 4 │──▶│ Step 5 │                                            │
│  │装备武器 │   │卸下武器 │                                            │
│  └────────┘   └────────┘                                            │
│                    │                                                 │
│                    ▼                                                 │
│  [Phase 3: 战斗系统]                                                  │
│  ┌────────┐   ┌────────┐   ┌────────┐   ┌────────┐                  │
│  │ Step 6 │──▶│ Step 7 │──▶│ Step 8 │──▶│ Step 9 │                  │
│  │ 轻攻击  │   │ 重攻击  │   │  连击   │   │击败敌人 │                  │
│  └────────┘   └────────┘   └────────┘   └────────┘                  │
│                                              │                       │
│                                              ▼                       │
│  [Phase 4: 技能系统]                                                  │
│  ┌────────┐   ┌────────┐                                            │
│  │Step 10 │──▶│Step 11 │──▶  🎉 引导完成！                           │
│  │  翻滚   │   │闪避攻击 │                                            │
│  └────────┘   └────────┘                                            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### 3.2 详细步骤设计

#### Phase 1: 基础操作

| Step | ID | 名称 | 描述 | 完成条件 | 输入Tag |
|------|----|----|------|---------|---------|
| 1 | `Move` | 移动 | 使用 WASD 移动角色 | 移动一定距离 | `InputTag.Move` |
| 2 | `Look` | 视角 | 使用鼠标控制视角 | 旋转视角90度 | `InputTag.Look` |
| 3 | `Jump` | 跳跃 | 按空格键跳跃 | 成功跳跃1次 | `InputTag.Jump` |

#### Phase 2: 装备系统

| Step | ID | 名称 | 描述 | 完成条件 | 输入Tag |
|------|----|----|------|---------|---------|
| 4 | `EquipWeapon` | 装备武器 | 按 1 装备斧头 | 技能激活 | `InputTag.EquipAxe` |
| 5 | `UnequipWeapon` | 卸下武器 | 按 2 卸下斧头 | 技能激活 | `InputTag.UnequipAxe` |

#### Phase 3: 战斗系统

| Step | ID | 名称 | 描述 | 完成条件 | 输入Tag |
|------|----|----|------|---------|---------|
| 6 | `LightAttack` | 轻攻击 | 点击鼠标左键进行轻攻击 | 攻击3次 | `InputTag.LightAttack.Axe` |
| 7 | `HeavyAttack` | 重攻击 | 点击鼠标右键进行重攻击 | 攻击2次 | `InputTag.HeavyAttack.Axe` |
| 8 | `Combo` | 连击 | 连续攻击形成连击 | 完成4连击 | `Player.Ability.Attack.*` |
| 9 | `DefeatEnemy` | 击败敌人 | 击败训练假人 | 目标死亡 | `Shared.Event.MeleeHit` |

#### Phase 4: 技能系统

| Step | ID | 名称 | 描述 | 完成条件 | 输入Tag |
|------|----|----|------|---------|---------|
| 10 | `Roll` | 翻滚 | 按 Shift 翻滚闪避 | 翻滚2次 | `InputTag.Roll` |
| 11 | `DodgeAttack` | 闪避攻击 | 闪避敌人攻击并反击 | 成功闪避1次 | 自定义 |

---

## 🎨 四、UI/UX 设计

### 4.1 引导HUD布局

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                              [跳过] │
│                                                                      │
│                                                                      │
│                                                                      │
│                          ┌──────────────┐                           │
│                          │   3D 箭头    │                           │
│                          │   指向目标   │                           │
│                          └──────────────┘                           │
│                                                                      │
│                                                                      │
│                                                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  ╔═══╗                                                         │  │
│  │  ║ W ║  使用 WASD 移动角色                                      │  │
│  │  ╚═══╝                                                         │  │
│  │  ────────────────────────────────────────────────────────────  │  │
│  │  [■■■■■□□□□□] 引导进度 (5/11)                                  │  │
│  └───────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
```

### 4.2 视觉效果

| 效果 | 实现方式 |
|------|---------|
| **蒙版遮罩** | 使用 `UMG` 的 `RetainerBox` + Material 实现半透明遮罩 |
| **高亮效果** | Widget 动画 + 发光材质 |
| **3D 箭头** | `UWidgetComponent` 附加到场景Actor |
| **按键图标** | 使用 `CommonUI` 的 `InputActionWidget` 自动适配平台 |

### 4.3 动画效果

| 动画 | 描述 |
|------|------|
| `Anim_FadeIn` | 引导UI淡入 |
| `Anim_FadeOut` | 引导UI淡出 |
| `Anim_Pulse` | 按键图标脉冲闪烁 |
| `Anim_ArrowBounce` | 箭头上下弹跳 |
| `Anim_StepComplete` | 步骤完成庆祝动画 |

---

## 💾 五、数据存储设计

### 5.1 存档插槽

```
SaveSlot: "TutorialProgress"
```

### 5.2 存档结构

```cpp
USTRUCT(BlueprintType)
struct FTutorialSaveData
{
    GENERATED_BODY()
    
    // 是否完成全部引导
    UPROPERTY(BlueprintReadWrite)
    bool bTutorialCompleted = false;
    
    // 当前步骤索引
    UPROPERTY(BlueprintReadWrite)
    int32 CurrentStepIndex = 0;
    
    // 已完成的步骤ID集合
    UPROPERTY(BlueprintReadWrite)
    TSet<FName> CompletedStepIDs;
    
    // 存档时间
    UPROPERTY(BlueprintReadWrite)
    FDateTime SaveTime;
};
```

### 5.3 存档时机

| 时机 | 描述 |
|------|------|
| 完成步骤时 | 每完成一个步骤自动保存 |
| 退出游戏时 | 游戏退出前保存当前进度 |
| 完成引导时 | 标记引导已完成 |

---

## 🔗 六、与现有系统集成

### 6.1 与 GAS 系统集成

```
┌──────────────────────────────────────────────────────────────────┐
│                     GAS Integration                               │
├──────────────────────────────────────────────────────────────────┤
│                                                                   │
│  [Input System]                                                   │
│  UWarriorInputComponent                                          │
│         │                                                         │
│         ▼ OnAbilityInputPressed                                  │
│  ┌──────────────────┐      ┌─────────────────────────────────┐  │
│  │ Tutorial Manager │◀─────│ UTutorialTask_WaitInput         │  │
│  └──────────────────┘      └─────────────────────────────────┘  │
│         │                                                         │
│         ▼ OnAbilityActivated                                     │
│  ┌──────────────────┐      ┌─────────────────────────────────┐  │
│  │ ASC Delegate     │◀─────│ UTutorialTask_WaitAbility       │  │
│  └──────────────────┘      └─────────────────────────────────┘  │
│         │                                                         │
│         ▼ OnGameplayEvent                                        │
│  ┌──────────────────┐      ┌─────────────────────────────────┐  │
│  │ Combat Component │◀─────│ UTutorialTask_WaitHit           │  │
│  └──────────────────┘      └─────────────────────────────────┘  │
│                                                                   │
└──────────────────────────────────────────────────────────────────┘
```

### 6.2 监听点

| 监听点 | 来源类 | 信号 |
|--------|--------|------|
| 输入按下 | `UWarriorAbilitySystemComponent` | `OnAbilityInputPressed` |
| 技能激活 | `UAbilitySystemComponent` | `OnAbilityEnded` |
| 命中事件 | `UHeroCombatComponent` | `OnHitTargetActor` |
| 连击完成 | `GA_Hero_LightAttackMaster` | Custom Event |
| 翻滚状态 | `UWarriorAttributeSet` | Tag Change |

### 6.3 Gameplay Tags 扩展

需要添加的新 Tags：
```cpp
// 引导系统 Tags
Tutorial.State.InProgress       // 引导进行中
Tutorial.State.Completed        // 引导已完成
Tutorial.State.Skipped          // 引导被跳过
Tutorial.Step.Current           // 当前步骤标记
```

---

## ?️ 六B、UI界面引导系统设计

### 6B.1 UI引导概述

UI界面引导用于在特定UI界面中，逐步引导玩家点击指定的控件，完成特定操作流程。

**典型应用场景：**
- 首次打开背包界面，引导装备物品
- 首次进入商店，引导购买流程
- 技能树界面，引导升级技能
- 设置界面，引导调整画质

### 6B.2 UI引导架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                      UI Guide System                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌──────────────────┐    ┌──────────────────┐    ┌────────────────┐ │
│  │ UUIGuideManager  │───▶│ FUIGuideStep     │───▶│ UUIGuideMask   │ │
│  │   (Subsystem)    │    │   (Struct)       │    │   (Widget)     │ │
│  └────────┬─────────┘    └──────────────────┘    └────────────────┘ │
│           │                                                          │
│           ▼                                                          │
│  ┌──────────────────┐    ┌──────────────────┐    ┌────────────────┐ │
│  │UUIGuideDataAsset │    │ UIGuideHighlight │    │ UUIGuideFinger │ │
│  │   (DataAsset)    │    │   (Component)    │    │   (Widget)     │ │
│  └──────────────────┘    └──────────────────┘    └────────────────┘ │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### 6B.3 核心类设计

#### 6B.3.1 UUIGuideManagerSubsystem（UI引导管理器）

**职责**: 管理UI界面内的引导流程

**类型**: `UGameInstanceSubsystem`

**关键接口**:
```cpp
// 开始指定界面的UI引导
UFUNCTION(BlueprintCallable, Category = "Warrior|UIGuide")
void StartUIGuide(FName GuideID);

// 推进到下一步
UFUNCTION(BlueprintCallable, Category = "Warrior|UIGuide")
void NextStep();

// 当前步骤完成（由目标控件触发）
UFUNCTION(BlueprintCallable, Category = "Warrior|UIGuide")
void OnTargetWidgetClicked(UWidget* ClickedWidget);

// 中止UI引导
UFUNCTION(BlueprintCallable, Category = "Warrior|UIGuide")
void AbortUIGuide();

// 检查是否在引导中
UFUNCTION(BlueprintPure, Category = "Warrior|UIGuide")
bool IsUIGuideActive() const;
```

---

#### 6B.3.2 FUIGuideStep（UI引导步骤结构体）

```cpp
USTRUCT(BlueprintType)
struct FUIGuideStep
{
    GENERATED_BODY()
    
    // 步骤ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StepID;
    
    // 目标Widget路径（支持嵌套路径）
    // 格式: "ParentWidget.ChildWidget.TargetButton"
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetWidgetPath;
    
    // 目标Widget名称（简单模式）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetWidgetName;
    
    // 提示文字
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText PromptText;
    
    // 手指/箭头位置（相对于目标Widget）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EUIGuideFingerPosition FingerPosition;
    
    // 高亮形状
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EUIGuideHighlightShape HighlightShape;
    
    // 高亮边距
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMargin HighlightPadding;
    
    // 是否阻挡其他区域点击
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBlockOtherInput = true;
    
    // 完成条件
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EUIGuideCompleteCondition CompleteCondition;
    
    // 延迟自动完成时间（仅当CompleteCondition为Delay时有效）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoCompleteDelay = 0.0f;
    
    // 完成后的回调事件名
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName OnCompleteEventName;
};
```

---

#### 6B.3.3 相关枚举定义

```cpp
// 手指位置
UENUM(BlueprintType)
enum class EUIGuideFingerPosition : uint8
{
    Top,           // 上方
    Bottom,        // 下方
    Left,          // 左侧
    Right,         // 右侧
    TopLeft,       // 左上
    TopRight,      // 右上
    BottomLeft,    // 左下
    BottomRight,   // 右下
    Center         // 中心（无手指，仅高亮）
};

// 高亮形状
UENUM(BlueprintType)
enum class EUIGuideHighlightShape : uint8
{
    Rectangle,     // 矩形
    RoundedRect,   // 圆角矩形
    Circle,        // 圆形
    Ellipse,       // 椭圆
    Custom         // 自定义（使用Material）
};

// 完成条件
UENUM(BlueprintType)
enum class EUIGuideCompleteCondition : uint8
{
    OnClick,       // 点击目标Widget
    OnHover,       // 悬停目标Widget
    OnAnyClick,    // 点击任意位置
    Delay,         // 延迟自动完成
    Manual         // 手动调用完成
};
```

---

#### 6B.3.4 WBP_UIGuideMask（UI引导蒙版控件）

**职责**: 显示遮罩和高亮效果

**实现方式**: 使用 **Material** 实现挖孔效果

```
┌─────────────────────────────────────────────────────────────────────┐
│  ████████████████████████████████████████████████████████████████  │
│  ████████████████████████████████████████████████████████████████  │
│  ████████████████████████████████████████████████████████████████  │
│  ████████████████┌────────────────┐██████████████████████████████  │
│  ████████████████│                │██████████████████████████████  │
│  ████████████████│  透明高亮区域   │██████████████████████████████  │
│  ████████████████│   (挖孔效果)    │██████████████████████████████  │
│  ████████████████│                │██████████████████████████████  │
│  ████████████████└────────────────┘██████████████████████████████  │
│  ████████████████████████████████████████████████████████████████  │
│  ████████████████████████████████████████████████████████████████  │
└─────────────────────────────────────────────────────────────────────┘
         ↑ 半透明遮罩层（阻挡点击）      ↑ 高亮区域（允许点击穿透）
```

**关键属性**:
```cpp
// 遮罩颜色和透明度
UPROPERTY(EditAnywhere, BlueprintReadWrite)
FLinearColor MaskColor = FLinearColor(0, 0, 0, 0.7f);

// 高亮位置和大小
UPROPERTY(BlueprintReadWrite)
FVector2D HighlightPosition;

UPROPERTY(BlueprintReadWrite)
FVector2D HighlightSize;

// 高亮形状
UPROPERTY(BlueprintReadWrite)
EUIGuideHighlightShape HighlightShape;

// 高亮边框颜色
UPROPERTY(EditAnywhere, BlueprintReadWrite)
FLinearColor HighlightBorderColor = FLinearColor(1, 1, 0, 1);

// 高亮边框宽度
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float HighlightBorderWidth = 3.0f;
```

---

#### 6B.3.5 WBP_UIGuideFinger（手指指示器控件）

**职责**: 显示点击提示的手指/箭头动画

**动画效果**:
- 上下/左右轻微晃动
- 点击时缩放反馈
- 出现/消失淡入淡出

```
        ┌─────┐
        │  👆 │  ← 手指图标（带动画）
        └──┬──┘
           │
           ▼
    ┌──────────────┐
    │   目标按钮   │
    └──────────────┘
```

---

### 6B.4 UI引导流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                      UI Guide Flow                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  1️⃣ 触发UI引导                                                       │
│     ┌──────────────────┐                                            │
│     │ StartUIGuide()   │                                            │
│     └────────┬─────────┘                                            │
│              ▼                                                       │
│  2️⃣ 查找目标Widget                                                   │
│     ┌──────────────────┐                                            │
│     │ FindWidgetByPath │                                            │
│     └────────┬─────────┘                                            │
│              ▼                                                       │
│  3️⃣ 计算高亮区域                                                      │
│     ┌──────────────────┐                                            │
│     │ GetWidgetGeometry│                                            │
│     └────────┬─────────┘                                            │
│              ▼                                                       │
│  4️⃣ 显示蒙版和高亮                                                    │
│     ┌──────────────────┐                                            │
│     │ ShowMaskWidget   │                                            │
│     │ ShowFingerWidget │                                            │
│     │ ShowPromptText   │                                            │
│     └────────┬─────────┘                                            │
│              ▼                                                       │
│  5️⃣ 等待用户操作                                                      │
│     ┌──────────────────┐                                            │
│     │ 监听目标Widget   │                                             │
│     │ 点击/悬停事件    │                                             │
│     └────────┬─────────┘                                            │
│              ▼                                                       │
│  6️⃣ 完成当前步骤                                                      │
│     ┌──────────────────┐                                            │
│     │ OnStepCompleted  │───▶ 有下一步? ──Yes──▶ 回到步骤2️⃣           │
│     └────────┬─────────┘         │                                  │
│              │                   No                                  │
│              ▼                   ▼                                   │
│  7️⃣ 结束UI引导                                                       │
│     ┌──────────────────┐                                            │
│     │ EndUIGuide()     │                                            │
│     │ HideMaskWidget   │                                            │
│     └──────────────────┘                                            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

### 6B.5 Widget查找机制

**支持的查找方式：**

| 方式 | 格式 | 示例 |
|------|------|------|
| **名称查找** | `WidgetName` | `Button_Confirm` |
| **路径查找** | `Parent.Child.Target` | `Panel_Main.ScrollBox.Button_Item_0` |
| **标签查找** | `#TagName` | `#TutorialTarget` |
| **索引查找** | `Parent[Index]` | `ItemList[0]` |

**查找实现：**
```cpp
UWidget* UUIGuideManagerSubsystem::FindWidgetByPath(UUserWidget* RootWidget, const FString& Path)
{
    TArray<FString> PathSegments;
    Path.ParseIntoArray(PathSegments, TEXT("."));
    
    UWidget* CurrentWidget = RootWidget;
    
    for (const FString& Segment : PathSegments)
    {
        if (Segment.StartsWith(TEXT("#")))
        {
            // 标签查找
            FName Tag = FName(*Segment.RightChop(1));
            CurrentWidget = FindWidgetByTag(CurrentWidget, Tag);
        }
        else if (Segment.Contains(TEXT("[")))
        {
            // 索引查找
            FString Name, IndexStr;
            Segment.Split(TEXT("["), &Name, &IndexStr);
            int32 Index = FCString::Atoi(*IndexStr.LeftChop(1));
            CurrentWidget = FindWidgetByNameAndIndex(CurrentWidget, FName(*Name), Index);
        }
        else
        {
            // 名称查找
            CurrentWidget = FindWidgetByName(CurrentWidget, FName(*Segment));
        }
        
        if (!CurrentWidget) break;
    }
    
    return CurrentWidget;
}
```

---

### 6B.6 点击穿透机制

**关键问题**: 蒙版覆盖在目标Widget上方，如何让目标Widget仍然可以接收点击？

**解决方案**: **使用 Hit Test 过滤**

```cpp
// 在 WBP_UIGuideMask 中重写 OnMouseButtonDown
FReply UUIGuideMask::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FVector2D LocalPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
    
    // 检查是否点击在高亮区域内
    if (IsPointInHighlightArea(LocalPosition))
    {
        // 穿透点击，让底层Widget处理
        return FReply::Unhandled();
    }
    else
    {
        // 阻挡点击，播放提示音效
        PlayBlockedClickSound();
        return FReply::Handled();
    }
}
```

**或者使用 Visibility 控制：**
```cpp
// 设置高亮区域为 HitTestInvisible，允许点击穿透
HighlightArea->SetVisibility(ESlateVisibility::HitTestInvisible);

// 设置遮罩区域为 Visible，阻挡点击
MaskArea->SetVisibility(ESlateVisibility::Visible);
```

---

### 6B.7 示例UI引导配置

**背包界面引导配置：**

```cpp
// DA_UIGuide_Inventory.uasset
TArray<FUIGuideStep> Steps = {
    // Step 1: 点击物品
    {
        StepID = "ClickItem",
        TargetWidgetPath = "ScrollBox_Items.WBP_ItemSlot[0]",
        PromptText = NSLOCTEXT("UIGuide", "ClickItem", "点击选择一件装备"),
        FingerPosition = EUIGuideFingerPosition::Right,
        HighlightShape = EUIGuideHighlightShape::RoundedRect,
        CompleteCondition = EUIGuideCompleteCondition::OnClick
    },
    // Step 2: 点击装备按钮
    {
        StepID = "ClickEquip",
        TargetWidgetPath = "Panel_ItemDetail.Button_Equip",
        PromptText = NSLOCTEXT("UIGuide", "ClickEquip", "点击装备按钮"),
        FingerPosition = EUIGuideFingerPosition::Top,
        HighlightShape = EUIGuideHighlightShape::Rectangle,
        CompleteCondition = EUIGuideCompleteCondition::OnClick
    },
    // Step 3: 关闭界面
    {
        StepID = "ClosePanel",
        TargetWidgetPath = "Button_Close",
        PromptText = NSLOCTEXT("UIGuide", "ClosePanel", "点击关闭背包"),
        FingerPosition = EUIGuideFingerPosition::Bottom,
        HighlightShape = EUIGuideHighlightShape::Circle,
        CompleteCondition = EUIGuideCompleteCondition::OnClick
    }
};
```

---

### 6B.8 与游戏世界引导的整合

两种引导可以**无缝切换**：

```
┌─────────────────────────────────────────────────────────────────────┐
│                   Combined Tutorial Flow                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  🎮 游戏世界引导                                                      │
│  ┌────────┐   ┌────────┐   ┌────────┐                               │
│  │ 移动   │──▶│ 攻击   │──▶│打开背包 │                               │
│  └────────┘   └────────┘   └───┬────┘                               │
│                                │                                     │
│                                ▼ 触发UI引导                          │
│  🖥️ UI界面引导                                                       │
│  ┌────────────┐   ┌────────────┐   ┌────────────┐                   │
│  │ 选择物品   │──▶│ 点击装备   │──▶│ 关闭背包   │                    │
│  └────────────┘   └────────────┘   └─────┬──────┘                   │
│                                          │                           │
│                                          ▼ 返回游戏世界引导          │
│  🎮 游戏世界引导                                                      │
│  ┌────────────┐   ┌────────────┐                                    │
│  │ 使用新装备 │──▶│ 引导完成   │                                    │
│  └────────────┘   └────────────┘                                    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**整合代码：**
```cpp
// 在 TutorialManagerSubsystem 中
void UTutorialManagerSubsystem::OnWorldStepCompleted(FName StepID)
{
    if (StepID == "OpenInventory")
    {
        // 切换到UI引导
        UUIGuideManagerSubsystem* UIGuide = GetGameInstance()->GetSubsystem<UUIGuideManagerSubsystem>();
        UIGuide->OnUIGuideCompleted.AddDynamic(this, &ThisClass::OnInventoryGuideCompleted);
        UIGuide->StartUIGuide("Inventory_FirstTime");
    }
}

void UTutorialManagerSubsystem::OnInventoryGuideCompleted()
{
    // UI引导完成，继续游戏世界引导
    NextStep();
}
```

---

## ?📁 七、文件结构

### 7.1 C++ 文件

```
Source/Warrior/
├── Public/
│   └── Tutorial/
│       ├── TutorialManagerSubsystem.h
│       ├── TutorialStepDataAsset.h
│       ├── TutorialTask.h
│       ├── Tasks/
│       │   ├── TutorialTask_WaitInput.h
│       │   ├── TutorialTask_WaitAbility.h
│       │   ├── TutorialTask_WaitHit.h
│       │   ├── TutorialTask_WaitVolume.h
│       │   └── TutorialTask_SpawnActor.h
│       ├── TutorialVolume.h
│       ├── TutorialSaveGame.h
│       └── TutorialTypes.h
└── Private/
    └── Tutorial/
        ├── TutorialManagerSubsystem.cpp
        ├── TutorialStepDataAsset.cpp
        ├── TutorialTask.cpp
        ├── Tasks/
        │   ├── TutorialTask_WaitInput.cpp
        │   ├── TutorialTask_WaitAbility.cpp
        │   ├── TutorialTask_WaitHit.cpp
        │   ├── TutorialTask_WaitVolume.cpp
        │   └── TutorialTask_SpawnActor.cpp
        ├── TutorialVolume.cpp
        └── TutorialSaveGame.cpp
```

### 7.2 蓝图/资产文件

```
Content/
└── Tutorial/
    ├── Data/
    │   ├── DA_TutorialConfig.uasset           // 总配置
    │   └── Steps/
    │       ├── DA_Step_Move.uasset
    │       ├── DA_Step_Look.uasset
    │       ├── DA_Step_Jump.uasset
    │       ├── DA_Step_EquipWeapon.uasset
    │       ├── DA_Step_UnequipWeapon.uasset
    │       ├── DA_Step_LightAttack.uasset
    │       ├── DA_Step_HeavyAttack.uasset
    │       ├── DA_Step_Combo.uasset
    │       ├── DA_Step_DefeatEnemy.uasset
    │       ├── DA_Step_Roll.uasset
    │       └── DA_Step_DodgeAttack.uasset
    ├── Widgets/
    │   ├── WBP_TutorialHUD.uasset
    │   ├── WBP_TutorialPrompt.uasset
    │   ├── WBP_TutorialArrow.uasset
    │   └── WBP_TutorialMask.uasset
    ├── Actors/
    │   ├── BP_TutorialVolume.uasset
    │   └── BP_TrainingDummy.uasset            // 训练假人
    ├── Audio/
    │   ├── SFX_Tutorial_Start.uasset
    │   ├── SFX_Tutorial_StepComplete.uasset
    │   └── SFX_Tutorial_Complete.uasset
    └── Textures/
        └── Icons/
            ├── T_Key_WASD.uasset
            ├── T_Key_Mouse.uasset
            ├── T_Key_Space.uasset
            └── T_Key_Shift.uasset
```

---

## 🔄 八、实现步骤

### Phase 1: 基础框架（预计2天）

- [ ] 1.1 创建 `TutorialTypes.h` 定义所有结构体和枚举
- [ ] 1.2 实现 `UTutorialManagerSubsystem` 基础功能
- [ ] 1.3 实现 `UTutorialStepDataAsset` 数据资产
- [ ] 1.4 实现 `UTutorialSaveGame` 存档系统
- [ ] 1.5 添加 Gameplay Tags

### Phase 2: 任务系统（预计2天）

- [ ] 2.1 实现 `UTutorialTask` 基类
- [ ] 2.2 实现 `UTutorialTask_WaitInput`
- [ ] 2.3 实现 `UTutorialTask_WaitAbility`
- [ ] 2.4 实现 `UTutorialTask_WaitHit`
- [ ] 2.5 实现 `UTutorialTask_WaitVolume`
- [ ] 2.6 实现 `UTutorialTask_SpawnActor`

### Phase 3: UI系统（预计2天）

- [ ] 3.1 创建 `WBP_TutorialHUD` 主控件
- [ ] 3.2 创建 `WBP_TutorialPrompt` 提示控件
- [ ] 3.3 创建 `WBP_TutorialArrow` 箭头控件
- [ ] 3.4 创建 `WBP_TutorialMask` 蒙版控件
- [ ] 3.5 实现UI动画

### Phase 4: 场景集成（预计1天）

- [ ] 4.1 创建 `BP_TutorialVolume` 蓝图
- [ ] 4.2 创建 `BP_TrainingDummy` 训练假人
- [ ] 4.3 布置引导场景

### Phase 5: 配置与测试（预计2天）

- [ ] 5.1 创建所有步骤的 DataAsset
- [ ] 5.2 配置所有步骤参数
- [ ] 5.3 全流程测试
- [ ] 5.4 边界情况测试
- [ ] 5.5 存档功能测试

---

## ⚠️ 九、注意事项

### 9.1 性能考虑

| 问题 | 解决方案 |
|------|---------|
| UI频繁更新 | 使用属性绑定而非Tick |
| 大量事件监听 | 按需注册/注销监听器 |
| 3D箭头渲染 | 使用Widget Component的LOD |

### 9.2 边界情况

| 场景 | 处理方式 |
|------|---------|
| 玩家死亡 | 暂停引导，复活后继续 |
| 切换关卡 | 保存进度，新关卡恢复 |
| 网络断开 | 本地继续，重连后同步 |
| 存档损坏 | 提供重置选项 |

### 9.3 可扩展性

- 预留自定义任务接口，便于添加新任务类型
- 步骤配置使用 DataAsset，无需修改代码即可添加新步骤
- UI使用蓝图实现，策划可自由调整样式

---

## 📊 十、进度追踪

| 阶段 | 状态 | 开始日期 | 完成日期 |
|------|------|---------|---------|
| Phase 1: 基础框架 | ⏳ 待开始 | - | - |
| Phase 2: 任务系统 | ⏳ 待开始 | - | - |
| Phase 3: UI系统 | ⏳ 待开始 | - | - |
| Phase 4: 场景集成 | ⏳ 待开始 | - | - |
| Phase 5: 配置测试 | ⏳ 待开始 | - | - |

---

## 📎 附录

### A. 参考资料

- [UE5 Game Instance Subsystem](https://docs.unrealengine.com/5.0/en-US/API/Runtime/Engine/Subsystems/UGameInstanceSubsystem/)
- [UE5 Primary Data Asset](https://docs.unrealengine.com/5.0/en-US/API/Runtime/Engine/Engine/UPrimaryDataAsset/)
- [UE5 Save Game](https://docs.unrealengine.com/5.0/en-US/saving-and-loading-your-game-in-unreal-engine/)

### B. 相关文档

- [ProjectDocumentation.md](./ProjectDocumentation.md) - 项目总体文档
- [.codemaker/rules/rules.mdc](../.codemaker/rules/rules.mdc) - 编码规范

---

*文档结束*
