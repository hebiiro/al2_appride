#pragma once

namespace apn::appride::view
{
	//
	// このクラスはビューの管理者です。
	//
	inline struct manager_t
	{
		//
		// サブプロセスウィンドウのコレクションです。
		//
		std::vector<std::shared_ptr<placeholder_t>> extrapps;

		//
		// 初期化処理を実行します。
		//
		BOOL init()
		{
			MY_TRACE_FUNC("");

			main_window.init(idd_config);

			return TRUE;
		}

		//
		// 後始末処理を実行します。
		//
		BOOL exit()
		{
			MY_TRACE_FUNC("");

			main_window.exit();

			return TRUE;
		}

		//
		// プレースホルダーを作成して返します。
		//
		auto create_placeholder(const auto& node)
		{
			// プレースホルダーを作成します。
			auto placeholder = std::make_shared<view::placeholder_t>(node);

			// プレースホルダーをコレクションに追加して返します。
			return extrapps.emplace_back(placeholder);
		}
	} manager;
}
