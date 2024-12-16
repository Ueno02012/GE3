#pragma once
#include <wrl.h>
#include"DirectXCommon.h"
// スプライト共通部
class SpriteCommon
{
 public: // メンバ変数
   template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
   HRESULT hr;

   // 初期化
   void Initialize(DirectXCommon* dxCommon);;

   DirectXCommon* GetDxCommon() const { return dxCommon_; }

   void DrawCommonSeting();

private:
  // ルートシグネイチャの作成
  void CreateRootSignature();

  //グラフィックスパイプラインの生成
  void CreateGraphicsPipeline();


  DirectXCommon* dxCommon_;

  D3D12_ROOT_PARAMETER rootParameters[4] = {};
  D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
  D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
  D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
  //======= BlendStateの設定 =========//
  D3D12_BLEND_DESC blendDesc{};
  //===== RasterizerStateの設定を行う ======//   
  D3D12_RASTERIZER_DESC rasterizerDesc{};

  ComPtr <ID3D12RootSignature> rootSignature = nullptr;
  ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
  D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

  ComPtr<ID3DBlob> signatureBlob = nullptr;
  ComPtr<ID3DBlob> errorBlob = nullptr;
};

