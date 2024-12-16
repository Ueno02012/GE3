#include "SpriteCommon.h"
#include <d3d12.h>
#include <cassert>

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
  //引数で受け取ってメンバ変数に記録
  dxCommon_ = dxCommon;

  CreateGraphicsPipeline();
}

void SpriteCommon::CreateRootSignature()
{
  //DescriptorRange作成
  descriptorRange[0].BaseShaderRegister = 0;
  descriptorRange[0].NumDescriptors = 1;
  descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

  //RootParameter作成
  rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
  rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
  rootParameters[0].Descriptor.ShaderRegister = 0;// レジスタ番号0を使う

  rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
  rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;// VertexShaderでを使う
  rootParameters[1].Descriptor.ShaderRegister = 0;// レジスタ番号0を使う

  rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
  rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
  rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
  rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//利用する数

  rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
  rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
  rootParameters[3].Descriptor.ShaderRegister = 1;// レジスタ番号1を使う


  //RootSignature作成
  D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
  descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
  descriptionRootSignature.pParameters = rootParameters;// ルートパラメータ配列へのポインタ
  descriptionRootSignature.NumParameters = _countof(rootParameters);// 配列の長さ

  //Samplerの設定
  D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
  staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ	
  staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0～1の範囲外をリピート		
  staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない		
  staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う	
  staticSamplers[0].ShaderRegister = 0;//レジスタ番号０を使う
  staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
  descriptionRootSignature.pStaticSamplers = staticSamplers;
  descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


  hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

  hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
  assert(SUCCEEDED(hr));
}

void SpriteCommon::CreateGraphicsPipeline()
{
  CreateRootSignature();

  // DepthStencilStateの設定
  // Depthの機能を有効化する
  depthStencilDesc.DepthEnable = true;
  // 書き込みする
  depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
  // 比較関数はLessEqual。つまり、近ければ描画される
  depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


  //======== InputLayout設定 ==========//
  D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
  inputElementDescs[0].SemanticName = "POSITION";
  inputElementDescs[0].SemanticIndex = 0;
  inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

  inputElementDescs[1].SemanticName = "TEXCOORD";
  inputElementDescs[1].SemanticIndex = 0;
  inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

  inputElementDescs[2].SemanticName = "NORMAL";
  inputElementDescs[2].SemanticIndex = 0;
  inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

  inputLayoutDesc.pInputElementDescs = inputElementDescs;
  inputLayoutDesc.NumElements = _countof(inputElementDescs);

  //全ての色要素を書き込む
  blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

  //裏面(時計回り)を表示しない
  rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
  //三角形の中を塗りつぶす
  rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

  //======== ShaderをCompile ===========// 
  ComPtr <IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
  assert(vertexShaderBlob != nullptr);
  ComPtr <IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
  assert(pixelShaderBlob != nullptr);

  graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();   // RootSignature
  graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;    // InputLayout
  graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };  // VertexShader
  graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };    // PixelShader
  graphicsPipelineStateDesc.BlendState = blendDesc;// BlendState
  graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; //RasterizerState
  // 書き込むRTVの情報
  graphicsPipelineStateDesc.NumRenderTargets = 1;
  graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  //利用するトポロジ(形状)のタイプ
  graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  // どのように画面に色を打ち込むかの設定(気にしなくて良い)
  graphicsPipelineStateDesc.SampleDesc.Count = 1;
  graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
  // DepthStencilの設定
  graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
  graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

  hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
  assert(SUCCEEDED(hr));
}

void SpriteCommon::DrawCommonSeting()
{
  dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
  dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
  dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}
