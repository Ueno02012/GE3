#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3d12sdklayers.h>

#define DIRECTINPUT_VERSION     0x0800 //DirectInputのバージョン指定
#include<dinput.h>
#include <wrl.h>
#include <wrl/client.h>
#include<assert.h>

class Input
{
public:
  // namespace省略
  template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;


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
  //Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboardDevice;

  // キーボードのデバイス
  Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard;

};

