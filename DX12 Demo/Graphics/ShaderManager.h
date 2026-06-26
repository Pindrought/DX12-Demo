#pragma once
#include "pch.h"

class ShaderManager
{
public:
    struct Shader
    {
        std::vector<u8> Bytecode;
        D3D12_SHADER_BYTECODE GetBytecode() const;
    };

public:
    static ShaderManager* GetInstance();
    static bool LoadCSO(const std::string& name, const std::string& path);
    static Shader* GetShader(const std::string& name);

private:
    unordered_map<std::string, unique_ptr<Shader>> m_shaders;
};