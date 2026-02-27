#pragma once

namespace apn::appride::view
{
	//
	// このクラスはサブプロセス用のコンフィグダイアログです。
	//
	struct extrapp_dialog_t : my::layout::Window<config_dialog_base_t>
	{
		//
		// WM_INITDIALOGを処理します。
		//
		virtual void on_init_dialog() override
		{
			init_combobox(idc_exit_mode, L"ウィンドウを閉じる", L"プロセスを強制終了する");

			using namespace my::layout;

			auto margin_value = 2;
			auto margin = RECT { margin_value, margin_value, margin_value, margin_value };
			auto base_size = get_base_size();
			auto row_size = base_size + margin_value * 2;
			auto fixed_size = base_size * 3 + margin_value * 2;
			auto row = std::make_shared<RelativePos>(row_size);
			auto half = std::make_shared<AbsolutePos>(1, 2);
			auto full = std::make_shared<AbsolutePos>(2, 2);
			auto s_control = std::make_shared<RelativePos>(fixed_size * 1);
			auto m_control = std::make_shared<RelativePos>(fixed_size * 2);
			auto l_control = std::make_shared<RelativePos>(fixed_size * 3);

			// 先頭行のレイアウトです。
			{
				auto node = root->add_pane(c_axis.c_vert, c_align.c_top, row);
				node->add_pane(c_axis.c_horz, c_align.c_left, s_control, margin, ctrl(idc_flag_disable));
				node->add_pane(c_axis.c_horz, c_align.c_left, full, margin, ctrl(idc_finder_stc));
			}

			// クラス名の行のレイアウトです。
			{
				auto node = root->add_pane(c_axis.c_vert, c_align.c_top, row);
				node->add_pane(c_axis.c_horz, c_align.c_left, s_control, margin, ctrl(idc_class_name_stc));
				node->add_pane(c_axis.c_horz, c_align.c_left, full, margin, ctrl(idc_class_name));
			}

			// ウィンドウ名の行のレイアウトです。
			{
				auto node = root->add_pane(c_axis.c_vert, c_align.c_top, row);
				node->add_pane(c_axis.c_horz, c_align.c_left, s_control, margin, ctrl(idc_window_name_stc));
				node->add_pane(c_axis.c_horz, c_align.c_left, full, margin, ctrl(idc_window_name));
			}

			// パスの行のレイアウトです。
			{
				auto full = std::make_shared<RelativePos>(1, 1, -row_size);

				auto node = root->add_pane(c_axis.c_vert, c_align.c_top, row);
				node->add_pane(c_axis.c_horz, c_align.c_left, s_control, margin, ctrl(idc_path_stc));
				node->add_pane(c_axis.c_horz, c_align.c_left, full, margin, ctrl(idc_path));
				node->add_pane(c_axis.c_horz, c_align.c_left, row, margin, ctrl(idc_path_ref));
			}

			// 終了処理の行のレイアウトです。
			{
				auto node = root->add_pane(c_axis.c_vert, c_align.c_top, row);
				node->add_pane(c_axis.c_horz, c_align.c_left, s_control, margin, ctrl(idc_exit_mode_stc));
				node->add_pane(c_axis.c_horz, c_align.c_left, m_control, margin, ctrl(idc_exit_mode));
			}

			// デフォルトボタンの行のレイアウトです。
			{
				auto node = root->add_pane(c_axis.c_vert, c_align.c_top, row);
				node->add_pane(c_axis.c_horz, c_align.c_right, m_control, margin, ctrl(IDOK));
				node->add_pane(c_axis.c_horz, c_align.c_right, m_control, margin, ctrl(IDCANCEL));
			}
		}

		//
		// WM_COMMANDを処理します。
		//
		virtual void on_command(UINT code, UINT control_id, HWND control) override
		{
			switch (control_id)
			{
			case idc_path_ref:
				{
					// exeファイルのパスを取得します。
					auto file_name = get_open_file_name(
						*this,
						L"exeファイルを選択",
						L"exeファイル (*.exe)\0*.exe\0"
						L"すべてのファイル (*.*)\0*.*\0",
						get_text(idc_path).c_str());
					if (file_name.empty()) break;

					// 取得したパスをセットします。
					set_text(idc_path, file_name);

					break;
				}
			}
		}

		//
		// 指定されたプロセスイメージのパスを返します。
		//
		inline static std::filesystem::path get_process_image_path(DWORD pid)
		{
			my::handle::unique_ptr<> process(::OpenProcess(
				PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
			if (!process) return {};

			auto buffer_size = (DWORD)MAX_PATH;
			std::wstring buffer(buffer_size, L'\0');
			::QueryFullProcessImageNameW(process.get(), 0, buffer.data(), &buffer_size);
			return buffer.c_str();
		}

		//
		// マウスカーソル座標にあるサブプロセスをピックアップします。
		//
		void pickup_subprocess()
		{
			// マウスカーソル座標を取得します。
			auto point = my::get_cursor_pos();

			// マウスカーソル座標にあるウィンドウを取得します。
			auto hwnd = ::WindowFromPoint(point);
			if (!hwnd) return;

			// 最上位のウィンドウを取得します。
			hwnd = [](HWND hwnd)
			{
				// カレントスレッドのIDを取得します。
				auto tid = ::GetCurrentThreadId();

				// 上位ウィンドウを取得し続けるループです。
				while (1)
				{
					// オーナーウィンドウが有効の場合は
					if (auto owner = ::GetWindow(hwnd, GW_OWNER);
						owner && ::GetWindowThreadProcessId(owner, nullptr) != tid)
					{
						// オーナーウィンドウを対象にします。
						hwnd = owner;

						continue;
					}

					// 親ウィンドウが有効の場合は
					if (auto parent = ::GetParent(hwnd);
						parent && ::GetWindowThreadProcessId(parent, nullptr) != tid)
					{
						// 親ウィンドウを対象にします。
						hwnd = parent;

						continue;
					}

					// 上位ウィンドウを取得できなかった場合はループを終了します。
					break;
				}

				// 最上位のウィンドウを返します。
				return hwnd;
			}
			(hwnd);

			// クラス名を取得します。
			auto class_name = my::get_class_name(hwnd);

			// ウィンドウ名を取得します。
			auto window_name = my::get_window_text(hwnd);

			// ウィンドウが所属するプロセスIDを取得します。
			auto pid = DWORD {};
			auto tid = ::GetWindowThreadProcessId(hwnd, &pid);

			// 現在のプロセス内のウィンドウの場合は除外します。
			if (pid == ::GetCurrentProcessId()) return;

			// プロセスイメージのパスを取得します。
			auto path = get_process_image_path(pid);

			// 取得できた変数をコントロールにセットします。
			set_text(idc_class_name, class_name);
			set_text(idc_window_name, window_name);
			set_text(idc_path, path);

			return;
		}

		//
		// サブプロセス検索処理です。
		//
		LRESULT on_finder(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
		{
			// aviutl2ウィンドウを非表示にします。
			::ShowWindow(hive.aviutl2_window, SW_HIDE);

			// スコープ終了時にaviutl2ウィンドウを表示します。
			my::scope_exit scope_exit([]() {
				::ShowWindow(hive.aviutl2_window, SW_SHOW);
			});

			// マウスキャプチャを開始します。
			::SetCapture(hwnd);

			// メッセージループを開始します。
			auto msg = MSG {};
			while (::GetMessage(&msg, nullptr, 0, 0) > 0)
			{
				// マウスキャプチャが終了している場合は
				// メッセージループを終了します。
				if (::GetCapture() != hwnd)
					return 0;

				// メッセージウィンドウがこのウィンドウの場合は
				if (msg.hwnd == hwnd)
				{
					switch (msg.message)
					{
					// ユーザーがマウスドラッグした場合は
					case WM_MOUSEMOVE:
						{
							// マウスカーソル座標にあるサブプロセスをピックアップします。
							pickup_subprocess();

							break;
						}
					// ユーザーがマウスドラッグを終了した場合は
					case WM_LBUTTONUP:
						{
							// マウスキャプチャを終了します。
							::ReleaseCapture();

							// マウスカーソル座標にあるサブプロセスをピックアップします。
							pickup_subprocess();

							// メッセージループを終了します。
							return 0;
						}
					}
				}

				// 残りのメッセージをディスパッチします。
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}

			return __super::on_wnd_proc(hwnd, message, w_param, l_param);
		}

		//
		// ウィンドウプロシージャです。
		//
		virtual LRESULT on_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) override
		{
			switch (message)
			{
			case WM_LBUTTONDOWN:
				{
					MY_TRACE_FUNC("{/}, {/hex}, {/hex}", my::message_to_string(message), w_param, l_param);

					// ファインダーコントロールの矩形を取得します。
					auto rc = my::get_window_rect(ctrl(idc_finder_stc));
					my::map_window_points(nullptr, hwnd, &rc);

					// マウスカーソル座標を取得します。
					auto point = my::lp_to_pt(l_param);

					// ファインダーコントロールの矩形内の場合は
					if (::PtInRect(&rc, point))
					{
						// サブプロセスのピックアップを開始します。
						return on_finder(hwnd, message, w_param, l_param);
					}

					break;
				}
			}

			return __super::on_wnd_proc(hwnd, message, w_param, l_param);
		}
	};
}
