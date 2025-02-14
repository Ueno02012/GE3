#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include <d3d12sdklayers.h>
#include<dxcapi.h>
#include"WinApp.h"
#include <wrl.h>
#include<string>
#include <cstdint>
#include "externals/DirectXTex/DirectXTex.h"
#include<array>
#include<chrono>

class DirectXCommon
{
public: // メンバ関数
  // namespace省略
  template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

  //DirectXCommon():swapChainResources({nullptr}) {};

  /// <summary>
  /// 初期化
  /// </summary>
  /// <param name="winApp">WindowsAPI</param>
  void Initialize(WinApp* winApp);
  /// <summary>
  /// 終了処理
  /// </summary>
  void Finalize();

  /// <summary>
  /// 描画前処理
  /// </summary>
  void PreDraw();

  /// <summary>
  /// 描画後処理
  /// </summary>
  void PostDraw();


  /// <summary>
/// 指定番号のCPUデスクリプタハンドルを取得する
/// </summary>
  static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
  /// <summary>
/// 指定番号のGPUデスクリプタハンドルを取得する
/// </summary>
  static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

  //======== ShaderをCompile ===========// 
  ComPtr<IDxcBlob> CompileShader(
    const std::wstring& filePath,
    const wchar_t* profile);

  /// <summary>
/// SRVの指定番号のCPUデスクリプタハンドルを取得する
/// </summary>
  D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

  /// <summary>
  /// SRVの指定番号のGPUデスクリプタハンドルを取得する
  /// </summary>
  D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

  ComPtr<ID3D12Resource> CreateBufferResource (size_t sizeInBytes);
  /// <summary>
  /// テクスチャーリソースの生成
  /// </summary>
  /// <param name="device"></param>
  /// <param name="metadata"></param>
  /// <returns></returns>
  ComPtr<ID3D12Resource> CreateTextureResource(
    ID3D12Device* device, const DirectX::TexMetadata& metadata);
  /// <summary>
  /// テクスチャーデータの転送
  /// </summary>
  /// <param name="texture"></param>
  /// <param name="mipImage"></param>
  void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImage);
  /// <summary>
  /// テクスチャーファイルの読み込み
  /// </summary>
  /// <param name="filePath">テクスチャーファイルのパス</param>
  /// <returns>画像イメージデータ</returns>
  static DirectX::ScratchImage LoadTexture(const std::string& filePath);

  ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ComPtr <ID3D12Device>& device, int32_t width, int32_t heigth);

  //getter
  ID3D12Device* GetDevice() const { return device.Get(); }
  ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
  ID3D12DescriptorHeap* GetSRV() const { return srvDescriptorHeap.Get(); }


  void DepthStencilView();


  //DXCユーティリティ
  ComPtr <IDxcUtils> dxcUtils = nullptr;
  //DXCコンパイラの生成
  ComPtr <IDxcCompiler3> dxcCompiler = nullptr;
  ComPtr <IDxcIncludeHandler> includeHandler = nullptr;




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
  void CreateDescriptorHeaps();

  /// <summary>
  /// レンダーターゲットの初期化
  /// </summary>
  void RenderTerggetInitialize();
  /// <summary>
  /// スワップチェーンの生成
  /// </summary>
  void CreateSwapChain();

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

  //記録時間(FPS固定用)
  std::chrono::steady_clock::time_point reference_;


  //====================//
  //====================//

  // RTV用のヒープでディスクリプタの数は2。RTVはshader内で触るものではないので、ShaderVisibleはfalse
  ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap; 
  // SRV用のヒープでディスクリプタの数は128.RTVはshader内で触るものなので、ShaderVisibleはtrue
  ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap;  
  // DSV用のヒープでディスクリプタの数は1。DSVはshader内で触るものではないので、ShaderVisibleはfalse
  ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap;

  // swapChain
  ComPtr <IDXGISwapChain4> swapChain = nullptr;
  //SwapChain(スワップチェーン)を生成する
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

  // スワップチェーンリソース
  std::array<ComPtr<ID3D12Resource>, 2> swapChainResources;

  //RTVの設定
  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

  ComPtr <ID3D12Resource> depthStencilResource;

  // DepthStencilStateの設定
  D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

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


  ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
  D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};

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
  
  //RTVを2つ作るのでディスクリプタを2つ用意
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

  uint32_t descriptorsizeSRV;
  uint32_t descriptorsizeRTV;
  uint32_t descriptorsizeDSV;

  //フェンス
  ComPtr <ID3D12Fence> fence = nullptr;
  //フェンス値
  uint64_t fenceValue = 0;
  HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  // ビューポート
  D3D12_VIEWPORT viewport{};
  // シザー短形
  D3D12_RECT scissorRect{};
  // バリア
  D3D12_RESOURCE_BARRIER barrier{};

  // FPS固定初期化
  void InitializeFixFPS();
  // FPS固定更新
  void UpdateFixFPS();

};

