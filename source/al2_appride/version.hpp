#pragma once

namespace apn::appride
{
	//
	// このクラスはこのプログラムのバージョンを管理します。
	//
	inline struct version_t : version_base_t {
		//
		// コンストラクタです。
		//
		version_t() : version_base_t(L"外部アプリ前面表示", L"🐍外部アプリ前面表示🔖", L"r1") {}
	} version;
}
