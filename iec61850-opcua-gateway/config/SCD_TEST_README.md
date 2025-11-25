# SCD Test Files for Dynamic Topology Generation

本目录包含3个测试SCD文件，用于验证动态拓扑生成功能：

## 1. `prp_4ieds.scd` - 小型PRP站点
**架构**: PRP (Parallel Redundancy Protocol)  
**IED数量**: 4个  
**网络拓扑**: 双网并行 (NetworkA + NetworkB)  
**IED列表**:
- ProtectionIED1 (保护装置1)
- ProtectionIED2 (保护装置2)
- MeterIED1 (电表1)
- ControlIED1 (控制器1)

**IP地址段**:
- Network A: 192.168.10.x
- Network B: 192.168.20.x

**预期效果**: 4个节点水平排列，每对节点间有双线连接（绿色实线 + 蓝色虚线）

---

## 2. `hsr_6ieds.scd` - HSR环网站点
**架构**: HSR (High-availability Seamless Redundancy)  
**IED数量**: 6个  
**网络拓扑**: 环形冗余网络  
**IED列表**:
- RingIED1~6 (环网设备1-6)

**IP地址段**:
- HSR Ring: 192.168.100.x
- VLAN: 100

**预期效果**: 6个节点圆形排列，首尾相连形成环，每个连线上有流动的消息动画

---

## 3. `prp_10ieds.scd` - 大型PRP站点
**架构**: PRP (Parallel Redundancy Protocol)  
**IED数量**: 10个  
**网络拓扑**: 双网并行 (Station_Network_A + Station_Network_B)  
**IED列表**:
- Bay1_Prot, Bay1_Ctrl (间隔1保护+控制)
- Bay2_Prot, Bay2_Ctrl (间隔2保护+控制)
- Bay3_Prot, Bay3_Ctrl (间隔3保护+控制)
- Busbar_Prot (母线保护)
- StationMeter1, StationMeter2 (站用电表)
- SCADA_Gateway (网关)

**IP地址段**:
- Network A: 10.1.1.x
- Network B: 10.1.2.x

**预期效果**: 自动切换为网格布局（4列3行），双网连接清晰可见

---

## 测试方法

### 方法1: 手动软链接
```bash
cd /Users/danielliu/IEC6850/iec61850-opcua-gateway/config

# 测试 PRP 4节点
ln -sf prp_4ieds.scd station.scd
../build/./iec61850-opcua-gateway

# 测试 HSR 6节点
ln -sf hsr_6ieds.scd station.scd  
# 重启网关

# 测试 PRP 10节点
ln -sf prp_10ieds.scd station.scd
# 重启网关
```

### 方法2: 浏览器URL参数（使用Mock数据）
```
# PRP 5节点
http://localhost:6850/?topology=prp&nodes=5

# HSR 8节点
http://localhost:6850/?topology=hsr&nodes=8
```

---

## 验证要点

### PRP验证:
✅ 顶部标签显示 "PRP (N nodes)"  
✅ 双网健康卡片: Network A (eth0) + Network B (eth1)  
✅ 拓扑图连线: 绿色实线 + 蓝色虚线并行  
✅ 每个IED显示两个IP地址  

### HSR验证:
✅ 顶部标签显示 "HSR (N nodes)"  
✅ 单网健康卡片: HSR Ring  
✅ 拓扑图布局: 圆形环状  
✅ 首尾节点相连形成闭环  
✅ VLAN ID: 100

---

## 自动检测逻辑

系统通过以下规则自动识别网络类型：

1. SubNetwork名称包含 "HSR" 或 "Ring" → **HSR**
2. 2个SubNetwork + IED同时出现在两个网络 → **PRP**
3. VLAN标记 + 单SubNetwork → **HSR**
4. 默认: 2个SubNetwork → **PRP**, 1个SubNetwork → **HSR**

---

## 常见问题

**Q: 页面显示 "PRP (1 nodes)"？**  
A: SCD文件解析失败，检查文件路径和XML格式

**Q: 拓扑图没有双网连线？**  
A: 检查SCD中IED是否同时出现在两个SubNetwork

**Q: HSR显示为PRP？**  
A: SubNetwork名称需包含 "HSR" 或 "Ring"，或只有一个SubNetwork

**Q: 节点IP显示 192.168.1.x 而非SCD配置？**  
A: 后端未正确加载SCD，检查日志输出
