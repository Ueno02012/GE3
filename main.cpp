#include<cstdint>
#include<string>
#include<format>
#include"Transform.h"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>

#include<dxgidebug.h>
#include<dxcapi.h>
#include<cmath>
#include<assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
#include"MatrixVector.h"
#include<fstream>
#include<sstream>

#define DIRECTINPUT_VERSION     0x0800 //DirectInputのバージョン指定
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include"Logger.h"

#include"Sphere.h"
#include"Sprite.h"
#include"SpriteCommon.h"


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#pragma comment(lib,"dxcompiler.lib")


//struct TransformationMatrix {
//  Matrix4x4 WVP;
//  Matrix4x4 World;
//};

//struct Material {
//  Vector4 color;
//  int32_t endbleLighting;
//  float padding[3];
//  Matrix4x4 uvTransform;
//};

struct DirectionalLight {
  Vector4 color; //!< ライトの色
  Vector3 direction; //!< ライトの向き
  float intensity; //!< 輝度
};

struct MaterialDate {
  std::string textureFilePath;
};
struct ModelDate {
  std::vector<VertexData> vertices;
  MaterialDate material;
};




bool DepthFunc(float currZ, float prevZ) {
  return currZ <= prevZ;
}





/*----------------------------------------------------------------------*/
/*-------------------------Objファイルを読む関数---------------------------*/
/*----------------------------------------------------------------------*/


MaterialDate LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
   //1. 中で必要となる変数の宣言
  MaterialDate materialDate; // 構築するMaterialDate
  std::string line; // ファイルから読んだ1行を格納するもの
  std::ifstream file(directoryPath + "/" + filename); // 2.ファイルを開く
  assert(file.is_open()); // とりあえず開けなかったら止める
   //3. 実際にファイルを読み、MaterialDateを構築していく
  while (std::getline(file, line)) {
    std::string identifier;
    std::istringstream s(line);
    s >> identifier;

     //identifierの応じた処理
    if (identifier == "map_Kd") {
      std::string textureFilename;
      s >> textureFilename;
       //連結してファイルパスにする
      materialDate.textureFilePath = directoryPath + "/" + textureFilename;
    }
  }
  return materialDate;
}

ModelDate LoadObjFile(const std::string& directoryPath, const std::string& filename) {
   //1. 中で必要となる変数の宣言
  ModelDate modelDate; // 構築するModelDate
  std::vector<Vector4> positions; // 位置
  std::vector<Vector3> normals; // 法線
  std::vector<Vector2> texcoords; // テクスチャ座標
  std::string line; // ファイルから読んだ1桁を格納するもの
   //2.  ファイルを開く
  std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
  assert(file.is_open()); // とりあえず開けなかったら止める

   //3. 実際にファイルを読み、ModelDateを構築していく
  while (std::getline(file, line)) {
    std::string identifier;
    std::istringstream s(line);
    s >> identifier;// 先頭の識別子を読む

     //identifierの応じた処理
    if (identifier == "v") {
      Vector4 position;
      s >> position.x >> position.y >> position.z;
      position.x *= -1.0f;// 位置のx成分を反転
      position.w = 1.0f;
      positions.push_back(position);
    }
    else if (identifier == "vt") {
      Vector2 texcoord;
      s >> texcoord.x >> texcoord.y;
      texcoord.y = 1.0f - texcoord.y;
      texcoords.push_back(texcoord);
    }
    else if (identifier == "vn") {
      Vector3 normal;
      s >> normal.x >> normal.y >> normal.z;
      normal.x *= -1.0f;// 法線のx成分を反転
      normals.push_back(normal);
    }
    else if (identifier == "f") {
      VertexData triangle[3];
       //面は三角形限定。その他は未対応
      for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
        std::string vertexDefinition;
        s >> vertexDefinition;
         //頂点の要素へのIndexは、[位置/UV/法線]で格納されているので、分解してIndexを取得する
        std::istringstream v(vertexDefinition);
        uint32_t elementIndices[3];
        for (uint32_t element = 0; element < 3; ++element) {
          std::string index;
          std::getline(v, index, '/');// /区切りでインデックスを読んでいく
          elementIndices[element] = std::stoi(index);
        }
         //要素のIndexから、実際の要素の値を取得して、頂点を構築する
        Vector4 position = positions[elementIndices[0] - 1];
        Vector2 texcoord = texcoords[elementIndices[1] - 1];
        Vector3 normal = normals[elementIndices[2] - 1];
        VertexData vertex = { position,texcoord,normal };
        modelDate.vertices.push_back(vertex);
        triangle[faceVertex] = { position,texcoord,normal };
      }
       //頂点を逆順で登録することで、回り順を逆にする
      modelDate.vertices.push_back(triangle[2]);
      modelDate.vertices.push_back(triangle[1]);
      modelDate.vertices.push_back(triangle[0]);
    }
    else if (identifier == "mtllib") {
       //materialTemplateLibrarvファイルの名前を取得する
      std::string materialFilename;
      s >> materialFilename;
       //基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
      modelDate.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
    }
  }
   //4. ModelDateを返す
  return modelDate;
}

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#pragma region 基盤システムの初期化

  OutputDebugStringA("Hello,Directx!\n");

  //WindowsAPIのポインタ
  WinApp* winApp = nullptr;
  //WindowsAPIの初期化
  winApp = new WinApp();
  winApp->Initialize();

  //入力のポインタ
  Input* input = nullptr;
  //入力の初期化
  input = new Input();
  input->Initialize(winApp);

  //DirectXCommonのポインタ
  DirectXCommon* dxCommon = nullptr;

  // DirectXの初期化
  dxCommon = new DirectXCommon();
  dxCommon->Initialize(winApp);

  SpriteCommon* spriteCommon = nullptr;
  // スプライト共通部の初期化
  spriteCommon = new SpriteCommon;
  spriteCommon->Initialize(dxCommon);


#pragma endregion 基盤システムの初期化

#pragma region 最初のシーンの初期化

  Sprite* sprite = new Sprite();
  sprite->Initialize(spriteCommon);

  std::vector<Sprite*> sprites;
  for (uint32_t i = 0; i < 5; i++) {
    Sprite* sprite = new Sprite();
    sprite->Initialize(spriteCommon);
    sprites.push_back(sprite);
  }

#pragma endregion 最初のシーンの終了


  /*------------------------------------------------------------------*/
  /*----------------------マテリアル用のResource------------------------*/
  /*------------------------------------------------------------------*/

  // マテリアル用のリソース
  Microsoft::WRL::ComPtr <ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
  // マテリアル用にデータを書き込む
  Material* materialData = nullptr;
  // 書き込むためのアドレスを取得
  materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
  // 今回は白
  materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
  // Lightingするのでtrueを設定する
  materialData->endbleLighting = true;
  // 単位行列を書き込んでおく
  materialData->uvTransform = MakeIdentity4x4();

  /*------------------------------------------------------------------*/
  /*----------------TransformationMatrix用のResource-------------------*/
  /*------------------------------------------------------------------*/

   // WVP,World用のリソースを作る。TransformationMatrixを用意する
  Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource =dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
  // データを書き込む
  TransformationMatrix* transformationMatrixData = nullptr;
  // 書き込むためのアドレスを取得
  wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
  // 単位行列を書き込んでおく
  transformationMatrixData->WVP = MakeIdentity4x4();
  transformationMatrixData->World = MakeIdentity4x4();

  /*------------------------------------------------------------------*/
  /*------------------------Sprite用のResource-------------------------*/
  /*------------------------------------------------------------------*/

   //Sprite用のマテリアルリソースを作る
  Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceSprite =dxCommon->CreateBufferResource(sizeof(Material));
  // Sprite用にデータを書き込む
  Material* materialSpriteDate = nullptr;
  // 書き込むためのアドレスを取得
  materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialSpriteDate));
  // 今回は白
  materialSpriteDate->color = { 1.0f, 1.0f, 1.0f, 1.0f };
  // SpriteはLightingしないでfalseを設定する
  materialSpriteDate->endbleLighting = false;
  // 単位行列を書き込んでおく
  materialSpriteDate->uvTransform = MakeIdentity4x4();

  /*------------------------------------------------------------------*/
  /*-----------------------平行光源用のResource-------------------------*/
  /*------------------------------------------------------------------*/

   // 平行光源用のリソースを作る
  Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource =dxCommon->CreateBufferResource(sizeof(DirectionalLight));
  // 平行光源用にデータを書き込む
  DirectionalLight* directionalLightDate = nullptr;
  // 書き込むためのアドレスを取得
  directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDate));
  // デフォルト値はとりあえず以下のようにして置く
  directionalLightDate->color = { 1.0f, 1.0f, 1.0f, 1.0f };
  directionalLightDate->direction = { 0.0f,-1.0f,0.5f };
  directionalLightDate->intensity = 1.0f;

  /*-------------------------------------------------------*/
 /*----------------------球のデータ-------------------------*/
 /*-------------------------------------------------------*/

  const uint32_t kSubdivision = 16; //球の分割数

  uint32_t vertexCount = kSubdivision * kSubdivision * 6; //球の頂点数

  // モデル読み込み
  ModelDate modelDate = LoadObjFile("resources", "plane.obj");

  // 関数化したResouceで作成
  Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce =dxCommon->CreateBufferResource(sizeof(VertexData) * modelDate.vertices.size());

  /*-----------------------------------------------------*/
  /*-----------------------球の描画-----------------------*/
  /*-----------------------------------------------------*/

  //頂点バッファビューを作成する
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
  // リソースの先頭のアドレスから使う
  vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
  // 使用するリソースのサイズはの頂点のサイズ
  vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelDate.vertices.size());
  // 1頂点当たりのサイズ
  vertexBufferView.StrideInBytes = sizeof(VertexData);

  //頂点リソースにデータを書き込む
  VertexData* vertexData = nullptr;
  //書き込むためのアドレスを取得
  vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
  // 頂点データをリソースにコピー
  std::memcpy(vertexData, modelDate.vertices.data(), sizeof(VertexData) * modelDate.vertices.size());

  /*------------------------------------------------------------*/
  /*--------------------------SRVの設定--------------------------*/
  /*------------------------------------------------------------*/

  //Textureを読んで転送する
  DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/uvChecker.png");
  const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
  Microsoft::WRL::ComPtr <ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
  dxCommon->UploadTextureData(textureResource.Get(), mipImages);

  //2枚目のTextureを読んで転送する
  DirectX::ScratchImage mipImages2 =dxCommon->LoadTexture(modelDate.material.textureFilePath);
  const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
  Microsoft::WRL::ComPtr <ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2);
  dxCommon->UploadTextureData(textureResource2.Get(), mipImages2);

  //metaDataを基にSRVの設定
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
  srvDesc.Format = metadata.format;
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;				//2Dテクスチャ
  srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
  srvDesc2.Format = metadata2.format;
  srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;				//2Dテクスチャ
  srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

  ////SRVを作成するDescriptorHeapの場所を決める
  D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
  D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);
 
  //SRVの生成
  dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

  ////SRVを作成するDescriptorHeapの場所を決める
  D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetSRVCPUDescriptorHandle(2);
  D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetSRVGPUDescriptorHandle(2);

  //SRVの生成
  dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

  /*----------------------------------------------------------------------------------*/
  /*----------------------------各Descriptorの設定終了----------------------------------*/
  /*----------------------------------------------------------------------------------*/

   //======== ShaderをCompile ===========// 
  Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
  assert(vertexShaderBlob != nullptr);
  Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
  assert(pixelShaderBlob != nullptr);


  Transform transform{ {1.0f,1.0f,1.0f},{0.0f,3.0f,0.0f},{0.0f,0.0f,0.0f} };

  Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

  Transform  cameratransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-500.0f} };

  Transform  uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };


  bool useMonsterBall = true;



 //==================================================//
 //=================== メインループ ===================//
 //==================================================//

  // ウィンドウの×ボタンが押されるまでループ
  while (true) {
    // Windowにメッセージが来ていたら最優先で処理させる
    if (winApp->ProcessMessage()) {
      // ゲームループを抜ける
      break;
    }
    
  // ここからゲームの処理

  //===================================================//
  //==================== 入力の更新 =====================//
  //===================================================//

    input->Update();
    if (input->PushKey(DIK_A))
    {
      transform.translate.x -= 0.1f;
    }
    if (input->PushKey(DIK_D))
    {
      transform.translate.x += 0.1f;
    }
    if (input->TriggerKey(DIK_W)) {
      transform.translate.y += 0.1f;
    }
    if (input->TriggerKey(DIK_S)) {
      transform.translate.y -= 0.1f;
    }
    //=========================================================//
    // ===================== 移動テスト =========================//
    // ========================================================//
    // 現在の座標を変数で受ける
    Vector2 position = sprite->GetPosition();
    // 座標を変更する
    position.x += 0.1f;
    position.y += 0.1f;
    //変更を反映する
    sprite->SetPosition(position);
    sprite->Update();

    //=========================================================//
    // ===================== 回転テスト =========================//
    // ========================================================//
    // 角度を変化させるテスト
    float rotation = sprite->GetRotation();
    //rotation += 0.01f;
    sprite->SetRotation(rotation);

    //========================================================//
    // ===================== 色を変えるテスト ===================//
    // =======================================================//
    Vector4 color = sprite->GetColor();
    color.x += 0.01f;
    if (color.x > 1.0f) {
      color.x -= 1.0f;
    }
    sprite->SetColor(color);
    //========================================================//
    // ===================== サイズ変更のテスト ===================//
    // =======================================================//
    Vector2 size = sprite->GetSize();
    size.x += 0.5f;
    size.y += 0.5f;
    sprite->SetSize(size);


    //========================================//
    //================ ImGui =================//
    //========================================// 
    
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    //開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
    ImGui::ShowDemoWindow();

    ImGui::Begin("Sprite");
    ImGui::DragFloat3("translate", &transform.translate.x, 0.01f);
    ImGui::DragFloat3("translate", &position.x, 0.01f);
    ImGui::SliderAngle("SphererRotateX", &transform.rotate.x);
    ImGui::SliderAngle("SphererRotateY", &transform.rotate.y);
    ImGui::SliderAngle("SphererRotateZ", &transform.rotate.z);
    ImGui::ColorEdit3("colorSprite", reinterpret_cast<float*>(materialSpriteDate));
    ImGui::Checkbox("useMonsterBall", &useMonsterBall);
    ImGui::DragFloat3("LightDirection", &directionalLightDate->direction.x, 0.01f);
    ImGui::DragFloat("LightIntensity", &directionalLightDate->intensity, 0.01f);
    ImGui::DragFloat3("SpriteTranslate", (&transformSprite.translate.x));
    ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
    ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
    ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
    ImGui::End();

    ImGui::Render();

    /*------------------------------------------*/
    /*---------MVP,WorldMatrixの行列を作る--------*/
    /*------------------------------------------*/

    Matrix4x4 worludMatrix = MakeAftineMatrix(transform.scale, transform.rotate, transform.translate);
    Matrix4x4 cameraMatrix = MakeAftineMatrix(cameratransform.scale, cameratransform.rotate, cameratransform.translate);
    Matrix4x4 viewMatrix = Inverse(cameraMatrix);
    Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
    Matrix4x4 worldViewProjectionMatrix = Multiply(worludMatrix, Multiply(viewMatrix, projectionMatrix));
    transformationMatrixData->World = worludMatrix;
    transformationMatrixData->WVP = worldViewProjectionMatrix;

    /*-------------------------------------------*/
    /*---Sprite用のWorldViewProjectionMatrixを作る---*/
    /*--------------------------------------------*/

    /*Matrix4x4 worludMatrixSprite = MakeAftineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
    Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
    Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
    Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worludMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
    transformationMatrixDateSprite->World = worludMatrixSprite;
    transformationMatrixDateSprite->WVP = worldViewProjectionMatrixSprite;*/

    /*----------------------------------------*/
    /*---------UVTransform用の行列を作る--------*/
    /*----------------------------------------*/

    Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
    uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
    uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
    materialSpriteDate->uvTransform = uvTransformMatrix;

    // 描画用のDescriptorHeapの設定
    ID3D12DescriptorHeap* descriptorHeap[] = { dxCommon->GetSRV()};
    dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeap);

    // 描画前処理
    dxCommon->PreDraw();


    spriteCommon->DrawCommonSeting();

    //// RootSignatureを設定。PSOに設定しているけど別途設定が必要
    dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    //// マテリアルCBufferの場所を設定
    dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    //// wvp用のCBufferの場所を設定
    dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
    ////SRVのDescriptortableの先頭を設定。２はrootParameter[2]である。
    ////SRVを切り替えて画像を変えるS
    dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
    //// 平行光源用のCBufferの場所を設定 
    dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

    //// 描画！(今回は球) 
    dxCommon->GetCommandList()->DrawInstanced(UINT(modelDate.vertices.size()), 1, 0, 0);

    /*---------------------------------------------------*/
    /*-------------------2dの描画コマンド開始---------------*/
    /*---------------------------------------------------*/
    sprite->Draw();

    /*---------------------------------------------------*/
    /*-------------------2dの描画コマンド終了---------------*/
    /*---------------------------------------------------*/

    ////実際のcommandListのImGuiの描画コマンドを積む
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

    dxCommon->PostDraw();
    
  }

  // windowsAPIの終了処理
  winApp->Finalize();

  //ImGuiの終了処理。
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  //CloseHandle(fenceEvent);
  delete input;

  // WindowsAPI解放
  delete winApp;

  delete sprite;
  delete spriteCommon;

  // DirectX解放
  delete dxCommon;

  return 0;
}