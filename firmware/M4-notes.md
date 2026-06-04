# M4 实现说明 + 残留

M4 趣味打磨:非阻塞振动、自绘螺旋眼/爱心/冷汗/Zzz/眩晕星、Drowsy/Relief 状态、周期审批提醒。
native 35 测试 + 编译通过;真板视觉验证待做。

## 自绘实现要点(m5stack-avatar 0.10.0)
- 库无 plugin/accessory、Face 无 setEffect。自绘走:① 自定义 Drawable 子类;
  ② SpiralEye 经 setLeftEye/setRightEye 切换(Dizzy);③ BuddyEffect 装入 setMouth 槽
  (先委托真实 Mouth 再叠画符号,嘴不丢);④ avatar.setFace 注入。
- 底脸库表情每帧按 buddy 表情选择性设(baseLibExpression):Love/Happy/Dizzy/Sleepy 设 Neutral
  (库 Effect noop,符号由 BuddyEffect 自绘,避免双画);Sad/Angry/Doubt 设库对应表情
  (BuddyEffect 对它们 noop,不双画),保住这三态可读性。
- g_buddyFx(buddy_fx.h)是主线程→avatar 线程共享状态(单写单读 POD,撕裂仅一帧毛刺)。

## 残留 / 限制
- 冒汗强度依赖 running 计数,M3 已知:连接前会话漏计(纯 SSE)。
- 哈欠动画未做(YAGNI,Zzz 已表达打盹)。
- Drowsy 与未连接当前视觉**无法区分**(都是 Sleepy 底 + Off LED + "zzz" 文字 + Zzz 符号)。区分二者需给 Drowsy 独立标签/LED,留待后续。
- SpiralEye 每帧三角函数(~2-4% 帧预算),真板若帧紧可改查表。
- 库默认 Mouth 被 setMouth 替换后一次性泄漏 ~20 字节(嵌入式永不析构,可接受)。
- 真板视觉(螺旋眼/爱心/冒汗/Zzz 的观感、位置、不闪烁)待烧录肉眼验证。
- Relief("松口气")当前视觉=Happy+冷汗+"busy" 标签,与轻度运行态难区分;逻辑/测试正确但真板观感弱,后续给独立标签/动画。
- (已修)Deny/Error/Doubt 曾因底脸恒 Neutral 丢库表情;现按表情选择性设库底脸恢复(Love/Happy/Dizzy/Sleepy 用 Neutral 自绘,Sad/Angry/Doubt 用库表情)。

## M4.1 表情夸张化(render 层)

M4.1 只动 render 层把表情夸张,**不改状态机/切换逻辑/表情数量**。

- 做了什么:`driveFace()`(`face_drive.h`)逐表情设库脸参数,主 loop 每帧调用——Dizzy/Angry 用 `setRotation` 摇头/抖动、Love 用 `setScale` 弹跳 + 大嘴(`setMouthOpenRatio`)、Sleepy `eyeOpen≈0.05` 闭眼、Doubt `eyeOpen=1.0` 瞪眼、Sad `eyeOpen=0.3` 半闭。自绘符号整体放大;Doubt 新增感叹号、Sad 新增泪滴;螺旋眼线条加粗。底脸库表情 + 标签仍按 `if (v.expression != lastExpression)` 低频设(setExpression + setSpeechText),与每帧 driveFace 并存不冲突。
- 关键集成:setup() 在 `avatar.init()` 后调 `avatar.setIsAutoBlink(false)`,让 driveFace 全权接管 `eyeOpenRatio`。否则库 facialLoop 默认每 2.5~4.5s 自动眨眼(把 eyeOpenRatio 拍 0→1.0),会周期性顶掉 driveFace 设的固定眼态,Sleepy 闭眼/Doubt 瞪眼/Sad 半闭(本次区分表情的主力)就被破坏。
- 关闭自动眨眼的副作用/权衡:Neutral 显式 `setEyeOpenRatio(1.0)` 保证睁眼——否则从 Sleepy(0.05)/Sad(0.3)/Dizzy(0.4) 切回时眼睛会卡在前一表情的小值("睡醒后睁不开眼")。代价=Neutral 不再有眨眼动作(恒定睁)。follow-up 可考虑只在 Neutral 临时开 autoBlink 恢复眨眼、切出 Neutral 时再关。
- 已知限制 / 真板待验(📋):
  - ① breath 双线程竞争:库 facialLoop 每帧无条件 `setBreath(sin(...))`,**无 API 可关**;driveFace 每帧也写 breath(Happy 快 / Sleepy 慢 / 其余 0),两线程 last-writer-wins → breath 是最弱信号,真板看 Happy 快呼吸 / Sleepy 慢呼吸是否可感知。
  - ② Doubt 感叹号 / Sad 泪滴 与库底脸 Doubt/Sad 是否视觉重叠,真板评估。
  - ③ setRotation 摇头/抖、setScale 弹跳的幅度(Dizzy ±0.15 / Angry ±0.05 / Love 0.08)spec 数值仅为起点,真板可能需调。
  - ④ 各夸张数值的节奏(sin 除数)真板手感待调。
