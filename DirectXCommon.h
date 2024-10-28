#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include <d3d12sdklayers.h>
#include<dxcapi.h>
#include<cassert>
#include"WinApp.h"
#include"Logger.h"
#include <wrl.h>
#include"StringUtility.h"
#include<format>
#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon
{
public: // メンバ関数

  /// <summary>
  /// 初期化
  /// </summary>
  /// <param name="winApp">WindowsAPI</param>
  void Initialize(WinApp* winApp);



  /// <summary>
/// SRVの指定番号のCPUデスクリプタハンドルを取得する
/// </summary>
  D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

  /// <summary>
  /// SRVの指定番号のGPUデスクリプタハンドルを取得する
  /// </summary>
  //D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

private:
  HRESULT hr;

/// <summary>
/// 深度バッファの生成
/// </summary>
  void CreateDepthBuffer();

  /// <summary>
  /// 各種DescriptorHeapの生成
  /// </summary>
  void CreateDescriptorHeap();

  void RenderTerggetInitialize();

  /// <summary>
/// 指定番号のCPUデスクリプタハンドルを取得する
/// </summary>
  static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
  /// <summary>
/// 指定番号のGPUデスクリプタハンドルを取得する
/// </summary>
  static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);


  // RTV用のヒープでディスクリプタの数は2。RTVはshader内で触るものではないので、ShaderVisibleはfalse
  Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
  // SRV用のヒープでディスクリプタの数は128.RTVはshader内で触るものなので、ShaderVisibleはtrue
  Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
  // DSV用のヒープでディスクリプタの数は1。DSVはshader内で触るものではないので、ShaderVisibleはfalse
  Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);


  // swapChain
  Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = nullptr;

  // スワップチェーンリソース
  std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

  //デバックレイヤー
  Microsoft::WRL::ComPtr <ID3D12Debug1> debugController = nullptr;
  //仕様するアダプター用の変数。最初にnullptrを入れておく
  Microsoft::WRL::ComPtr <IDXGIAdapter4> useAdapter = nullptr;

  //コマンドアロケーターを生成する
  Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator = nullptr;
  //コマンドリストを生成する
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
  //コマンドキューを生成する
  Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue = nullptr;
  D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

  // WindowsAPI
  WinApp* winApp = nullptr;

  //IDXGIのファクトリーの生成
  Microsoft::WRL::ComPtr <IDXGIFactory7>  dxgiFactory = nullptr;
  //実際に頂点リソースを作る
  Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
  Microsoft::WRL::ComPtr <ID3D12Device> device = nullptr;

  /// <summary>
  /// DescriptorHeapの生成
  /// </summary>
  /// <param name="heapType"></param>
  /// <param name="numDescriptors"></param>
  /// <param name="shaderVisible"></param>
  /// <returns></returns>
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

  static uint32_t descriptorsizeSRV;
  static uint32_t descriptorsizeRTV;
  static uint32_t descriptorsizeDSV;
  Microsoft::WRL::ComPtr <IDxcUtils> dxcUtils = nullptr;

};

