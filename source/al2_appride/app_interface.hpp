#pragma once

namespace apn::appride
{
	//
	// このクラスはアプリケーションのインターフェイスです。
	//
	inline struct app_interface_t : app_base_t
	{
		//
		// コンストラクタです。
		//
		app_interface_t() { app = this; }
	} *app = nullptr;
}
