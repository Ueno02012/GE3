#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3d12sdklayers.h>
#include<dinput.h>
#include <wrl/client.h>
#include<assert.h>

class Input
{
public: // メンバ変数

  /// <summary>
  /// 初期化
  /// </summary>
  void  Initialize(HINSTANCE hInstance, HWND hwnd);

  /// <summary>
  /// 更新
  /// </summary>
  void Update();

private:
  // DirectInputキーボードデバイスを保持するメンバ変数
  Microsoft::WRL::ComPtr<IDirectInputDevice8> m_keyboardDevice;

};

