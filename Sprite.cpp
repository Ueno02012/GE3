#include "Sprite.h"
#include "SpriteCommon.h"

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
  // 引数で受け取ってメンバ変数に記録する
  this->spriteCommon = spriteCommon;


  vertexResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);
  indexResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);

  // リソースの先頭のアドレスから使う
  vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
  // 使用するリソースのサイズは4つ分のサイズ
  vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
  // 1頂点当たりのサイズ
  vertexBufferView.StrideInBytes = sizeof(VertexData);
  // リソースの先頭のアドレスから使う
  indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
  // 使用するリソースのサイズは6つ分のサイズ
  indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
  // インデックスはuint32_tとする
  indexBufferView.Format = DXGI_FORMAT_R32_UINT;

  //書き込むためのアドレスを取得
  vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

  indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
  indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
  indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

  materialResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));

  // 書き込むためのアドレスを取得
  materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
  // 今回は白
  materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
  // SpriteはLightingしないでfalseを設定する
  materialData->endbleLighting = false;
  // 単位行列を書き込んでおく
  materialData->uvTransform = MakeIdentity4x4();
  wvpResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
  // 書き込むためのアドレスを取得
  wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
  // 単位行列を書き込んでおく
  transformationMatrixData->WVP = MakeIdentity4x4();
  transformationMatrixData->World = MakeIdentity4x4();


}

void Sprite::Update()
{
  vertexData[0].position = { 0.0f,1.0f,0.0f,1.0f };
  vertexData[0].texcoord = { 0.0f,1.0f };

  vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };
  vertexData[1].texcoord = { 0.0f,0.0f };

  vertexData[2].position = { 1.0f,1.0f,0.0f,1.0f };
  vertexData[2].texcoord = { 1.0f,1.0f };

  vertexData[3].position = { 1.0f,0.0f,0.0f,1.0f };
  vertexData[3].texcoord = { 1.0f,0.0f };
  for (int i = 0; i < 4; i++) {
    vertexData[i].normal = { 0.0f,0.0f,-1.0f };
  }

  transformSprite.translate = { position.x,position.y,0.0f };
  transformSprite.rotate = { 0.0f,0.0f,rotation };
  transformSprite.scale = { size.x,size.y,1.0f };
  Matrix4x4 worludMatrixSprite = MakeAftineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
  Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
  Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
  Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worludMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
  transformationMatrixData->World = worludMatrixSprite;
  transformationMatrixData->WVP = worldViewProjectionMatrixSprite;

}

void Sprite::Draw()
{

  spriteCommon->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);//IBVを設定
  //// wvp用のCBufferの場所を設定
  spriteCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

  //// Spriteの描画。変更が必要なものだけ変更する
  spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
  spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

  spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, spriteCommon->GetDxCommon()->GetSRVGPUDescriptorHandle(1));

  //// wvp用のCBufferの場所を設定
  spriteCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

  //// 描画! (DrawCall/ドローコール) 6個のインデックスを使用し1つのインスタンスを描画、その他は当面０で良い
  spriteCommon->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}
