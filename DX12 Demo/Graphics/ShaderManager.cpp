#include "pch.h"
#include "ShaderManager.h"

static ShaderManager s_Instance;

D3D12_SHADER_BYTECODE ShaderManager::Shader::GetBytecode() const
{
    return
    {
        Bytecode.data(),
        Bytecode.size()
    };
}

ShaderManager* ShaderManager::GetInstance()
{
    return &s_Instance;
}

bool ShaderManager::LoadCSO(const std::string& name, const std::string& path)
{
    auto instance = ShaderManager::GetInstance();
    auto shader = std::make_unique<Shader>();
    vector<u8> bytes = ReadFileBinary(path);
    if (bytes.size() == 0)
    {
        DBG_LOG(sfmt("Failed to load CSO %s.", path.c_str()));
        return false;
    }

    shader->Bytecode = std::move(bytes);
    instance->m_shaders[name] = std::move(shader);
    return true;
}

ShaderManager::Shader* ShaderManager::GetShader(const std::string& name)
{
    auto instance = ShaderManager::GetInstance();
    auto it = instance->m_shaders.find(name);
    if (it == instance->m_shaders.end())
    {
        return nullptr;
    }
    return it->second.get();
}
