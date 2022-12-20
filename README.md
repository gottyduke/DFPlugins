Plugin Extender for "Dwarf Fortress" steam distribute ver.
---
适用于 "Dwarf Fortress" (简译名: "矮人要塞") steam发行版 的插件框架. 项目继承+改进自[SKSE](skse.silverlock.org)与[CommonLibSSE](https://github.com/Ryan-rsm-McKenzie/CommonLibSSE)

---
## 施工中!

- 加载器 (loader) &#10003;
- 框架 (dfpe) &#10003;
- 翻译拓展 (translation extender) <-

## 编译

使用主`dfpe`项目的`!rebuild`脚本生成解决方案.  

### 需求
+ [CMake](https://cmake.org)
+ [vcpkg](https://github.com/microsoft/vcpkg/releases)
+ `/std:c++23` 或 `/std:latest`
+ 环境变量  
   + `VCPKG_ROOT`指向本地vcpkg安装目录(`PATH`).  
   + `DFPath`指向本地`"Dwarf Fortress"`安装目录.  


```
git clone https://github.com/gottyduke/DFPlugins
cd DFPlugins
powershell -ExecutionPolicy Bypass -Command "& %~dp0/!rebuild.ps1"
```
