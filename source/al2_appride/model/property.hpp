#pragma once

namespace apn::appride::model
{
	//
	// このクラスはモデルのプロパティです。
	//
	inline struct property_t
	{
		//
		// このクラスは終了モードです。
		//
		inline static constexpr struct exit_mode_t {
			inline static constexpr int32_t c_window = 0;
			inline static constexpr int32_t c_process = 1;
			inline static constexpr my::Label labels[] = {
				{ c_window, L"window" },
				{ c_process, L"process" },
			};
		} c_exit_mode;

		//
		// このクラスはノードです。
		//
		struct node_t {
			BOOL flag_disable;
			std::wstring display_name;
			std::wstring class_name;
			std::wstring window_name;
			std::filesystem::path path;
			int32_t exit_mode = c_exit_mode.c_window;

			//
			// コンストラクタです。
			//
			node_t(
				BOOL flag_disable = FALSE,
				std::wstring display_name = L"",
				std::wstring class_name = L"",
				std::wstring window_name = L"",
				std::filesystem::path path = L"",
				int32_t exit_mode = c_exit_mode.c_window)
				: flag_disable(flag_disable)
				, display_name(display_name)
				, class_name(class_name)
				, window_name(window_name)
				, path(path)
				, exit_mode(exit_mode)
			{
			}
		};

		//
		// ノードのコレクションです。
		//
		std::vector<std::shared_ptr<node_t>> nodes;

		//
		// サブプロセスとプレースホルダーをシンクロさせる間隔(ms)です。
		//
		int32_t sync_elapse = 500;
	} property;
}
