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

  void Update();

  void Draw();

  // getter
  const Vector2& GetPosition() const { return position; }
  // setter
  void SetPosition(const Vector2& position) { this->position = position; }
  // 回転
  float GetRotation()const { return rotation; }
  void SetRotation(float rotation) { this->rotation = rotation; }

  // 色
  const Vector4& GetColor() const { return materialData->color; }
  void SetColor(const Vector4& color) { materialData->color = color; }

  // サイズ
  Vector2 size = { 640.0f,360.0f };
  const Vector2& GetSize() const { return size; }
  void SetSize(const Vector2& size) { this->size = size; }

private:
  Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
  Microsoft::WRL::ComPtr <ID3D12Resource> indexResource;
  Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
  Microsoft::WRL::ComPtr <ID3D12Resource> spriteResource;
  Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource;

  //頂点リソースにデータを書き込む
  VertexData* vertexData = nullptr;
  uint32_t* indexData = nullptr;
  Material* materialData = nullptr;
  TransformationMatrix* transformationMatrixData = nullptr;

  D3D12_GPU_DESCRIPTOR_HANDLE handle;

  //頂点バッファビューを作成する
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
  D3D12_INDEX_BUFFER_VIEW indexBufferView{};

  Transform transform{ {1.0f,1.0f,1.0f},{0.0f,3.0f,0.0f},{0.0f,0.0f,0.0f} };

  Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

  Transform  cameratransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-500.0f} };

  Transform  uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

  Vector2 position = { 0.0f,0.0f };

  float rotation = 0.0f;

};

