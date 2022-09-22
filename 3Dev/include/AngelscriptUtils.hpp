#pragma once
#include "Utils.hpp"

static void MessageCallback(const asSMessageInfo *msg, void *param)
{
    switch(msg->type)
    {
    case asMSGTYPE_ERROR:
        Log::Write(std::string(msg->section) + " (" + std::to_string(msg->row) + ", "
                    + std::to_string(msg->col) + "): " + std::string(msg->message), Log::Type::Error);
        break;
    case asMSGTYPE_WARNING:
        Log::Write(std::string(msg->section) + " (" + std::to_string(msg->row) + ", "
                    + std::to_string(msg->col) + "): " + std::string(msg->message), Log::Type::Warning);
        break;
    case asMSGTYPE_INFORMATION:
        Log::Write(std::string(msg->section) + " (" + std::to_string(msg->row) + ", "
                    + std::to_string(msg->col) + "): " + std::string(msg->message), Log::Type::Info);
        break;
    }
}

template<class T>
static void MakeType(T* memory) { new(memory) T(); }

template<class T>
static void CopyType(const T& other, T* memory) { new(memory) T(other); }

template<class T>
static void DestroyType(T* memory) { memory->~T(); }

template<class T>
static T& AssignType(T* dst, T* src) { return *dst = *src; }

static void MakeVector3(float x, float y, float z, rp3d::Vector3* vec) { new(vec) rp3d::Vector3(x, y, z); }
