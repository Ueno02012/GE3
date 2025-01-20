#pragma once

#include"MatrixVector.h"
#include <cstdint>
#define _USE_MATH_DEFINES
#include <math.h>
#include<cmath>

struct Sphere {
  Vector3 center; // 中心点
  float radius; // 半径
};

struct VertexData
{
  Vector4 position;
  Vector2 texcoord;
  Vector3 normal;
};



  /*------------------------------------------------------------------------------------*/
  /*-------------------------------------球の作成関数-------------------------------------*/
  /*------------------------------------------------------------------------------------*/
void DrawSphere(const uint32_t ksubdivision, VertexData* vertexdata);

