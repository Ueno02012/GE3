#pragma once
#include"Sphere.h"
#include"MatrixVector.h"
#include "Transform.h"
#include<wrl.h>
#include<d3d12.h>


struct Material {
  Vector4 color;
  int32_t endbleLighting;
  float padding[3];
  Matrix4x4 uvTransform;
};
struct TransformationMatrix {
  Matrix4x4 WVP;
  Matrix4x4 World;
};


class SpriteCommon;

// スプライト
class Sprite
{
public: // メンバ変数

  SpriteCommon* spriteCommon = nullptr;
  // 初期化
  void Initialize(SpriteCommon* spriteCommon);


  Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
  Microsoft::WRL::ComPtr <ID3D12Resource> indexResource;
  Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
  Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource;

  //頂点リソースにデータを書き込む
  VertexData* vertexData = nullptr;
  uint32_t* indexData = nullptr;
  Material* materialData = nullptr;
  TransformationMatrix* transformationMatrixData = nullptr;

  //頂点バッファビューを作成する
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
  D3D12_INDEX_BUFFER_VIEW indexBufferView{};

  Transform transform{ {1.0f,1.0f,1.0f},{0.0f,3.0f,0.0f},{0.0f,0.0f,0.0f} };

  Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

  Transform  cameratransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-500.0f} };

  Transform  uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

  void Draw();




};

