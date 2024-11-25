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
  // namespace省略
  template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

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
  /// デバイスの初期化
  /// </summary>
  void DeviceInitilaze();

  /// <summary>
  /// コマンド関連の初期化
  /// </summary>
  void CommandInitilize();
/// <summary>
/// 深度バッファの生成
/// </summary>
  void CreateDepthBuffer();

  /// <summary>
  /// 各種DescriptorHeapの生成
  /// </summary>
  void CreateDescriptorHeap();

  /// <summary>
  /// レンダーターゲットの初期化
  /// </summary>
  void RenderTerggetInitialize();
  /// <summary>
  /// スワップチェーンの生成
  /// </summary>
  void CreateSwapChain();

  /// <summary>
  /// 深度ステンシルビューの初期化
  /// </summary>
  void DSVInitialize();

  /// <summary>
  /// フェンスの生成
  /// </summary>
  void FenceInitialize();

  /// <summary>
  /// ビューポート矩形の初期化
  /// </summary>
  void ViewportRectInitialize();

  /// <summary>
  /// シザリング矩形の初期化
  /// </summary>
  void ScissorRect();

  /// <summary>
  /// DXCコンパイラの生成
  /// </summary>
  void DXCCompiler();

  /// <summary>
  /// ImGuiの初期化
  /// </summary>
  void ImGuiInitilize();

  /// <summary>
/// 指定番号のCPUデスクリプタハンドルを取得する
/// </summary>
  static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
  /// <summary>
/// 指定番号のGPUデスクリプタハンドルを取得する
/// </summary>
  static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);


  // RTV用のヒープでディスクリプタの数は2。RTVはshader内で触るものではないので、ShaderVisibleはfalse
  ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
  // SRV用のヒープでディスクリプタの数は128.RTVはshader内で触るものなので、ShaderVisibleはtrue
  ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
  // DSV用のヒープでディスクリプタの数は1。DSVはshader内で触るものではないので、ShaderVisibleはfalse
  ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);


  // swapChain
  ComPtr <IDXGISwapChain4> swapChain = nullptr;
  //SwapChain(スワップチェーン)を生成する
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

  // スワップチェーンリソース
  std::array<ComPtr<ID3D12Resource>, 2> &swapChainResources;

  //RTVの設定
  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

  //デバックレイヤー
  ComPtr <ID3D12Debug1> debugController = nullptr;
  //仕様するアダプター用の変数。最初にnullptrを入れておく
  ComPtr <IDXGIAdapter4> useAdapter = nullptr;

  //コマンドアロケーターを生成する
  ComPtr <ID3D12CommandAllocator> commandAllocator = nullptr;
  //コマンドリストを生成する
  ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
  //コマンドキューを生成する
  ComPtr <ID3D12CommandQueue> commandQueue = nullptr;
  D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

  // WindowsAPI
  WinApp* winApp = nullptr;

  //IDXGIのファクトリーの生成
  ComPtr <IDXGIFactory7>  dxgiFactory = nullptr;
  //実際に頂点リソースを作る
  ComPtr<ID3D12Resource> resource = nullptr;
  ComPtr <ID3D12Device> device = nullptr;

  /// <summary>
  /// DescriptorHeapの生成
  /// </summary>
  /// <param name="heapType"></param>
  /// <param name="numDescriptors"></param>
  /// <param name="shaderVisible"></param>
  /// <returns></returns>
  ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

  static uint32_t descriptorsizeSRV;
  static uint32_t descriptorsizeRTV;
  static uint32_t descriptorsizeDSV;

  //フェンス
  ComPtr <ID3D12Fence> fence = nullptr;

  // ビューポート
  D3D12_VIEWPORT viewport{};
  // シザー短形
  D3D12_RECT scissorRect{};

  //DXCユーティリティ
  ComPtr <IDxcUtils> dxcUtils = nullptr;
  //DXCコンパイラの生成
  ComPtr <IDxcCompiler3> dxcCompiler = nullptr;
  ComPtr <IDxcIncludeHandler> includeHandler = nullptr;

};

