# Warrior 项目技术文档

## 一、项目概述

**Warrior** 是一个基于 Unreal Engine 5 (UE5) 开发的 C++ RPG 游戏项目。该项目使用虚幻引擎的 **Gameplay Ability System (GAS)** 作为核心框架，实现了一个包含玩家角色战斗、敌人AI行为、伤害计算等完整功能的动作角色扮演游戏原型。

### 核心技术栈

- **引擎版本**: Unreal Engine 5
- **编程语言**: C++
- **核心系统**: 
  - Gameplay Ability System (GAS)
  - Enhanced Input System
  - Motion Warping
  - AI Behavior Tree
  - AI Perception System

### 项目依赖模块

```cpp
- Core / CoreUObject / Engine
- InputCore / EnhancedInput
- GameplayTags / GameplayTasks / GameplayAbilities
- AnimGraphRuntime
- MotionWarping
```

---

## 二、项目架构

### 目录结构

```
Source/Warrior/
├── Private/                    # 私有实现文件
│   ├── AbilitySystem/         # 技能系统实现
│   │   ├── Abilities/         # 具体技能类
│   │   └── GEExecCalc/        # GameplayEffect 计算器
│   ├── AI/                    # AI 行为树任务和服务
│   ├── AnimInstance/          # 动画实例
│   │   └── Hero/              # 英雄角色动画
│   ├── Characters/            # 角色类
│   ├── Components/            # 组件
│   │   ├── Combat/            # 战斗组件
│   │   ├── Input/             # 输入组件
│   │   └── UI/                # UI组件
│   ├── Controller/            # 控制器
│   ├── DataAssets/            # 数据资产
│   │   ├── Input/             # 输入配置
│   │   └── StarUpData/        # 启动数据
│   ├── GameMode/              # 游戏模式
│   ├── Interfaces/            # 接口
│   ├── Items/                 # 物品
│   │   └── Weapons/           # 武器
│   ├── WarriorTypes/          # 类型定义
│   └── Widgets/               # UI控件
└── Public/                    # 公开头文件（与Private对应）
```

---

## 三、核心模块详解

### 3.1 角色系统 (Characters)

#### 3.1.1 AWarriorBaseCharacter（基础角色类）

**文件**: `Characters/WarriorBaseCharacter.h/.cpp`

**功能**: 所有游戏角色的基类，提供共享的基础功能。

**核心组件**:
- `UWarriorAbilitySystemComponent` - 技能系统组件
- `UWarriorAttributeSet` - 属性集
- `UMotionWarpingComponent` - 运动扭曲组件（用于攻击锁定等）

**关键代码实现**:
```cpp
AWarriorBaseCharacter::AWarriorBaseCharacter()
{
    // 禁用Tick以优化性能
    PrimaryActorTick.bCanEverTick = false;
    
    // 创建GAS组件
    WarriorAbilitySystemComponent = CreateDefaultSubobject<UWarriorAbilitySystemComponent>("WarriorAbilitySystemComponent");
    WarriorAttributeSet = CreateDefaultSubobject<UWarriorAttributeSet>("WarriorAttributeSet");
    
    // 创建Motion Warping组件
    MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>("MotionWarpingComponent");
}
```

**接口实现**:
- `IAbilitySystemInterface` - 提供 `GetAbilitySystemComponent()`
- `IPawnCombatInterface` - 提供 `GetPawnCombatComponent()`
- `IPawnUIInterface` - 提供 `GetPawnUIComponent()`

---

#### 3.1.2 AWarriorHeroCharacter（玩家角色类）

**文件**: `Characters/WarriorHeroCharacter.h/.cpp`

**功能**: 玩家控制的英雄角色，处理输入、相机、战斗等功能。

**核心组件**:
- `USpringArmComponent` - 相机摇臂
- `UCameraComponent` - 跟随相机
- `UHeroCombatComponent` - 英雄战斗组件
- `UHeroUIComponent` - 英雄UI组件

**输入系统实现**:
```cpp
void AWarriorHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // 获取增强输入子系统
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    
    // 添加输入映射上下文
    Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);
    
    // 绑定原生输入动作
    WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset, WarriorGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
    WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset, WarriorGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
    
    // 绑定技能输入动作
    WarriorInputComponent->BindAbilityInputAction(InputConfigDataAsset, this, &ThisClass::Input_AbilityInputPressed, &ThisClass::Input_AbilityInputRealesed);
}
```

**移动系统**:
```cpp
void AWarriorHeroCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
    const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
    const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
    
    // 前后移动
    if (MovementVector.Y != 0.f)
    {
        const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);
        AddMovementInput(ForwardDirection, MovementVector.Y);
    }
    
    // 左右移动
    if (MovementVector.X != 0.f)
    {
        const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}
```

---

#### 3.1.3 AWarriorEnemyCharacter（敌人角色类）

**文件**: `Characters/WarriorEnemyCharacter.h/.cpp`

**功能**: AI控制的敌人角色，支持异步加载启动数据。

**核心组件**:
- `UEnemyCombatComponent` - 敌人战斗组件
- `UEnemyUIComponent` - 敌人UI组件
- `UWidgetComponent` - 血条Widget组件

**异步数据加载**:
```cpp
void AWarriorEnemyCharacter::InitEnemyStartUpData()
{
    if (CharacterStartUpData.IsNull()) return;

    // 使用Asset Manager进行异步加载
    UAssetManager::GetStreamableManager().RequestAsyncLoad(
        CharacterStartUpData.ToSoftObjectPath(),
        FStreamableDelegate::CreateLambda([this]()
        {
            if (UDataAsset_StarUpDataBase* LoadedData = CharacterStartUpData.Get())
            {
                LoadedData->GiveToAbilitySystemComponent(WarriorAbilitySystemComponent);
            }
        })
    );
}
```

---

### 3.2 技能系统 (Ability System)

#### 3.2.1 UWarriorAbilitySystemComponent（技能系统组件）

**文件**: `AbilitySystem/WarriorAbilitySystemComponent.h/.cpp`

**功能**: 自定义的ASC，处理技能输入、授予和移除。

**核心方法**:

| 方法 | 功能 |
|------|------|
| `OnAbilityInputPressed` | 根据输入Tag激活对应技能 |
| `OnAbilityInputReleased` | 处理技能输入释放 |
| `GrantHeroWeaponAbilities` | 授予武器相关技能 |
| `RemoveGrantedHeroWeaponAbilities` | 移除武器技能 |
| `TryActivaAbilityByTag` | 通过Tag尝试激活技能 |

**技能激活实现**:
```cpp
void UWarriorAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InInputTag)
{
    if (!InInputTag.IsValid()) return;

    for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        // 检查技能的动态Tag是否包含输入Tag
        if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InInputTag)) continue;
        TryActivateAbility(AbilitySpec.Handle);
    }
}
```

**武器技能授予**:
```cpp
void UWarriorAbilitySystemComponent::GrantHeroWeaponAbilities(
    const TArray<FWarriorHeroAbilitySet>& InDefaultWeaponAbilities, 
    int32 ApplyLevel, 
    TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandle)
{
    for (const FWarriorHeroAbilitySet& AbilitySet : InDefaultWeaponAbilities)
    {
        if(!AbilitySet.IsVaild()) continue;
        
        FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
        AbilitySpec.SourceObject = GetAvatarActor();
        AbilitySpec.Level = ApplyLevel;
        AbilitySpec.DynamicAbilityTags.AddTag(AbilitySet.InputTag);  // 添加输入Tag
        OutGrantedAbilitySpecHandle.AddUnique(GiveAbility(AbilitySpec));
    }
}
```

---

#### 3.2.2 UWarriorAttributeSet（属性集）

**文件**: `AbilitySystem/WarriorAttributeSet.h/.cpp`

**功能**: 定义角色的所有数值属性。

**属性列表**:

| 属性 | 类型 | 说明 |
|------|------|------|
| `CurrentHealth` | FGameplayAttributeData | 当前生命值 |
| `MaxHealth` | FGameplayAttributeData | 最大生命值 |
| `CurrentRage` | FGameplayAttributeData | 当前怒气值 |
| `MaxRage` | FGameplayAttributeData | 最大怒气值 |
| `AttackPower` | FGameplayAttributeData | 攻击力 |
| `DefensePower` | FGameplayAttributeData | 防御力 |
| `DamageTaken` | FGameplayAttributeData | 受到的伤害（Meta属性） |

**属性变更处理**:
```cpp
void UWarriorAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    // 获取UI组件接口
    UPawnUIComponent* PawnUIComponent = CachedPawnUIInterface->GetPawnUIComponent();
    
    // 生命值变更
    if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
    {
        const float NewCurrentHealth = FMath::Clamp(GetCurrentHealth(), 0.f, GetMaxHealth());
        SetCurrentHealth(NewCurrentHealth);
        PawnUIComponent->OnCurrentHealthChanged.Broadcast(GetCurrentHealth()/GetMaxHealth());
    }
    
    // 怒气值变更
    if (Data.EvaluatedData.Attribute == GetCurrentRageAttribute())
    {
        const float NewCurrentRage = FMath::Clamp(GetCurrentRage(), 0.f, GetMaxRage());
        SetCurrentRage(NewCurrentRage);
        if (UHeroUIComponent* HeroUIComponent = CachedPawnUIInterface->GetHeroUIComponent())
        {
            HeroUIComponent->OnCurrentRageChanged.Broadcast(GetCurrentRage()/GetMaxRage());
        }
    }
    
    // 伤害处理
    if (Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
    {
        const float NewCurrentHealth = FMath::Clamp(OldHealth - DamageDone, 0.f, GetMaxHealth());
        SetCurrentHealth(NewCurrentHealth);
        
        // 死亡判定
        if (GetCurrentHealth() == 0.f)
        {
            UWarriorFunctionLibrary::AddGameplayTagToActorIfNone(
                Data.Target.GetAvatarActor(), 
                WarriorGameplayTags::Shared_Status_Dead
            );
        }
    }
}
```

---

#### 3.2.3 UWarriorGameplayAbility（技能基类）

**文件**: `AbilitySystem/Abilities/WarriorGameplayAbility.h/.cpp`

**功能**: 所有游戏技能的基类。

**激活策略枚举**:
```cpp
enum class EWarriorAbilityActivationPolicy : uint8
{
    OnTriggered,  // 手动触发
    OnGiven       // 授予时自动激活
};
```

**关键方法**:
```cpp
// 技能授予时的回调
void UWarriorGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnGiveAbility(ActorInfo, Spec);
    
    // 如果策略是OnGiven，则自动激活
    if (AbilityActivationPolicy == EWarriorAbilityActivationPolicy::OnGiven)
    {
        if (ActorInfo && !Spec.IsActive())
        {
            ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
        }
    }
}

// 应用GameplayEffect到目标
FActiveGameplayEffectHandle UWarriorGameplayAbility::NativeApplyEffectSpecHandleToTarget(
    AActor* TargetActor, 
    const FGameplayEffectSpecHandle& InSpecHandle)
{
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    return GetWarriorAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
        *InSpecHandle.Data,
        TargetASC
    );
}
```

---

#### 3.2.4 UWarriorHeroGameplayAbility（英雄技能类）

**文件**: `AbilitySystem/Abilities/WarriorHeroGameplayAbility.h/.cpp`

**功能**: 玩家角色专用技能基类。

**伤害效果创建**:
```cpp
FGameplayEffectSpecHandle UWarriorHeroGameplayAbility::MakeHeroDamageEffectSpecHandle(
    TSubclassOf<UGameplayEffect> EffectClass, 
    float InWeaponBaseDamage, 
    FGameplayTag InCurrentAttackTypeTag, 
    int32 InUsedComoboCount)
{
    FGameplayEffectContextHandle ContextHandle = GetWarriorAbilitySystemComponentFromActorInfo()->MakeEffectContext();
    ContextHandle.SetAbility(this);
    ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
    ContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

    FGameplayEffectSpecHandle EffectSpecHandle = GetWarriorAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
        EffectClass,
        GetAbilityLevel(),
        ContextHandle
    );

    // 设置基础伤害
    EffectSpecHandle.Data->SetSetByCallerMagnitude(
        WarriorGameplayTags::Shared_SetByCaller_BaseDamage,
        InWeaponBaseDamage
    );
    
    // 设置攻击类型和连击数
    if (InCurrentAttackTypeTag.IsValid())
    {
        EffectSpecHandle.Data->SetSetByCallerMagnitude(InCurrentAttackTypeTag, InUsedComoboCount);
    }
    
    return EffectSpecHandle;
}
```

---

#### 3.2.5 UGEExecCalc_DamageTaken（伤害计算器）

**文件**: `AbilitySystem/GEExecCalc/GEExecCalc_DamageTaken.h/.cpp`

**功能**: 自定义的伤害计算逻辑。

**伤害公式**:
```
最终伤害 = 基础伤害 × 攻击力 ÷ 防御力 × 连击加成
```

**连击加成规则**:
- **轻攻击**: 每次连击增加5%伤害
- **重攻击**: 每次连击增加15%伤害

**核心计算逻辑**:
```cpp
void UGEExecCalc_DamageTaken::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& EffectSpec = ExecutionParams.GetOwningSpec();
    
    // 获取攻击者攻击力
    float SourceAttackPower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        GetWarriorDamageCapture().AttackPowerDef, 
        EvaluateParameters, 
        SourceAttackPower
    );

    // 获取目标防御力
    float TargetDefensePower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        GetWarriorDamageCapture().DefensePowerDef, 
        EvaluateParameters, 
        TargetDefensePower
    );

    // 从SetByCaller获取基础伤害和连击数
    float BaseDamage = 0.f;
    int32 UsedLightAttackComboCount = 0;
    int32 UsedHeavyAttackComboCount = 0;
    
    for (const TPair<FGameplayTag, float>& TagNagnitude : EffectSpec.SetByCallerTagMagnitudes)
    {
        if (TagNagnitude.Key.MatchesTagExact(WarriorGameplayTags::Shared_SetByCaller_BaseDamage))
            BaseDamage = TagNagnitude.Value;
        if (TagNagnitude.Key.MatchesTagExact(WarriorGameplayTags::Player_SetByCaller_AttackType_Light))
            UsedLightAttackComboCount = TagNagnitude.Value;
        if (TagNagnitude.Key.MatchesTagExact(WarriorGameplayTags::Player_SetByCaller_AttackType_Heavy))
            UsedHeavyAttackComboCount = TagNagnitude.Value;
    }

    // 应用连击加成
    if (UsedLightAttackComboCount != 0)
    {
        const float DamageIncreasePercentLight = (UsedLightAttackComboCount - 1) * 0.05f + 1.f;
        BaseDamage *= DamageIncreasePercentLight;
    }
    if (UsedHeavyAttackComboCount != 0)
    {
        const float DamageIncreasePercentHeavy = UsedHeavyAttackComboCount * 0.15f + 1.f;
        BaseDamage *= DamageIncreasePercentHeavy;
    }

    // 计算最终伤害
    const float FinalDamageDone = BaseDamage * SourceAttackPower / TargetDefensePower;
    
    if (FinalDamageDone > 0)
    {
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(
                GetWarriorDamageCapture().DamageTakenProperty,
                EGameplayModOp::Override,
                FinalDamageDone
            )
        );
    }
}
```

---

### 3.3 战斗系统 (Combat System)

#### 3.3.1 UPawnCombatComponent（战斗组件基类）

**文件**: `Components/Combat/PawnCombatComponent.h/.cpp`

**功能**: 管理角色的武器和战斗行为。

**核心数据**:
- `CharacterCarriedWeaponMap` - 武器Tag到武器实例的映射
- `CurrentEquippedWeaponTag` - 当前装备的武器Tag
- `OverlappedActors` - 当前攻击命中的角色列表

**武器管理**:
```cpp
void UPawnCombatComponent::RegisterSpawnedWeapon(
    FGameplayTag InWeaponTagToRegister, 
    AWarriorWeaponBase* InWeaponToRegister, 
    bool bRegisterAsEquippedWeapon)
{
    CharacterCarriedWeaponMap.Emplace(InWeaponTagToRegister, InWeaponToRegister);
    
    // 绑定武器碰撞回调
    InWeaponToRegister->OnWeaponHitTarget.BindUObject(this, &ThisClass::OnHitTargetActor);
    InWeaponToRegister->OnWeaponPulledFromTarget.BindUObject(this, &ThisClass::OnWeaponPulledFromTargetActor);
    
    if (bRegisterAsEquippedWeapon)
    {
        CurrentEquippedWeaponTag = InWeaponTagToRegister;
    }
}

void UPawnCombatComponent::ToggleWeaponCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType)
{
    if (ToggleDamageType == EToggleDamageType::CurrentEquippedWeapon)
    {
        AWarriorWeaponBase* WeaponToToggle = GetCharacterCurrentEquippedWeapon();
        
        if (bShouldEnable)
        {
            WeaponToToggle->GetWeaponCollisionBox()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        else
        {
            WeaponToToggle->GetWeaponCollisionBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            OverlappedActors.Empty();  // 清空命中列表
        }
    }
}
```

---

#### 3.3.2 UHeroCombatComponent（英雄战斗组件）

**文件**: `Components/Combat/HeroCombatComponent.h/.cpp`

**功能**: 玩家角色的战斗逻辑。

**命中处理**:
```cpp
void UHeroCombatComponent::OnHitTargetActor(AActor* HitActor)
{
    // 防止重复命中
    if (OverlappedActors.Contains(HitActor)) return;
    OverlappedActors.AddUnique(HitActor);

    // 构造事件数据
    FGameplayEventData Data;
    Data.Instigator = GetOwningPawn();
    Data.Target = HitActor;

    // 发送近战命中事件
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        GetOwningPawn(),
        WarriorGameplayTags::Shared_Event_MeleeHit,
        Data
    );

    // 触发顿帧效果
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        GetOwningPawn(),
        WarriorGameplayTags::Player_Event_HitPause,
        FGameplayEventData()
    );
}
```

---

#### 3.3.3 UEnemyCombatComponent（敌人战斗组件）

**文件**: `Components/Combat/EnemyCombatComponent.h/.cpp`

**功能**: 敌人角色的战斗逻辑，支持格挡检测。

```cpp
void UEnemyCombatComponent::OnHitTargetActor(AActor* HitActor)
{
    if (OverlappedActors.Contains(HitActor)) return;
    OverlappedActors.AddUnique(HitActor);

    bool bIsValidBlock = false;
    const bool bIsPlayerBlocking = false;      // TODO: 实现格挡检测
    const bool bIsMyAttackUnblockable = false; // TODO: 实现破防攻击

    FGameplayEventData EventData;
    EventData.Instigator = GetOwningPawn();
    EventData.Target = HitActor;

    if (bIsValidBlock)
    {
        // TODO: 处理成功格挡
    }
    else
    {
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            GetOwningPawn(),
            WarriorGameplayTags::Shared_Event_MeleeHit,
            EventData
        );
    }
}
```

---

### 3.4 武器系统 (Weapons)

#### 3.4.1 AWarriorWeaponBase（武器基类）

**文件**: `Items/Weapons/WarriorWeaponBase.h/.cpp`

**功能**: 所有武器的基类，处理碰撞检测和伤害事件。

**核心组件**:
- `UStaticMeshComponent` - 武器网格
- `UBoxComponent` - 碰撞检测盒

**碰撞处理**:
```cpp
void AWarriorWeaponBase::OnCollisionBeginOverlap(
    UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult)
{
    // 获取武器所有者
    APawn* WeaponOwingPawn = GetInstigator<APawn>();
    
    if (APawn* HitPawn = Cast<APawn>(OtherActor))
    {
        // 检查是否为敌对目标
        if (UWarriorFunctionLibrary::IsTargetPawnHostile(WeaponOwingPawn, HitPawn))
        {
            OnWeaponHitTarget.ExecuteIfBound(OtherActor);
        }
    }
}
```

**委托**:
- `OnWeaponHitTarget` - 武器命中目标时触发
- `OnWeaponPulledFromTarget` - 武器离开目标时触发

---

#### 3.4.2 AWarriorHeroWeapon（英雄武器类）

**文件**: `Items/Weapons/WarriorHeroWeapon.h/.cpp`

**功能**: 玩家使用的武器，包含技能和伤害数据。

**数据结构**:
```cpp
struct FWarriorHeroWeaponData
{
    FScalableFloat WeaponBaseDamage;  // 可缩放的基础伤害
    // ... 其他武器属性
};
```

**技能句柄管理**:
```cpp
void AWarriorHeroWeapon::AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InSpecHandles)
{
    GrantedAbilitySpecHandles = InSpecHandles;
}
```

---

### 3.5 AI系统 (Artificial Intelligence)

#### 3.5.1 AWarriorAIController（AI控制器）

**文件**: `Controller/WarriorAIController.h/.cpp`

**功能**: 敌人的AI控制器，集成感知系统和群体避障。

**核心配置**:
- **感知系统**: 视觉感知，360度视野，5000单位感知范围
- **阵营系统**: TeamId = 1（敌方）
- **群体避障**: 使用Detour Crowd Avoidance

**感知配置**:
```cpp
AWarriorAIController::AWarriorAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>("PathFollowingComponent"))
{
    // 配置视觉感知
    AISenseConfig_Sight = CreateDefaultSubobject<UAISenseConfig_Sight>("EnemySenseConfig_Sight");
    AISenseConfig_Sight->DetectionByAffiliation.bDetectEnemies = true;
    AISenseConfig_Sight->DetectionByAffiliation.bDetectFriendlies = false;
    AISenseConfig_Sight->SightRadius = 5000.0f;
    AISenseConfig_Sight->PeripheralVisionAngleDegrees = 360.0f;

    EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("EnemyPerceptionComponent");
    EnemyPerceptionComponent->ConfigureSense(*AISenseConfig_Sight);
    EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ThisClass::OnEnemyPerceptionUpdated);

    SetGenericTeamId(FGenericTeamId(1));  // 设置为敌方阵营
}
```

**阵营态度判断**:
```cpp
ETeamAttitude::Type AWarriorAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    const APawn* PawnToCheck = Cast<const APawn>(&Other);
    const IGenericTeamAgentInterface* OtherTeamAgant = Cast<const IGenericTeamAgentInterface>(PawnToCheck->GetController());
    
    if (OtherTeamAgant && OtherTeamAgant->GetGenericTeamId() < GetGenericTeamId())
    {
        return ETeamAttitude::Hostile;
    }
    return ETeamAttitude::Friendly;
}
```

**感知更新处理**:
```cpp
void AWarriorAIController::OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
    {
        if (!BlackboardComponent->GetValueAsObject(FName("TargetActor")))
        {
            if (Stimulus.WasSuccessfullySensed() && Actor)
            {
                BlackboardComponent->SetValueAsObject(FName("TargetActor"), Actor);
            }
        }
    }
}
```

---

#### 3.5.2 UBTTask_RotateToFaceTarget（行为树任务）

**文件**: `AI/BTTask_RotateToFaceTarget.h/.cpp`

**功能**: 平滑旋转面向目标的行为树任务。

**参数**:
- `AnglePercision` - 角度精度（默认10度）
- `RotationInterpSpeed` - 旋转插值速度（默认5）
- `InTargetToFaceKey` - 黑板键（目标Actor）

**实现逻辑**:
```cpp
void UBTTask_RotateToFaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FRotateToFaceTargetTaskMemory* Memory = CastInstanceNodeMemory<FRotateToFaceTargetTaskMemory>(NodeMemory);

    if (HasReachedAnglePercision(Memory->OwningPawn.Get(), Memory->TargetActor.Get()))
    {
        Memory->Reset();
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
    else
    {
        // 计算目标旋转
        const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(
            Memory->OwningPawn->GetActorLocation(),
            Memory->TargetActor->GetActorLocation()
        );
        // 平滑插值
        const FRotator TargetRot = FMath::RInterpTo(
            Memory->OwningPawn->GetActorRotation(), 
            LookAtRot, 
            DeltaSeconds, 
            RotationInterpSpeed
        );
        Memory->OwningPawn->SetActorRotation(TargetRot);
    }
}

bool UBTTask_RotateToFaceTarget::HasReachedAnglePercision(APawn* QueryPawn, AActor* TargetActor) const
{
    const FVector OwnerForward = QueryPawn->GetActorForwardVector();
    const FVector OwnerToTargetNormalized = (TargetActor->GetActorLocation() - QueryPawn->GetActorLocation()).GetSafeNormal();
    
    const float DotResult = FVector::DotProduct(OwnerForward, OwnerToTargetNormalized);
    const float AngleDiff = UKismetMathLibrary::DegAcos(DotResult);
    
    return AngleDiff <= AnglePercision;
}
```

---

### 3.6 动画系统 (Animation)

#### 3.6.1 UWarriorCharacterAnimInstance（角色动画实例）

**文件**: `AnimInstance/WarriorCharacterAnimInstance.h/.cpp`

**功能**: 提供动画蓝图所需的运行时数据。

**动画参数**:
- `GroundSpeed` - 地面速度
- `bHasAcceleration` - 是否有加速度
- `LocomotionDirection` - 运动方向角度

```cpp
void UWarriorCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    if (!OwningMovementComponent || !OwningCharacter) return;
    
    // 计算地面速度（忽略Z轴）
    GroundSpeed = OwningCharacter->GetVelocity().Size2D();
    
    // 判断是否有加速度
    bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.f;
    
    // 计算运动方向
    LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(
        OwningCharacter->GetVelocity(), 
        OwningCharacter->GetActorRotation()
    );
}
```

---

### 3.7 数据资产 (Data Assets)

#### 3.7.1 UDataAsset_StarUpDataBase（启动数据基类）

**文件**: `DataAssets/StarUpData/DataAsset_StarUpDataBase.h/.cpp`

**功能**: 角色初始化时授予的技能和效果配置。

**数据内容**:
- `ActivateOnGivenAbilities` - 授予时立即激活的技能
- `ReactiveAbilities` - 响应式技能
- `StartUpGameplayEffects` - 启动时应用的效果

```cpp
void UDataAsset_StarUpDataBase::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
    // 授予技能
    GrantAbilities(ActivateOnGivenAbilities, InASCToGive, ApplyLevel);
    GrantAbilities(ReactiveAbilities, InASCToGive, ApplyLevel);

    // 应用启动效果
    for (const TSubclassOf<UGameplayEffect> EffectClass : StartUpGameplayEffects)
    {
        if (!EffectClass) continue;
        UGameplayEffect* EffectDOC = EffectClass->GetDefaultObject<UGameplayEffect>();
        InASCToGive->ApplyGameplayEffectToSelf(
            EffectDOC,
            ApplyLevel,
            InASCToGive->MakeEffectContext()
        );
    }
}
```

---

#### 3.7.2 UDataAsset_InputConfig（输入配置）

**文件**: `DataAssets/Input/DataAsset_InputConfig.h/.cpp`

**功能**: 将GameplayTag与输入动作关联。

**查找输入动作**:
```cpp
UInputAction* UDataAsset_InputConfig::FindNativeInputActionByTag(const FGameplayTag& InInputTag) const
{
    for (const FWarriorInputActionConfig& InputActionConfig : NativeInputActions)
    {
        if (InputActionConfig.InputTag == InInputTag && InputActionConfig.InputAction)
        {
            return InputActionConfig.InputAction;
        }
    }
    return nullptr;
}
```

---

### 3.8 UI系统 (User Interface)

#### 3.8.1 UPawnUIComponent（UI组件基类）

**文件**: `Components/UI/PawnUIComponent.h/.cpp`

**功能**: 提供角色UI相关的委托和数据。

**委托**:
- `OnCurrentHealthChanged` - 生命值变化委托

---

#### 3.8.2 UWarriorWidgetBase（Widget基类）

**文件**: `Widgets/WarriorWidgetBase.h/.cpp`

**功能**: 游戏UI控件的基类，提供初始化接口。

```cpp
void UWarriorWidgetBase::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // 尝试获取玩家角色的UI组件
    if (IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(GetOwningPlayerPawn()))
    {
        if (UHeroUIComponent* HeroUIComponent = PawnUIInterface->GetHeroUIComponent())
        {
            BP_OnOwningHeroUIComponentInitialized(HeroUIComponent);
        }
    }
}

void UWarriorWidgetBase::InitEnemyCreatedWidget(AActor* OwningEnemyActor)
{
    if (IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(OwningEnemyActor))
    {
        UEnemyUIComponent* EnemyUIComponent = PawnUIInterface->GetEnemyUIComponent();
        BP_OnOwningEnemyUIComponentInitialized(EnemyUIComponent);
    }
}
```

---

### 3.9 工具库 (Utilities)

#### 3.9.1 UWarriorFunctionLibrary（函数库）

**文件**: `WarriorFunctionLibrary.h/.cpp`

**功能**: 全局静态工具函数。

**主要函数**:

| 函数 | 功能 |
|------|------|
| `NativeGetWarriorASCFromActor` | 获取Actor的ASC组件 |
| `AddGameplayTagToActorIfNone` | 安全添加GameplayTag |
| `RemoveGameplayTagFromActorIfFound` | 安全移除GameplayTag |
| `NativeDoesActorHaveTag` | 检查是否拥有指定Tag |
| `NativeGetPawnCombatComponentFromActor` | 获取战斗组件 |
| `IsTargetPawnHostile` | 判断目标是否敌对 |
| `GetScalableFloatValueAtLevel` | 获取等级化数值 |

**敌对判断实现**:
```cpp
bool UWarriorFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
    IGenericTeamAgentInterface* QueryTeamAgent = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController());
    IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController());

    if (QueryTeamAgent && TargetTeamAgent)
    {
        return QueryTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
    }
    return false;
}
```

---

### 3.10 Gameplay Tags

**文件**: `WarriorGamePlayTags.h/.cpp`

**功能**: 定义游戏中使用的所有GameplayTag。

**Tag分类**:

#### 输入Tags
```cpp
InputTag.Move          // 移动
InputTag.Look          // 视角
InputTag.Jump          // 跳跃
InputTag.EquipAxe      // 装备斧头
InputTag.UnequipAxe    // 卸下斧头
InputTag.LightAttack.Axe   // 轻攻击
InputTag.HeavyAttack.Axe   // 重攻击
InputTag.Roll          // 翻滚
```

#### 玩家Tags
```cpp
Player.Ability.Equip.Axe        // 装备斧头技能
Player.Ability.Attack.Light.Axe // 轻攻击技能
Player.Ability.Attack.Heavy.Axe // 重攻击技能
Player.Ability.HitPause         // 顿帧技能
Player.Ability.Roll             // 翻滚技能
Player.Weapon.Axe               // 斧头武器
Player.Event.Equip.Axe          // 装备事件
Player.Event.HitPause           // 顿帧事件
Player.Status.JumpToFinisher    // 跳转终结状态
Player.Status.Rolling           // 翻滚中状态
Player.SetByCaller.AttackType.Light  // 轻攻击类型
Player.SetByCaller.AttackType.Heavy  // 重攻击类型
```

#### 敌人Tags
```cpp
Enemy.Weapon            // 敌人武器
Enemy.Ability.Melee     // 近战技能
Enemy.Ability.Ranged    // 远程技能
Enemy.Status.Strafing   // 绕行状态
Enemy.Status.UnderAttack // 受击状态
```

#### 共享Tags
```cpp
Shared.Ability.HitReact      // 受击反应技能
Shared.Ability.Death         // 死亡技能
Shared.Event.MeleeHit        // 近战命中事件
Shared.Event.HitReact        // 受击反应事件
Shared.SetByCaller.BaseDamage // 基础伤害
Shared.Status.Dead           // 死亡状态
```

---

## 四、系统交互流程

### 4.1 攻击伤害流程

```
玩家输入攻击
    ↓
InputComponent 检测输入Tag
    ↓
AbilitySystemComponent::OnAbilityInputPressed()
    ↓
激活攻击技能 (GA_Hero_LightAttack / GA_Hero_HeavyAttack)
    ↓
播放攻击蒙太奇
    ↓
AnimNotifyState 开启武器碰撞
    ↓
武器碰撞检测命中敌人
    ↓
AWarriorWeaponBase::OnCollisionBeginOverlap()
    ↓
检查目标是否敌对 (UWarriorFunctionLibrary::IsTargetPawnHostile)
    ↓
UHeroCombatComponent::OnHitTargetActor()
    ↓
发送 Shared_Event_MeleeHit 事件
    ↓
技能响应事件，创建 GameplayEffectSpec
    ↓
UGEExecCalc_DamageTaken 计算最终伤害
    ↓
修改目标 DamageTaken 属性
    ↓
UWarriorAttributeSet::PostGameplayEffectExecute()
    ↓
更新 CurrentHealth，广播 UI 更新
    ↓
如果 CurrentHealth = 0，添加 Shared_Status_Dead Tag
```

### 4.2 敌人AI感知流程

```
玩家进入感知范围
    ↓
UAIPerceptionComponent 检测到玩家
    ↓
OnEnemyPerceptionUpdated() 回调
    ↓
设置黑板键 TargetActor = 玩家
    ↓
行为树获取 TargetActor
    ↓
执行追踪/攻击行为
```

---

## 五、蓝图资产

### Content目录结构

```
Content/
├── EnemyCharacters/           # 敌人角色
│   ├── BTDecorator/          # 行为树装饰器
│   ├── BTService/            # 行为树服务
│   ├── BTTask/               # 行为树任务
│   ├── EQS/                  # 环境查询系统
│   └── Gruntling/            # 小兵敌人
├── PlayerCharacters/          # 玩家角色
│   ├── AnimBP/               # 动画蓝图
│   ├── GameplayEffect/       # 游戏效果
│   ├── GamplayAbility/       # 技能蓝图
│   ├── HeroWeapons/          # 英雄武器
│   ├── Input/                # 输入配置
│   └── Montages/             # 动画蒙太奇
├── Shared/                    # 共享资产
│   ├── AnimNotify/           # 动画通知
│   ├── AnimNotifyState/      # 动画通知状态
│   ├── GameplayAbility/      # 共享技能
│   └── GameplayEffect/       # 共享效果
└── Widgets/                   # UI控件
    ├── EnemyWidgets/         # 敌人UI
    ├── HeroWidgets/          # 英雄UI
    └── TemplateWidgets/      # 模板UI
```

---

## 六、已实现功能

- ✅ 轻攻击和重攻击的组合系统
- ✅ 小怪的受击表现以及属性更新
- ✅ 主角和小怪的血条，主角的武器图标
- ✅ 小怪的攻击行为AI
- ✅ 主角的定向翻滚
- ✅ 伤害计算系统（含连击加成）
- ✅ 阵营系统（玩家vs敌人）
- ✅ 群体避障系统

---

## 七、待实现功能

- ⬜ 主角受击表现
- ⬜ 防御系统
- ⬜ 格挡系统
- ⬜ 目标锁定
- ⬜ 具有冷却时间的特殊武器技能
- ⬜ 怪物的武器发射
- ⬜ Boss战斗

---

## 八、使用说明

### 环境要求
- Unreal Engine 5.x
- Visual Studio 2022
- Windows 10/11

### 项目设置
```bash
git clone https://github.com/MZHuangJie/Warrior.git
cd Warrior
右键 Warrior.uproject -> Generate Visual Studio Project Files
双击 Warrior.uproject 打开编辑器
```

### 调试技巧
项目提供了 `WarriorDebugHelper.h` 用于调试输出，支持在屏幕上打印变量值。

---

## 九、总结

Warrior 项目是一个结构清晰、模块化设计良好的 UE5 动作RPG原型。项目充分利用了 Gameplay Ability System 的强大功能，实现了完整的战斗系统框架，为后续扩展提供了良好的基础。核心架构遵循了虚幻引擎的最佳实践，采用组件化设计、接口抽象和数据驱动的方式，使代码具有良好的可维护性和可扩展性。
