#pragma once
#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include"Matrix4x4.h"
#define _USE_MATH_DEFINES
#include <cmath>
// 単位行列の作成
Matrix4x4 MakeIdentity4x4(); 

//長さ（ノルム）
float Length(const Vector3& v);

//正規化
Vector3  Normalize(const Vector3& v);
    
// 逆行列
Matrix4x4 Inverse(const Matrix4x4& matrix);

//平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& teanslate);
    
// スケーリング行列
Matrix4x4 MakeScaleMatrix(const Vector3& scale);


// 透視投影行列　
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farclip);

// 平行投影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float facDlip);

// X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian);

// Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian);

// Z軸回8;転行列
Matrix4x4 MakeRotateZMatrix(float radian);

// 合成
Matrix4x4 Multiply(const Matrix4x4 m1, const Matrix4x4 m2);


// 3次元アフィン変換
Matrix4x4 MakeAftineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
