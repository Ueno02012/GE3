#include "Input.h"


void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
  // DirectInputのインスタンス生成
  ComPtr<IDirectInput8> directInput = nullptr;
  HRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
  assert(SUCCEEDED(result));

  // キーボードデバイスの生成
  ComPtr<IDirectInputDevice8> keyboardDevice = nullptr;
  result = directInput->CreateDevice(GUID_SysKeyboard, keyboardDevice.GetAddressOf(), nullptr);
  assert(SUCCEEDED(result));

  // 入力データ形式のセット
  result = keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
  assert(SUCCEEDED(result));

  // 排他制御レベルの設定
  result = keyboardDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
  assert(SUCCEEDED(result));


}

void Input::Update()
{
  // キーボード情報の取得開始
  keyboard->Acquire();

  // 全キーの入力情報を取得する
  BYTE key[256] = {};
  keyboard->GetDeviceState(sizeof(key), key);
}
