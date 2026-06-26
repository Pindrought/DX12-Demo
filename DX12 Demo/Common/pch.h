#pragma once
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include <algorithm>
#include <cassert>
#include <chrono>

#include "directx/d3d12.h"
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "directx/d3dx12.h"
#include "D3D12MemAlloc.h"
#include <wrl.h>
#include <queue>
#include <fstream>
#include <unordered_map>
#include "WeakWrapper.h"

#define CONSOLE_LOG_ENABLED
#define NUMBER_FRAMES_IN_FLIGHT 3

using namespace Microsoft::WRL;
using std::queue;
using std::shared_ptr;
using std::unordered_map;
using std::unique_ptr;
using std::vector;
using std::weak_ptr;


inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

#define SAFE_RELEASE(p) if (p) (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

#ifdef _DEBUG
    #define DBG_LOG(msg) std::cout << msg << std::endl;
#else
    #ifdef CONSOLE_LOG_ENABLED
        #define DBG_LOG(msg) std::cout << msg << std::endl;
    #else
        #define DBG_LOG(msg) //No-op in release builds
    #endif
#endif

//Fixed-width primitive type aliases using bit-width suffixes
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef std::atomic_uint64_t atomic_u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

class Engine;
class Graphics;
class Window;

#include "sfmt.h" //String formatting helper function
#include "Timer.h"

static std::vector<u8> ReadFileBinary(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file)
        throw std::runtime_error("Failed to open file: " + path);

    size_t size = file.tellg();
    std::vector<u8> data(size);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), size);

    return data;
}