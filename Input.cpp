#include "Input.h"


void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
  // DirectInputのインスタンス生成
  Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;
  HRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
  assert(SUCCEEDED(result));

  // キーボードデバイスの生成
  Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard = nullptr;
  result = directInput->CreateDevice(GUID_SysKeyboard, keyboard.GetAddressOf(), nullptr);
  assert(SUCCEEDED(result));

  // 入力データ形式のセット
  result = keyboard->SetDataFormat(&c_dfDIKeyboard);
  assert(SUCCEEDED(result));

  // 排他制御レベルの設定
  result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
  assert(SUCCEEDED(result));


}

void Input::Update()
{

}
