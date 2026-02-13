# AI Agents Integration

> This guide aims to demonstrate how to use the `llms.txt` file to bring intelligent interaction capabilities to AI models.

---

## Prerequisites

- It is recommended to use **VSCode** or **Atom** as the code editor
- A model that supports context windows

---

## What Should I Do Now?

### 1. Add `llms.txt` to the Context Window

[⬇️ Download](https://github.com/LiteLDev/LegacyScriptEngine/raw/refs/heads/develop/llms.txt)

Directly copy and paste the content of the `llms.txt` file into the AI model's conversation context, or upload the file in an interface that supports file uploads.

### 2. Write Plugins

Use the following prompt template to have the AI generate plugin code for you:

```text
Refer to the LegacyScriptEngine documentation and use JavaScript to write a plugin for LegacyScriptEngine.  
Please help me implement the following functionality:

[Please describe the specific details of the functionality you need to implement here]
```