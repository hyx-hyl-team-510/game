LightTrace Echo | Warrior of Chronos

📖 Project Introduction
LightTrace Echo is a standalone 2D action-adventure platformer developed with Qt and C++17, featuring three core mechanics: AI-procedurally generated weapons, dual light/dark world switching, and an original time-freeze combo combat system.
Torn apart by a spacetime rift, the world splits into the Realm of Light and the Shadow Realm. Players take on the role of a transdimensional warrior who traverses both realms with time-freeze powers, defeating the final boss to mend the spacetime crack and save the collapsing world.
The game is cross-platform compatible with Windows, macOS and Linux.

✨ Core Features
Native DeepSeek API AI Integration
After clearing each stage, the AI generates brand-new randomized weapons which are automatically added to the in-game shop.
Dynamic AI-driven dialogue pops up randomly during gameplay to enhance narrative immersion.
AI assisted the team with framework coding throughout development.
Dual-Realm Map Mechanic
Realm of Light: Full map visibility; enemies have higher HP and attack values for high-risk high-reward gameplay.
Shadow Realm: Pitch-black map with vision limited only to the area surrounding the player; enemies are weaker in stats.
Both realms share identical base maps. Undefeated enemies respawn when switching back between realms; the final boss fight merges both worlds into one battlefield.
Original Time-Stop Combo Combat System
Press key L to activate a 3-second global time freeze:
All in-game entities stop moving, allowing unrestricted player movement and attacks.
Every basic attack marks a positional coordinate on the map.
When time resumes, the player teleports sequentially to all marked positions to unleash chained strikes before returning to the original spot.

🎮 Game Controls
Key	Function
W/A/S/D	Character Movement
J	Basic Attack
K	Swap Equipped Weapon
T	Toggle between Light / Shadow Realm
L	Activate Time-Freezing Ultimate Skill
U	Use HP Potion
I	Use Mana/Energy Potion
O	Fireball Ranged Attack
B	Open Shop Menu (when adjacent to shop NPC)

🧩 Game System Modules
Dual World Switch System: Instant environment toggle with independent enemy spawn rules and vision settings for each realm.
Real-time Combat System: Weapon switching, consumable potions, fireball projectile and signature time-freeze ultimate ability.
Shop & Progression System: Spend in-game currency to purchase AI-generated random weapons, healing/energy potions and miscellaneous consumables.
Stage & Boss System: Campaign progresses synchronously across both realms; the concluding boss battle merges two worlds, and defeating the boss completes the full playthrough.

🛠️ Technical Details
Development Environment: Qt 6.10.1 + MinGW
Programming Language: C++17
AI Solution: DeepSeek LLM (for project framework auxiliary development) + DeepSeek API (in-game procedural weapon & dialogue generation)
Core Architecture: Object-Oriented entity design. A unified EntityManager manages all game objects including playable character, enemies, projectiles, items and map colliders.

🤝 Development Team
hyxCoding, ffpinkle

📄 License
This open-source project is licensed under the MIT License.



超时空勇士：光痕回响 | LightTrace Echo

📖 项目简介
超时空勇士：光痕回响是基于 Qt + C++17 开发的 2D 动作冒险闯关单机游戏，主打AI 动态生成武器、光暗双界切换、独创时停连招三大核心玩法。
世界因时空裂痕分裂为光明界与黯界，玩家化身跨时空勇士，依靠时停之力穿梭双地图，击败最终 BOSS 修复时空裂隙，阻止世界崩坏。
支持 Windows /macOS/ Linux 全平台运行。

✨ 核心亮点
1. 原生 AI 接入游戏（DeepSeek API）
关卡通关后 AI 随机生成全新武器，自动上架商店
实时 AI 对话生成，随机台词弹窗，强化剧情沉浸氛围
项目开发结合 AI 辅助编码
2. 光暗双界同图机制
光明界：全图视野开放，怪物血量与攻击力偏高
黯界：仅角色周身小范围可视，全域黑暗，怪物强度更低
同一地图两套环境，切换世界未清理怪物再次返回将会刷新；BOSS 战强制双界地图融合
3. 原创时停连招战斗系统
按下 L 触发 3 秒全局时停：
世界静止，玩家自由移动、自由攻击
每次普攻记录一处攻击坐标点位
时停结束角色依次瞬移至所有标记点位连锁打击，最后回归原位

🎮 游戏操作按键
按键	功能
W/A/S/D	角色移动
J	普通攻击
K	切换持有武器
T	光 / 黯世界切换
L	开启时间停止大招
U	使用生命药水
I	使用能量药水
O	火球远程攻击
B	靠近商店打开购物面板

🧩 游戏系统模块
双界切换系统：随时切换地图环境，怪物刷新、视野规则独立
即时战斗系统：多武器轮换、药水消耗、火球远程、大招时停
商店养成系统：使用资源购买 AI 随机武器、红蓝药水、消耗道具
关卡 & BOSS 系统：双世界同步推进流程，终局 BOSS 战双界融合，击杀 BOSS 达成通关

🛠️ 技术详情
开发环境：Qt 6.10.1 + MinGW
编程语言：C++17
AI 方案：DeepSeek 大模型（辅助项目框架开发）+ DeepSeek API（游戏内动态生成武器、台词）
核心架构：面向对象实体架构，统一实体管理器EntityManager管控角色、怪物、投射物、道具、墙体等

🤝 开发团队
hyxCoding
ffpinkle

📄 License
本项目开源遵循 MIT 协议
