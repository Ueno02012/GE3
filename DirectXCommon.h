#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include <d3d12sdklayers.h>
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


private:

/// <summary>
/// 深度バッファの生成
/// </summary>
  void CreateDepthBuffer();
  /// <summary>
  /// 各種DescriptorHeapの生成
  /// </summary>
  void CreateDescriptorHeap();

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
};

