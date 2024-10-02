#include "Input.h"


void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
  // DirectInputのインスタンス生成
  Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;
  HRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
  assert(SUCCEEDED(result));

  // キーボードデバイスの生成
  Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboardDevice = nullptr;
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

}
