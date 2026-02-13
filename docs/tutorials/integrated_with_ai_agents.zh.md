# AI Agents集成

> 本指南旨在展示如何使用 `llms.txt` 文件，为AI模型带来智能交互能力。

---

## 先决条件

- 建议使用 **VSCode** 或 **Atom** 作为代码编辑器
- 支持上下文窗口的模型

---

## 我现在该做什么？

### 1. 将 `llm.txt` 添加到上下文窗口

[⬇️下载](https://github.com/LiteLDev/LegacyScriptEngine/raw/refs/heads/develop/llms.txt)

直接将 `llm.txt` 文件内容复制粘贴到AI模型的对话上下文中，或在支持文件上传的界面中上传该文件。

### 2. 编写插件

使用以下提示词模板，让AI为你生成插件代码：

```text
参考LegacyScriptEngine文档，使用JavaScript编写适用于LegacyScriptEngine的插件。
请帮我实现以下功能：

[请在此处描述您需要实现的功能的具体细节]
```