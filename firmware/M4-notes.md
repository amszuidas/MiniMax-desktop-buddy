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
