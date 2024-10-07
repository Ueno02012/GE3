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

  /// <summary>
  /// キーの入力をチェック
  /// </summary>
  /// <param name="keyNumber"></param>
  /// <returns></returns>
  bool PushKey(BYTE keyNumber);

  /// <summary>
  /// キーのトリガーをチェック
  /// </summary>
  /// <param name="keyNumber"></param>
  /// <returns></returns>
  bool TriggerKey(BYTE keyNumber);

private:

  // キーボードのデバイス
  ComPtr<IDirectInputDevice8> keyboard;

  // DirectInputのインスタンス
  ComPtr<IDirectInput8> directInput;

  // 全キーの状態
  BYTE key[256] = {};
  // 前回の全キーの状態
  BYTE keyPre[256] = {};

};

