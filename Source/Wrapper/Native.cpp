/*
 * ShaderConductor
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "Native.h"
#include <ShaderConductor/ShaderConductor.hpp>
#include <iostream>

using namespace ShaderConductor;

void Compile(SourceDescription* source, OptionsDescription* optionsDesc, TargetDescription* target, ResultDescription* result)
{
    Compiler::SourceDesc sourceDesc;
    sourceDesc.entryPoint = source->entryPoint;
    sourceDesc.source = source->source;
    sourceDesc.stage = source->stage;
    sourceDesc.fileName = nullptr;
    sourceDesc.defines = nullptr;
    sourceDesc.numDefines = 0;

    Compiler::Options options;
    options.packMatricesInRowMajor = optionsDesc->packMatricesInRowMajor;
    options.enable16bitTypes = optionsDesc->enable16bitTypes;
    options.enableDebugInfo = optionsDesc->enableDebugInfo;
    options.disableOptimizations = optionsDesc->disableOptimizations;
    options.optimizationLevel = optionsDesc->optimizationLevel;
    options.shaderModel = {static_cast<uint8_t>(optionsDesc->shaderModel.major), static_cast<uint8_t>(optionsDesc->shaderModel.minor)};
    options.shiftAllTexturesBindings = optionsDesc->shiftAllTexturesBindings;
    options.shiftAllSamplersBindings = optionsDesc->shiftAllSamplersBindings;
    options.shiftAllCBuffersBindings = optionsDesc->shiftAllCBuffersBindings;
    options.shiftAllUABuffersBindings = optionsDesc->shiftAllUABuffersBindings;

    Compiler::TargetDesc targetDesc{};
    targetDesc.language = target->shadingLanguage;
    targetDesc.version = target->version;

    try
    {
        const auto translation = Compiler::Compile(sourceDesc, options, targetDesc);

        if (translation.errorWarningMsg != nullptr)
        {
            result->errorWarningMsg = reinterpret_cast<ShaderConductorBlob*>(translation.errorWarningMsg);
        }
        if (translation.target != nullptr)
        {
            result->target = reinterpret_cast<ShaderConductorBlob*>(translation.target);
        }

        result->hasError = translation.hasError;
        result->isText = translation.isText;      
        result->reflection = translation.reflection;
    }
    catch (std::exception& ex)
    {
        const char* exception = ex.what();
        result->errorWarningMsg = CreateShaderConductorBlob(exception, static_cast<uint32_t>(strlen(exception)));
        result->hasError = true;
    }
}

void Disassemble(DisassembleDescription* source, ResultDescription* result)
{
    Compiler::DisassembleDesc disassembleSource;
    disassembleSource.language = source->language;
    disassembleSource.binary = reinterpret_cast<uint8_t*>(source->binary);
    disassembleSource.binarySize = source->binarySize;

    const auto disassembleResult = Compiler::Disassemble(disassembleSource);

    if (disassembleResult.errorWarningMsg != nullptr)
    {
        result->errorWarningMsg = reinterpret_cast<ShaderConductorBlob*>(disassembleResult.errorWarningMsg);
    }
    if (disassembleResult.target != nullptr)
    {
        result->target = reinterpret_cast<ShaderConductorBlob*>(disassembleResult.target);
    }

    result->hasError = disassembleResult.hasError;
    result->isText = disassembleResult.isText;
}

ShaderConductorBlob* CreateShaderConductorBlob(const void* data, int size)
{
    return reinterpret_cast<ShaderConductorBlob*>(ShaderConductor::CreateBlob(data, size));
}

void DestroyShaderConductorBlob(ShaderConductorBlob* blob)
{
    DestroyBlob(reinterpret_cast<Blob*>(blob));
}

const void* GetShaderConductorBlobData(ShaderConductorBlob* blob)
{
    return reinterpret_cast<Blob*>(blob)->Data();
}

int GetShaderConductorBlobSize(ShaderConductorBlob* blob)
{
    return reinterpret_cast<Blob*>(blob)->Size();
}

Compiler::ReflectionDesc* GetReflection(ResultDescription* result)
{
    return result == nullptr ? nullptr : (Compiler::ReflectionDesc*)result->reflection;
}

int GetStageInputCount(ResultDescription* result)
{
    return GetReflection(result) == nullptr ? 0 : (int)GetReflection(result)->stageInputs.size();
}

int GetUniformBufferCount(ResultDescription* result)
{
    return GetReflection(result) == nullptr ? 0 : (int)GetReflection(result)->uniformBuffers.size();
}

int GetSamplerCount(ResultDescription* result)
{
    return GetReflection(result) == nullptr ? 0 : (int)GetReflection(result)->samplers.size();
}

void GetStageInput(ResultDescription* result, int stageInputIndex, char* name, int maxNameLength, int* location)
{
    Compiler::StageInput si = GetReflection(result)->stageInputs[stageInputIndex];
    strcpy_s(name, maxNameLength, si.name.c_str());
    *location = si.location;
}

void GetUniformBuffer(ResultDescription* result, int bufferIndex, char* blockName, char* instanceName, int maxNameLength, int* byteSize,
                      int* parameterCount)
{
    Compiler::UniformBuffer ub = GetReflection(result)->uniformBuffers[bufferIndex];
    strcpy_s(blockName, maxNameLength, ub.blockName.c_str());
    strcpy_s(instanceName, maxNameLength, ub.instanceName.c_str());
    *byteSize = ub.byteSize;
    *parameterCount = (int)ub.parameters.size();
}

void GetParameter(ResultDescription* result, int bufferIndex, int parameterIndex, char* name, int maxNameLength, int* type, int* rows, int* columns, int* byteOffset, int* arrayDimensions)
{
    Compiler::Parameter p = GetReflection(result)->uniformBuffers[bufferIndex].parameters[parameterIndex];
    strcpy_s(name, maxNameLength, p.name.c_str());
    *type = p.type;
    *rows = p.rows;
    *columns = p.columns;
    *byteOffset = p.byteOffset;
    *arrayDimensions = p.arrayDimensions;
}

void GetParameterArraySize(ResultDescription* result, int bufferIndex, int parameterIndex, int dimension, int* arraySize)
{
    Compiler::Parameter p = GetReflection(result)->uniformBuffers[bufferIndex].parameters[parameterIndex];
    *arraySize = p.arraySize[dimension];
}

void GetSampler(ResultDescription* result, int samplerIndex, char* name, char* originalName, char* textureName, int maxNameLength, int* type, int* slot, int* textureSlot)
{
    Compiler::Sampler s = GetReflection(result)->samplers[samplerIndex];
    strcpy_s(name, maxNameLength, s.name.c_str());
    strcpy_s(originalName, maxNameLength, s.originalName.c_str());
    strcpy_s(textureName, maxNameLength, s.textureName.c_str());
    *type = s.type;
    *slot = s.slot;
    *textureSlot = s.textureSlot;
}