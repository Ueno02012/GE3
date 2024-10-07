#pragma once
#include <Windows.h>

// WindowsAPI
class WinApp
{
public:// 静的メンバ変数

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:// メンバ変数

  /// <summary>
  /// 初期化
  /// </summary>
  void Initialize();

  /// <summary>
  /// 更新
  /// </summary>
  void Update();

};

