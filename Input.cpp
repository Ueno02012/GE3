#include "Input.h"


void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
  // DirectInputのインスタンス生成
  HRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
  assert(SUCCEEDED(result));

  // キーボードデバイスの生成
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
  // 前回のキー入力を保存
  memcpy(keyPre, key, sizeof(key));

  // キーボード情報の取得開始
  keyboard->Acquire();

  // 全キーの入力情報を取得する
  keyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber)
{
  //指定キーを押していればtrueを返す
  if (key[keyNumber]){
    return true;
  }
  // そうでなければfalseを返す
  return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
  if (key[keyNumber] != 0 && keyPre[keyNumber] ==0) {
    return true;
  }
  return false;
}
