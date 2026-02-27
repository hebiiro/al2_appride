#pragma once

namespace apn::appride
{
	//
	// このクラスはアプリケーションです。
	//
	inline struct app_t : app_base_t
	{
		//
		// プラグインの初期化処理を実行します。
		//
		virtual BOOL on_initialize_plugin(DWORD version) override
		{
			MY_TRACE_FUNC("");

			// 各種開始処理を実行します。
			debug.init();
			config_io.init();
			view::manager.init();

			// コンフィグをファイルから読み込みます。
			// ※書き込みはコンフィグダイアログの終了時に実行します。
			config_io.read();

			return TRUE;
		}

		//
		// プラグインの後始末処理を実行します。
		//
		virtual BOOL on_uninitialize_plugin() override
		{
			MY_TRACE_FUNC("");

			// 各種終了処理を実行します。
			view::manager.exit();
			config_io.exit();
			debug.exit();

			return TRUE;
		}

		//
		// プラグインを登録します。
		//
		virtual BOOL on_register_plugin(HOST_APP_TABLE* host) override
		{
			MY_TRACE_FUNC("");

			// メインウィンドウを登録します。
			host->register_window_client(version_base->get_name().c_str(), view::main_window);

			// ノードを走査します。
			for (const auto& node : model::property.nodes)
			{
				// プレースホルダーを作成します。
				auto placeholder = view::manager.create_placeholder(node);

				// プレースホルダーを登録します。
				host->register_window_client(node->display_name.c_str(), *placeholder);
			}

			return TRUE;
		}
	} app_impl;
}
