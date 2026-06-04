# M4 实现说明 + 残留

M4 趣味打磨:非阻塞振动、自绘螺旋眼/爱心/冷汗/Zzz/眩晕星、Drowsy/Relief 状态、周期审批提醒。
native 35 测试 + 编译通过;真板视觉验证待做。

## 自绘实现要点(m5stack-avatar 0.10.0)
- 库无 plugin/accessory、Face 无 setEffect。自绘走:① 自定义 Drawable 子类;
  ② SpiralEye 经 setLeftEye/setRightEye 切换(Dizzy);③ BuddyEffect 装入 setMouth 槽
  (先委托真实 Mouth 再叠画符号,嘴不丢);④ avatar.setFace 注入。
- 底脸恒 setExpression(Neutral):库内置 Effect 在 Neutral 时 noop,避免与 BuddyEffect 双画。
- g_buddyFx(buddy_fx.h)是主线程→avatar 线程共享状态(单写单读 POD,撕裂仅一帧毛刺)。

## 残留 / 限制
- 冒汗强度依赖 running 计数,M3 已知:连接前会话漏计(纯 SSE)。
- 哈欠动画未做(YAGNI,Zzz 已表达打盹)。
- Drowsy 与未连接都用 Sleepy 底,靠 BuddyEffect 的 Zzz + 气泡文字区分;未加独立 Drowsy 文字标签。
- SpiralEye 每帧三角函数(~2-4% 帧预算),真板若帧紧可改查表。
- 库默认 Mouth 被 setMouth 替换后一次性泄漏 ~20 字节(嵌入式永不析构,可接受)。
- 真板视觉(螺旋眼/爱心/冒汗/Zzz 的观感、位置、不闪烁)待烧录肉眼验证。
