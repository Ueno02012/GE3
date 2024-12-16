#include "DirectXCommon.h"
#include<cassert>
#include<format>
#include"Logger.h"
#include"StringUtility.h"
//#include"Resource.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

void DirectXCommon::Initialize(WinApp* winApp)
{
  // Null検出
  assert(winApp);
  // メンバ変数に記録
  this->winApp = winApp;

  DeviceInitilaze();// デバイスの初期化
  CommandInitilize();// コマンド関連の初期化
  CreateSwapChain();// スワップチェーンの生成
  CreateDepthBuffer();// 深度バッファの生成
  CreateDescriptorHeaps();// 各種デスクリプタヒープの生成
  RenderTerggetInitialize();// レンダーターゲットビューの初期化
  DepthStencilView();// 深度ステンシルビューの初期化
  FenceInitialize();// フェンスの初期化
  ViewportRectInitialize();// ビューポート矩形の初期化
  ScissorRect();// シザリング矩形の初期化
  DXCCompiler();// DXCコンパイラの生成
  ImGuiInitilize();// ImGuiの初期化
}

ComPtr<IDxcBlob> DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile)
{
  //1.hlslファイルを読む
  //これからシェーダーをコンパイルする旨をログに出す
  Logger::Log(StringUtility::ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
  ComPtr <IDxcBlobEncoding> shaderSource = nullptr;
  hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
  //読めなかったら止める
  assert(SUCCEEDED(hr));

  //読み込んだファイルの内容を設定する
  DxcBuffer shaderSourceBuffer;
  shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
  shaderSourceBuffer.Size = shaderSource->GetBufferSize();
  shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8のコードであることを通知

  //2.Compileする
  LPCWSTR arguments[] =
  {

       filePath.c_str(),
       L"-E",L"main",
       L"-T",profile,
       L"-Zi",L"-Qembed_debug",
       L"-Od",
       L"-Zpr",
  };
  //実際にshaderをコンパイルする
  ComPtr <IDxcResult> shaderResult = nullptr;
  hr = dxcCompiler->Compile(
    &shaderSourceBuffer,
    arguments,
    _countof(arguments),
    includeHandler.Get(),
    IID_PPV_ARGS(&shaderResult)
  );

  assert(SUCCEEDED(hr));

  //警告・エラーが出てたらログを出して止める
  ComPtr <IDxcBlobUtf8> shaderError = nullptr;
  shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
  if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
    Logger::Log(shaderError->GetStringPointer());
    assert(false);
  }

  //コンパイル結果から実行用のバイナリ部分を取得
  ComPtr <IDxcBlob> shaderBlob = nullptr;
  hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
  assert(SUCCEEDED(hr));
  //成功したログを出す
  Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
  //もう使わないリソースを解放
  shaderSource->Release();
  shaderResult->Release();
  //実行用のバイナリを返却
  return shaderBlob;

}

void DirectXCommon::DeviceInitilaze()
{
  //　デバッグレイヤー
#ifdef _DEBUG
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
    //デバッグレイヤーを有効化
    debugController->EnableDebugLayer();
    //さらにGPU側でもチェックを行えるようにする
    debugController->SetEnableGPUBasedValidation(TRUE);
  }
#endif // _DEBUG

  //リソースリークチェック
  //D3DResourceLeakChecker leakCheck;

  //HRESULTはWindows系のエラーコードであり、
  //関数が成功したかどうかをSUCCEEDEDマクロで判定できる
  hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
  assert(SUCCEEDED(hr));

  // 使用するアダプター
  // 良い順にアダプターを頼む
  for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
    DXGI_ERROR_NOT_FOUND; ++i) {
    //アダプターの情報を取得する
    DXGI_ADAPTER_DESC3 adapterDesc{};
    hr = useAdapter->GetDesc3(&adapterDesc);
    assert(SUCCEEDED(hr));//取得できないのは一大事
    //ソフトウェアアダプターでなければ採用
    if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
      //採用したアダプタの情報をログに出力。wstringの方なので注意
      Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
      break;
    }
    useAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
  }
  //適切なアダプタが見つからないので起動できない
  assert(useAdapter != nullptr);

  // DirectX12デバイス
  //機能レベルとログ出力用の文字列
  D3D_FEATURE_LEVEL featureLevels[] = {
    D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
  };
  const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
  //高い順に生成できるか試す
  for (size_t i = 0; i < _countof(featureLevels); ++i) {
    //採用したアダプターでデバイスを生成
    hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
    //指定した機能レベルでデバイスが生成できたか確認
    if (SUCCEEDED(hr)) {
      //生成できたのでログ出力を行ってループを抜ける
      Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
      break;
    }
  }
  //デバイスの生成がうまくいかなかったので起動できない
  assert(device != nullptr);
  Logger::Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

}

// SRV専用の取得関数
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
{
  return GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorsizeSRV, index);
}

void DirectXCommon::CommandInitilize()
{
  // コマンドアロケータ
  hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
  //コマンドアロケータの生成がうまくいかなかったので起動できない
  assert(SUCCEEDED(hr));

  // コマンドリスト
  hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
    IID_PPV_ARGS(&commandList));
  //コマンドリストの生成がうまくいかなかったので起動できない
  assert(SUCCEEDED(hr));

  // コマンドキュー
  hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
  //コマンドキューの生成がうまくいかなかったので起動できない
  assert(SUCCEEDED(hr));

}
void DirectXCommon::CreateSwapChain()
{
  swapChainDesc.Width = WinApp::kClientWidth;//画面の幅。ウィンドウのクライアント領域を同じものにしておく
  swapChainDesc.Height = WinApp::kClientHeight;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
  swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
  swapChainDesc.BufferCount = 2;//ダブルバッファ
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに移したら、中身居を破棄
  //コマンドキュー、ウィンドウハンドル、設定を渡して生成する
  hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
  assert(SUCCEEDED(hr));

}

void DirectXCommon::CreateDepthBuffer()
{
  depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kClientWidth, WinApp::kClientHeight);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
{
  return GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorsizeSRV, index);
}

ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes)
{
  //頂点リソース用のヒープの設定
  D3D12_HEAP_PROPERTIES uploadHeapProperties{};
  uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
  //頂点リソースの設定
  D3D12_RESOURCE_DESC vertexResourceDesc{};
  //バッファリソース、テクスチャの場合はまた別の設定をする
  vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  vertexResourceDesc.Width = sizeInBytes;
  //バッファの場合はこれらは１にする
  vertexResourceDesc.Height = 1;
  vertexResourceDesc.DepthOrArraySize = 1;
  vertexResourceDesc.MipLevels = 1;
  vertexResourceDesc.SampleDesc.Count = 1;
  //バッファの場合はこれにする
  vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  //実際に頂点リソースを作る
  Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
  hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
    &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&Resource));
  assert(SUCCEEDED(hr));
  return Resource;
}

ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata)
{
  //1. metadataを基にResourceの設定
  D3D12_RESOURCE_DESC resourceDesc{};
  resourceDesc.Width = UINT(metadata.width);									//Textureの幅
  resourceDesc.Height = UINT(metadata.height);								//Textureの高さ
  resourceDesc.MipLevels = UINT16(metadata.mipLevels);						//mipmapの数
  resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);					//奥行 or 配列Textureの配列行数
  resourceDesc.Format = metadata.format;										//TextureのFormat
  resourceDesc.SampleDesc.Count = 1;											//サンプリングカウント。1固定
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);		//Textureの次元数。普段使っているのは二次元

  //2. 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある
  D3D12_HEAP_PROPERTIES heapProperties{};
  heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;								//細かい設定を行う
  heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;		//WriteBackポリシーでCPUアクセス可能
  heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;					//プロセッサの近くに配膳

  //3. Resourceを生成する
  hr = device->CreateCommittedResource(
    &heapProperties,														//Heapの設定
    D3D12_HEAP_FLAG_NONE,													//Heapの特殊な設定。特になし。
    &resourceDesc,															///Resourceの設定
    D3D12_RESOURCE_STATE_GENERIC_READ,										//初回のResourceState。Textureは基本読むだけ
    nullptr,																//Clear最適値。使わないのでnullptr
    IID_PPV_ARGS(&resource));												//作成するResourceポインタへのポインタ
  assert(SUCCEEDED(hr));
  return resource;
}


void DirectXCommon::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
{  //Textureを読んで転送する

  //Meta情報を取得
  const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
  //全MipMapについて
  for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel)
  {
    //MipMapLevelを指定して各Imageを取得
    const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
    //Textureに転送
    hr = texture->WriteToSubresource(
      UINT(mipLevel),
      nullptr,				//全領域へコピー
      img->pixels,			//元データアドレス
      UINT(img->rowPitch),	//1ラインサイズ
      UINT(img->slicePitch)	//1枚サイズ
    );
    assert(SUCCEEDED(hr));
  }

}

DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath)
{
  //テクスチャファイルを呼んでプログラムで扱えるようにする
  DirectX::ScratchImage image{};
  std::wstring filePathW = StringUtility::ConvertString(filePath);
  HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
  assert(SUCCEEDED(hr));

  //ミップマップの作成
  DirectX::ScratchImage mipImages{};
  hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
  assert(SUCCEEDED(hr));

  //ミップマップ付きのデータを返す
  return mipImages;
}

ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(ComPtr <ID3D12Device>& device, int32_t width, int32_t heigth)
{
  D3D12_RESOURCE_DESC resourceDesc{};
  // 生成するResourceの設定
  resourceDesc.Width = width; // Textureの幅
  resourceDesc.Height = heigth; // Textureの高さ
  resourceDesc.MipLevels = 1; // mipmapの数
  resourceDesc.DepthOrArraySize = 1; // 奥行　or 配列のTexture配列数
  resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
  resourceDesc.SampleDesc.Count = 1; // サンプリングカウント 1固定
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
  resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

  // 利用Heapの設定
  D3D12_HEAP_PROPERTIES heapProperies{};
  heapProperies.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAW上に作る

  // 深度値のクリア設定
  D3D12_CLEAR_VALUE depthClearValue{};
  depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f (最大値)でクリア
  depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。 Resourceと合わせる

  //3. Resourceを生成する
  hr = device->CreateCommittedResource(
    &heapProperies, //Heapの設定
    D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定。特になし。
    &resourceDesc, //Resourceの設定
    D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値を書き込む状態にしておく
    &depthClearValue, //Clear最適値
    IID_PPV_ARGS(&resource)); //作成するResourceポインタへのポインタ
  assert(SUCCEEDED(hr));
  return resource;

}

void DirectXCommon::DepthStencilView()
{
  // DSVの設定
  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
  dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format。基本的にはResource合わせる
  dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2dTexture 
  // DSVDescの先頭にDSVを作る
  device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}


// デスクリプタヒープ生成関数
void DirectXCommon::CreateDescriptorHeaps()
{
  rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
  srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
  dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);


  // DescriptorSizeを取得する
  descriptorsizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  descriptorsizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  descriptorsizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

}

ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{

  //ディスクリプタヒープの生成
  descriptorHeapDesc.Type = heapType;
  descriptorHeapDesc.NumDescriptors = numDescriptors;
  descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));

  //ディスクリプタヒープが作れなかったので起動できない
  assert(SUCCEEDED(hr));
  return descriptorHeap;
}


void DirectXCommon::RenderTerggetInitialize()
{
  //SwapChainからResourceを引っ張ってくる
  hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
  //上手く取得できなければ起動できない
  assert(SUCCEEDED(hr));
  hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
  assert(SUCCEEDED(hr));

  rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGB2変換して書き込む
  rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャとして読み込む
  //ディスクリプタの先頭を取得する
  D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorsizeRTV, 0);
  //まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
  rtvHandles[0] = rtvStartHandle;
  device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
  //2つ目のディスクリプタハンドルを得る
  rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  //2つ目を作る
  device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);
}

void DirectXCommon::FenceInitialize()
{
  //初期値0でFenceを作る
  hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
  assert(SUCCEEDED(hr));
  //FenceのSignalを待つためのイベントを作成する
  HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  assert(fenceEvent != nullptr);

}

void DirectXCommon::ViewportRectInitialize()
{
  //クライアント領域のサイズと一緒にして画面全体に表示
  viewport.Width = WinApp::kClientWidth;
  viewport.Height = WinApp::kClientHeight;
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;


}

void DirectXCommon::ScissorRect()
{
  // 基本的にビューポートと同じ矩形が構成されるようにする
  scissorRect.left = 0;
  scissorRect.right = WinApp::kClientWidth;
  scissorRect.top = 0;
  scissorRect.bottom = WinApp::kClientHeight;

}

void DirectXCommon::DXCCompiler()
{
  // dxCompilerを初期化
  hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
  assert(SUCCEEDED(hr));
  hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
  assert(SUCCEEDED(hr));
  //現時点でincludeはしないが、includeに対応するための設定を行っていく
  hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
  assert(SUCCEEDED(hr));

}

void DirectXCommon::ImGuiInitilize()
{
  //-------ImGuiの初期化-----------//
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplWin32_Init(winApp->GetHwnd());
  ImGui_ImplDX12_Init(device.Get(),
    swapChainDesc.BufferCount,
    rtvDesc.Format,
    srvDescriptorHeap.Get(),
    srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
    srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

}

void DirectXCommon::PreDraw()
{
  //バックバッファの番号取得
  UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  // Noneにしておく
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
  // 遷移前のResourceState
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  // 遷移後のResourceState
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  // TransitionBarrierを張る
  commandList->ResourceBarrier(1, &barrier);
  commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
  //描画先のRTVとDSVを指定する
  D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
  commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

  // 画面全体の色をクリア
  float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順

  //画面全体の深度をクリア
  commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
  commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

  commandList->RSSetViewports(1, &viewport);
  commandList->RSSetScissorRects(1, &scissorRect);

}

void DirectXCommon::PostDraw()
{
  UINT bbIndex = swapChain->GetCurrentBackBufferIndex();
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  commandList->ResourceBarrier(1, &barrier);

  commandList->RSSetViewports(1, &viewport);
  commandList->RSSetScissorRects(1, &scissorRect);
  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  hr = commandList->Close();
  assert(SUCCEEDED(hr));

  // GPUにコマンドリストのリストの実行を行わせる
  ID3D12CommandList* commandLists[] = { commandList.Get() };
  commandQueue->ExecuteCommandLists(1, commandLists);
  // GPUとOSに画面の交換を行うように通知する
  swapChain->Present(1, 0);
  // Fenceの値の更新
  fenceValue++;
  // GPUがここまでたどり着いたときに、Fenceの値に代入するようにSignalを送る
  commandQueue->Signal(fence.Get(), fenceValue);
  assert(SUCCEEDED(hr));
  // Fenceの値が指定したSignal値にたどり着いているか確認する
  // GetCompletedValueの初期値はFence作成時に渡した初期値
  if (fence->GetCompletedValue() < fenceValue)
  {
    // 指定したSignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
    fence->SetEventOnCompletion(fenceValue, fenceEvent);
    //イベントを待つ
    WaitForSingleObject(fenceEvent, INFINITE);
  }
  // 次のフレーム用のコマンドリストを準備
  hr = commandAllocator->Reset();
  assert(SUCCEEDED(hr));
  hr = commandList->Reset(commandAllocator.Get(), nullptr);
  assert(SUCCEEDED(hr));


}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
  D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
  handleCPU.ptr += (descriptorSize * index);
  return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{

  D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
  handleGPU.ptr += (descriptorSize * index);
  return handleGPU;
}

