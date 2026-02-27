#pragma once

namespace apn::appride::view
{
	//
	// このクラスは外部アプリのプレースホルダーです。
	// このウィンドウがサブプロセスウィンドウを内包します。
	//
	struct placeholder_t : my::Window
	{
		//
		// このクラスはタイマーのIDです。
		//
		inline static constexpr struct timer_id_t {
			inline static constexpr UINT_PTR c_init = 1000;
			inline static constexpr UINT_PTR c_sync = 1001;
		} c_timer_id;

		//
		// このクラスはサブサブプロセスです。
		//
		struct extrapp_t {
			//
			// サブプロセスのノードです。
			//
			const std::shared_ptr<model::property_t::node_t> node;

			//
			// サブプロセスのプロセスIDです。
			//
			DWORD pid = 0;

			//
			// サブプロセスのハンドルです。
			//
			my::handle::unique_ptr<> process;

			//
			// サブプロセスのウィンドウです。
			//
			HWND hwnd = nullptr;

			//
			// サブプロセス用のジョブハンドルです。
			//
			my::handle::unique_ptr<> job;
		} extrapp;

		//
		// このクラスはプレースホルダーの状態です。
		//
		struct state_t {
			//
			// プレースホルダーの表示状態です。
			//
			BOOL flag_is_visible = FALSE;

			//
			// プレースホルダーの表示位置です。
			//
			RECT rc = {};
		} state;

		//
		// コンストラクタです。
		//
		placeholder_t(const std::shared_ptr<model::property_t::node_t>& node)
			: extrapp(node)
		{
			MY_TRACE_FUNC("");

			constexpr auto c_class_name = L"appride.placeholder";

			// ウィンドウクラスを登録します。
			WNDCLASS wc = {};
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wc.lpfnWndProc = ::DefWindowProc;
			wc.hInstance = hive.instance;
			wc.lpszClassName = c_class_name;
			::RegisterClass(&wc);

			// ウィンドウを作成します。
			if (!__super::create(
				WS_EX_NOPARENTNOTIFY,
				wc.lpszClassName,
				nullptr,
				WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				0, 0, 0, 0,
				nullptr, nullptr, hive.instance, nullptr))
			{
				MY_TRACE("プレースホルダーの作成に失敗しました\n");

				return;
			}

			// サブプロセスのパスが指定されている場合は
			if (!node->path.empty())
			{
				// サブプロセスのフォルダを取得します。
				auto dir = node->path.parent_path();

				// サブプロセスを起動します。
				SHELLEXECUTEINFO sei = { sizeof(sei) };
				sei.lpFile = node->path.c_str();
				sei.lpDirectory = dir.c_str();
				sei.nShow = SW_HIDE;
				sei.fMask = SEE_MASK_NOCLOSEPROCESS;
				if (::ShellExecuteEx(&sei) && sei.hProcess)
				{
					// サブプロセス情報を取得します。
					extrapp.pid = ::GetProcessId(sei.hProcess);
					extrapp.process.reset(sei.hProcess);

					// このタイミングではまだサブプロセスのウィンドウが
					// 作成されていないかもしれないので、
					// タイマーで遅らせてから初期化処理を行います。
					::SetTimer(*this, c_timer_id.c_init, 1000, nullptr);

					// サブプロセスを自動終了する場合は
					if (extrapp.node->exit_mode == model::property.c_exit_mode.c_process)
					{
						// メインプロセスとサブプロセスを関連付けます。
						extrapp.job.reset(::CreateJobObject(nullptr, nullptr));
						auto job_info = JOBOBJECT_EXTENDED_LIMIT_INFORMATION {
							.BasicLimitInformation = { .LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE },
						};
						::SetInformationJobObject(extrapp.job.get(), JobObjectExtendedLimitInformation, &job_info, sizeof(job_info));
						::AssignProcessToJobObject(extrapp.job.get(), extrapp.process.get());
					}
				}
			}
		}

		//
		// デストラクタです。
		//
		virtual ~placeholder_t() override
		{
			MY_TRACE_FUNC("");

			// サブプロセスウィンドウが有効の場合は
			if (::IsWindow(extrapp.hwnd))
			{
				// サブプロセスウィンドウのオーナーを変更します。
				::SetWindowLongPtrW(extrapp.hwnd,
					GWLP_HWNDPARENT, (LONG_PTR)(HWND)nullptr);

				// サブプロセスウィンドウの親ウィンドウを変更します。
				::SetParent(extrapp.hwnd, nullptr);

				// サブプロセスウィンドウを表示します。
				::ShowWindow(extrapp.hwnd, SW_RESTORE);

				// サブプロセスウィンドウを終了する場合は
				if (extrapp.node->exit_mode == model::property.c_exit_mode.c_window)
				{
					// WM_CLOSEをポストします。
					::PostMessage(extrapp.hwnd, WM_CLOSE, 0, 0);
				}
			}
		}

		// サブプロセスウィンドウをクライアント領域全体まで広げます。
		// ウィンドウ位置の変更は非同期で処理されるので、デッドロックは起きないはずです。
		void sync()
		{
			MY_TRACE_FUNC("");

			// サブプロセスウィンドウの表示状態を切り替える必要があるかどうかのフラグです。
			auto need_show = FALSE;

			// サブプロセスウィンドウをリサイズする必要があるかどうかのフラグです。
			auto need_resize = FALSE;

			// プレースホルダーの表示状態を取得します。
			auto is_placeholder_visible = ::IsWindowVisible(*this);

			// プレースホルダーの表示状態が前回と異なる場合は
			if (state.flag_is_visible != is_placeholder_visible)
			{
				// サブプロセスウィンドウの表示状態を切り替えるようにします。
				need_show = TRUE;

				// プレースホルダーの表示状態を更新します。
				need_resize = state.flag_is_visible = is_placeholder_visible;
			}

			// プレースホルダーが表示されている場合は
			if (state.flag_is_visible)
			{
				// プレースホルダーの位置を取得します。
				auto rc = my::get_client_rect(*this);
				my::map_window_points(*this, nullptr, &rc);

				// サブプロセスウィンドウをリサイズする必要がある場合、
				// またはプレースホルダーの位置が前回と異なる場合は
				if (need_resize || !::EqualRect(&state.rc, &rc))
				{
					// プレースホルダーの位置を更新します。
					state.rc = rc;

					// サブプロセスウィンドウのオーナーを変更します。
					::SetWindowLongPtrW(extrapp.hwnd,
						GWLP_HWNDPARENT, (LONG_PTR)(HWND)*this);

					// サブプロセスウィンドウの位置も更新します。
					my::set_window_pos(extrapp.hwnd, nullptr,
						&rc, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
				}

				// サブプロセスウィンドウも表示します。
				if (need_show) ::ShowWindowAsync(extrapp.hwnd, SW_RESTORE);
			}
			// プレースホルダーが表示されていない場合は
			else
			{
				// サブプロセスウィンドウも非表示にします。
				if (need_show) ::ShowWindowAsync(extrapp.hwnd, SW_HIDE);
			}
		}

		//
		// この仮想関数はサブプロセスウィンドウを初期化するために呼ばれます。
		//
		virtual BOOL on_init()
		{
			MY_TRACE_FUNC("");

			// サブプロセスウィンドウを検索します。
			if (!on_find_window()) return FALSE;

			MY_TRACE("サブプロセスウィンドウが見つかりました => {/hex}\n", extrapp.hwnd);

			// サブプロセスウィンドウを非表示にします。
			::ShowWindowAsync(extrapp.hwnd, SW_HIDE);

			// プレースホルダーの状態をリセットします。
			state = {};

			// 定期的にsync()を実行します。
			::SetTimer(*this, c_timer_id.c_sync, model::property.sync_elapse, nullptr);

			return TRUE;
		}

		//
		// この仮想関数はサブプロセスウィンドウを検索するために呼ばれます。
		//
		virtual BOOL on_find_window()
		{
			MY_TRACE_FUNC("");

			// トップレベルウィンドウを列挙します。
			::EnumWindows([](HWND hwnd, LPARAM l_param) -> BOOL
			{
				MY_TRACE_HWND(hwnd);

				// thisポインタを取得します。
				auto p = (placeholder_t*)l_param;

				// hwndが目的のウィンドウかどうかチェックします。
				// hwndが目的のウィンドウではない場合はTRUEを返して次のウィンドウに進めます。

				if (p->extrapp.pid)
				{
					// プロセスIDをチェックします。
					auto pid = DWORD {};
					auto tid = ::GetWindowThreadProcessId(hwnd, &pid);
					if (p->extrapp.pid != pid) return TRUE;
				}

				if (p->extrapp.node->window_name.length())
				{
					// ウィンドウ名をチェックします。
					auto window_name = my::get_window_text(hwnd);
					if (window_name.find(p->extrapp.node->window_name) == window_name.npos) return TRUE;
				}

				if (p->extrapp.node->class_name.length())
				{
					// クラス名をチェックします。
					auto class_name = my::get_class_name(hwnd);
					if (class_name.find(p->extrapp.node->class_name) == class_name.npos) return TRUE;
				}

				// サブプロセスウィンドウをセットします。
				p->extrapp.hwnd = hwnd;

				// 目的のウィンドウが見つかったので、FALSEを返して列挙を終了します。
				return FALSE;
			},
			(LPARAM)this);

			return !!extrapp.hwnd;
		}

		//
		// ウィンドウプロシージャです。
		//
		virtual LRESULT on_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) override
		{
			MY_TRACE_FUNC("{/}, {/hex}, {/hex}", my::message_to_string(message), w_param, l_param);

			switch (message)
			{
			case WM_TIMER:
				{
					switch (w_param)
					{
					case c_timer_id.c_init:
						{
							// 初期化を実行します。
							// 成功するまでタイマーで繰り返し実行されます。
							if (on_init())
							{
								// 初期化が完了したのでタイマーを終了します。
								::KillTimer(hwnd, w_param);
							}

							break;
						}
					case c_timer_id.c_sync:
						{
							// サブプロセスウィンドウの位置をプレースホルダーとシンクロさせます。
							sync();

							break;
						}
					}

					break;
				}
			}

			return __super::on_wnd_proc(hwnd, message, w_param, l_param);
		}
	};
}
