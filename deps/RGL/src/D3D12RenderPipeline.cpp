#if RGL_DX12_AVAILABLE
#include "D3D12RenderPipeline.hpp"
#include "D3D12Device.hpp"
#include "D3D12ShaderLibrary.hpp"
#include "D3D12Sampler.hpp"
#include <list>

using namespace Microsoft::WRL;

namespace RGL {
    D3D12_PRIMITIVE_TOPOLOGY rgl2d3dtopology(RGL::PrimitiveTopology mode) {
        switch (mode) {
        case decltype(mode)::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case decltype(mode)::LineListAdjacency : return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
        case decltype(mode)::LineStrip : return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case decltype(mode)::LineStripAdjacency : return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
        case decltype(mode)::PatchList : return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
        case decltype(mode)::PointList : return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case decltype(mode)::TriangleFan : return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case decltype(mode)::TriangleList : return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case decltype(mode)::TriangleListAdjacency  : return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
        case decltype(mode)::TriangleStrip  : return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case decltype(mode)::TriangleStripAdjacency  : return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
        default:
            FatalError("Unsupported topology mode");
        }
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE rgl2d3dtopology_family(RGL::PrimitiveTopology mode) {
        switch (mode) {
        case decltype(mode)::LineList:
        case decltype(mode)::LineListAdjacency: 
        case decltype(mode)::LineStrip:
        case decltype(mode)::LineStripAdjacency:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case decltype(mode)::PatchList: 
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
        case decltype(mode)::PointList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case decltype(mode)::TriangleFan: 
        case decltype(mode)::TriangleList: 
        case decltype(mode)::TriangleListAdjacency: 
        case decltype(mode)::TriangleStrip: 
        case decltype(mode)::TriangleStripAdjacency:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        default:
            FatalError("Unsupported topology mode");
        }
    }

    DXGI_FORMAT rgl2dxgiformat(RGL::VertexAttributeFormat format) {
        switch (format) {
        case decltype(format)::R32G32B32_SignedFloat:   return DXGI_FORMAT_R32G32B32_FLOAT;
        case decltype(format)::R32G32_SignedFloat:      return DXGI_FORMAT_R32G32_FLOAT;
        case decltype(format)::R32_Uint: return DXGI_FORMAT_R32_UINT;
        case decltype(format)::R32G32B32A32_SignedFloat: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default:
            FatalError("Unsupported vertex attribute format");
        }
    }

    D3D12_CULL_MODE rgl2d3d12cull(RGL::CullMode cull) {
        switch (cull) {
        case decltype(cull)::None:
            return D3D12_CULL_MODE_NONE;
        case decltype(cull)::Front:
            return D3D12_CULL_MODE_FRONT;
        case decltype(cull)::Back:
            return D3D12_CULL_MODE_BACK;
        default:
            return D3D12_CULL_MODE_NONE;    //TODO: this should be the All option which results in no rendering
        }
    };

    D3D12_COMPARISON_FUNC rgl2d3dcompfn(decltype(RenderPipelineDescriptor::DepthStencilConfig::depthFunction) depthFunction) {
        switch (depthFunction) {
        case decltype(depthFunction)::None: return D3D12_COMPARISON_FUNC(0); //D3D12_COMPARISON_FUNC_NONE:
        case decltype(depthFunction)::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
        case decltype(depthFunction)::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
        case decltype(depthFunction)::Greater: return D3D12_COMPARISON_FUNC_GREATER;
        case decltype(depthFunction)::GreaterOrEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case decltype(depthFunction)::Less: return D3D12_COMPARISON_FUNC_LESS;
        case decltype(depthFunction)::LessOrEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case decltype(depthFunction)::Never: return D3D12_COMPARISON_FUNC_NEVER;
        case decltype(depthFunction)::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        default:
            FatalError("Compare function not supported");

        }
    }

    D3D12_FILL_MODE rgl2d3d12FillMode(RGL::PolygonOverride mode) {
        switch (mode) {
        case decltype(mode)::Fill:
            return D3D12_FILL_MODE_SOLID;
        case decltype(mode)::Line:
            return D3D12_FILL_MODE_WIREFRAME;
        default:
            FatalError("Unsupported fill mode");
        }
    }

    D3D12_BLEND rgl2d3d12blendfactor(RGL::BlendFactor op) {
        switch (op) {
        case decltype(op)::Zero: return D3D12_BLEND_ZERO;
        case decltype(op)::One: return D3D12_BLEND_ONE;
        case decltype(op)::SourceColor: return D3D12_BLEND_SRC_COLOR;
        case decltype(op)::OneMinusSourceColor: return D3D12_BLEND_INV_SRC_COLOR;
        case decltype(op)::SourceAlpha: return D3D12_BLEND_SRC_ALPHA;
        case decltype(op)::OneMinusSourceAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
        case decltype(op)::DestAlpha: return D3D12_BLEND_DEST_ALPHA;
        case decltype(op)::OneMinusDestAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
        case decltype(op)::DestColor: return D3D12_BLEND_DEST_COLOR;
        case decltype(op)::OneMinusDestColor: return D3D12_BLEND_INV_DEST_COLOR;
        case decltype(op)::SourceAlphaSaturate: return D3D12_BLEND_SRC_ALPHA_SAT;
        case decltype(op)::Source1Color: return D3D12_BLEND_SRC1_COLOR;
        case decltype(op)::OneMinusSource1Color: return D3D12_BLEND_INV_SRC1_COLOR;
        case decltype(op)::Source1Alpha: return D3D12_BLEND_SRC1_ALPHA;
        case decltype(op)::OneMinusSource1Alpha: return D3D12_BLEND_INV_SRC1_ALPHA;
        default:
            FatalError("Unsupported blend factor");
        }
    }
    D3D12_LOGIC_OP rgl2d3d12logicop(RGL::RenderPipelineDescriptor::ColorBlendConfig::LogicalOperation op) {
        switch (op) {
        case decltype(op)::Clear: return D3D12_LOGIC_OP_CLEAR;
        case decltype(op)::Set: return D3D12_LOGIC_OP_SET;
        case decltype(op)::Copy: return D3D12_LOGIC_OP_COPY;
        case decltype(op)::CopyInverted: return D3D12_LOGIC_OP_COPY_INVERTED;
        case decltype(op)::Noop: return D3D12_LOGIC_OP_NOOP;
        case decltype(op)::Invert: return D3D12_LOGIC_OP_INVERT;
        case decltype(op)::AND: return D3D12_LOGIC_OP_AND;
        case decltype(op)::NAND: return D3D12_LOGIC_OP_NAND;
        case decltype(op)::OR: return D3D12_LOGIC_OP_OR;
        case decltype(op)::NOR: return D3D12_LOGIC_OP_NOR;
        case decltype(op)::XOR: return D3D12_LOGIC_OP_XOR;
        case decltype(op)::Equivalent: return D3D12_LOGIC_OP_EQUIV;
        case decltype(op)::AND_Reverse: return D3D12_LOGIC_OP_AND_REVERSE;
        case decltype(op)::ANDInverted: return D3D12_LOGIC_OP_AND_INVERTED;
        case decltype(op)::ORReverse: return D3D12_LOGIC_OP_OR_REVERSE;
        case decltype(op)::ORInverted: return D3D12_LOGIC_OP_OR_INVERTED;
        }
    }

    D3D12_BLEND_OP rgl2d3d12blendop(RGL::BlendOperation op) {
        switch (op) {
        case decltype(op)::Add: return D3D12_BLEND_OP_ADD;
        case decltype(op)::Max : return D3D12_BLEND_OP_MAX;
        case decltype(op)::Min : return D3D12_BLEND_OP_MIN;
        case decltype(op)::ReverseSubtract : return D3D12_BLEND_OP_REV_SUBTRACT;
        case decltype(op)::Subtract : return D3D12_BLEND_OP_SUBTRACT;
        }
    }



	PipelineLayoutD3D12::PipelineLayoutD3D12(decltype(owningDevice) owningDevice, const PipelineLayoutDescriptor& desc) : owningDevice(owningDevice), config(desc)
	{
        auto device = owningDevice->device;

        // Create a root signature.
        // version 1.1 is preferred, but we fall back to 1.0 if it is not available
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        // Allow input layout and deny unnecessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        // create the constants data
        const auto nconstants = desc.constants.size();

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
        std::list< D3D12_DESCRIPTOR_RANGE1> ranges; // stable memory location is necessary
        for (const auto& constant : desc.constants) {
            rootParameters.emplace_back().InitAsConstants(constant.size_bytes / sizeof(int), constant.n_register, 0, D3D12_SHADER_VISIBILITY_ALL);
        }

        // create combined image sampler data
        {
            for (const auto& item : desc.bindings) {
                switch (item.type) {
                case decltype(item.type)::Sampler:
                    {
                        auto& range = ranges.emplace_back(D3D12_DESCRIPTOR_RANGE1{
                            .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
                            .NumDescriptors = item.count,
                            .BaseShaderRegister = item.binding,
                            .RegisterSpace = 0,
                            .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
                            });
                        samplerBindingtoRootSlot[item.binding] = rootParameters.size();
                        rootParameters.emplace_back().InitAsDescriptorTable(1, &range);
                    }
                    break;
                case decltype(item.type)::SampledImage:
                
                    // SRV
                    {
                        auto& range = ranges.emplace_back(D3D12_DESCRIPTOR_RANGE1{
                            .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                            .NumDescriptors = item.count,
                            .BaseShaderRegister = item.isBindless ? 0 : item.binding,
                            .RegisterSpace = item.isBindless ? item.binding : 0u,
                            .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,
                            .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
                            });
                        textureBindingToRootSlot[{item.binding, item.isBindless ? item.binding : 0u}] = { uint32_t(rootParameters.size()), false };
                        rootParameters.emplace_back().InitAsDescriptorTable(1, &range);
                    }
                break;
                case decltype(item.type)::StorageImage:
                {
                    // UAV
                    auto& range = ranges.emplace_back(D3D12_DESCRIPTOR_RANGE1{
                        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                        .NumDescriptors = item.count,
                        .BaseShaderRegister = item.binding,
                        .RegisterSpace = 0,
                        .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,
                        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
                    });
                    textureBindingToRootSlot[item.binding] = { uint32_t(rootParameters.size()), true };
                    rootParameters.emplace_back().InitAsDescriptorTable(1, &range);
                }
                break;
                }
            }
        }

        // constant / uniform buffer bindings (SRVs)
        for (const auto& item : desc.bindings) {
            switch (item.type) {
            case decltype(item.type)::StorageBuffer:
            case decltype(item.type)::UniformBuffer:
                if (item.writable){
                    bufferBindingToRootSlot[item.binding] = { static_cast<uint32_t>(rootParameters.size()), true };
                    rootParameters.emplace_back().InitAsUnorderedAccessView(item.binding, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE);
                }
                else {
                    bufferBindingToRootSlot[item.binding] = { static_cast<uint32_t>(rootParameters.size()), false };
                    rootParameters.emplace_back().InitAsShaderResourceView(item.binding, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE);
                }
                break;

            }
        }

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1(rootParameters.size(), rootParameters.data(), 0, nullptr, rootSignatureFlags);

        // Serialize the root signature.
        // it becomes a binary object which can be used to create the actual root signature
        ComPtr<ID3DBlob> rootSignatureBlob;
        ComPtr<ID3DBlob> errorBlob;
        try {
            DX_CHECK(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
        }
        catch (std::exception& e) {
            FatalError(std::format("{}", (char*)errorBlob->GetBufferPointer()));
        }
        // Create the root signature.
            DX_CHECK(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
	}


    RenderPipelineD3D12::RenderPipelineD3D12(decltype(owningDevice) owningDevice, const RenderPipelineDescriptor& desc) : owningDevice(owningDevice), pipelineLayout(std::static_pointer_cast<PipelineLayoutD3D12>(desc.pipelineLayout)), overrideMode(rgl2d3dtopology(desc.inputAssembly.topology))
    {
        auto device = owningDevice->device;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
        
        const auto nattributes = desc.vertexConfig.attributeDescs.size();

        std::unordered_map<uint32_t, D3D12_INPUT_CLASSIFICATION> bindingToType;
        for (const auto& binding : desc.vertexConfig.vertexBindings) {
            bindingToType[binding.binding] = binding.inputRate == RGL::InputRate::Vertex ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        }

        // create the vertex attribute layout
        stackarray(inputLayout, D3D12_INPUT_ELEMENT_DESC, nattributes);
        for (UINT i = 0; i < nattributes; i++) {
            auto& attr = desc.vertexConfig.attributeDescs[i];
            auto type = bindingToType.at(attr.binding);
            inputLayout[i] = { 
                .SemanticName = "TEXCOORD", 
                .SemanticIndex = attr.location, 
                .Format = rgl2dxgiformat(attr.format), 
                .InputSlot = attr.binding, 
                .AlignedByteOffset =  D3D12_APPEND_ALIGNED_ELEMENT, 
                .InputSlotClass = type ,
                .InstanceDataStepRate = (type == D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA ? 1u : 0u) };
        }

        // setup the shaders
        std::shared_ptr<ShaderLibraryD3D12> vertFunc, fragFunc;
        for (const auto& func : desc.stages) {
            switch (func.type) {
            case RGL::ShaderStageDesc::Type::Vertex:
                vertFunc = std::static_pointer_cast<decltype(vertFunc)::element_type>(func.shaderModule);
                vsBufferBindings = vertFunc->bufferBindingStore;
                break;
            case RGL::ShaderStageDesc::Type::Fragment:
                fragFunc = std::static_pointer_cast<decltype(vertFunc)::element_type>(func.shaderModule);
                fsBufferBindings = fragFunc->bufferBindingStore;
                break;
            default:
                FatalError("Function type is not supported");
            }
        }

        // set the render target texture formats
        const auto nattachments = desc.colorBlendConfig.attachments.size();

        pipelineStateDesc.NumRenderTargets = static_cast<UINT>(nattachments);

        D3D12_BLEND_DESC colorBlendingConfig{
            .AlphaToCoverageEnable = false,
            .IndependentBlendEnable = true,
        };

        Assert(nattachments < std::size(pipelineStateDesc.RTVFormats), "Too many attachments!");
        for (int i = 0; i < nattachments; i++) {
            auto& attachment = desc.colorBlendConfig.attachments[i];
            pipelineStateDesc.RTVFormats[i] = rgl2dxgiformat_texture(attachment.format);

            colorBlendingConfig.RenderTarget[i] = {
                .BlendEnable = attachment.blendEnabled,
                .LogicOpEnable = desc.colorBlendConfig.logicalOpEnabled,
                .SrcBlend = rgl2d3d12blendfactor(attachment.sourceColorBlendFactor),
                .DestBlend = rgl2d3d12blendfactor(attachment.destinationColorBlendFactor),
                .BlendOp = rgl2d3d12blendop(attachment.colorBlendOperation),
                .SrcBlendAlpha = rgl2d3d12blendfactor(attachment.sourceAlphaBlendFactor),
                .DestBlendAlpha = rgl2d3d12blendfactor(attachment.destinationAlphaBlendFactor),
                .BlendOpAlpha = rgl2d3d12blendop(attachment.alphaBlendOperation),
                .LogicOp = rgl2d3d12logicop(desc.colorBlendConfig.logicalOperation),
                .RenderTargetWriteMask = static_cast<UINT8>(attachment.colorWriteMask)
            };
        }

        CD3DX12_RASTERIZER_DESC rasterizerDesc{ D3D12_DEFAULT };
        rasterizerDesc.CullMode = rgl2d3d12cull(desc.rasterizerConfig.cullMode);
        rasterizerDesc.FrontCounterClockwise = desc.rasterizerConfig.windingOrder == decltype(desc.rasterizerConfig.windingOrder)::Counterclockwise;
        rasterizerDesc.FillMode = rgl2d3d12FillMode(desc.rasterizerConfig.polygonOverride);

        CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc{ D3D12_DEFAULT };
        depthStencilDesc.DepthEnable = desc.depthStencilConfig.depthTestEnabled;
        depthStencilDesc.DepthFunc = rgl2d3dcompfn(desc.depthStencilConfig.depthFunction);
        depthStencilDesc.DepthWriteMask = desc.depthStencilConfig.depthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

        //TODO: complete stencil
        depthStencilDesc.StencilEnable = desc.depthStencilConfig.stencilTestEnabled;

        // describe the pipeline state object
        pipelineStateDesc.pRootSignature = std::static_pointer_cast<PipelineLayoutD3D12>(desc.pipelineLayout)->rootSignature.Get();
        pipelineStateDesc.InputLayout = { inputLayout, static_cast<uint32_t>(nattributes) };
        pipelineStateDesc.PrimitiveTopologyType = rgl2d3dtopology_family(desc.inputAssembly.topology);
        pipelineStateDesc.VS = vertFunc->shaderBytecode;
        if (fragFunc) {
            pipelineStateDesc.PS = fragFunc->shaderBytecode;
        }
        pipelineStateDesc.DSVFormat = rgl2dxgiformat_texture(desc.depthStencilConfig.depthFormat);
        pipelineStateDesc.RasterizerState = rasterizerDesc;
        pipelineStateDesc.BlendState = colorBlendingConfig;
        pipelineStateDesc.DepthStencilState = depthStencilDesc;
        pipelineStateDesc.SampleMask = UINT_MAX;
        pipelineStateDesc.SampleDesc.Count = 1;

        // create the PSO
        DX_CHECK(device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pipelineState)));
    }
}
#endif
