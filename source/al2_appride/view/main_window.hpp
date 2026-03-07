#pragma once

namespace apn::appride::view
{
	//
	// このクラスはビューのメインウィンドウです。
	//
	inline struct main_window_t : my::layout::Window<config_dialog_base_t>
	{
		//
		// コンフィグでコントロールを更新します。
		//
		virtual BOOL on_to_ui() override
		{
			MY_TRACE_FUNC("");

			return TRUE;
		}

		//
		// コントロールでコンフィグを更新します。
		//
		virtual BOOL on_from_ui() override
		{
			MY_TRACE_FUNC("");

			return TRUE;
		}

		//
		// リストボックスを更新します。
		//
		BOOL update_listbox()
		{
			// リストボックスを取得します。
			auto listbox = ctrl(idc_extrapps);

			// 一旦リストボックスの再描画を停止します。
			SetWindowRedraw(listbox, FALSE);

			// 選択項目を取得しておきます。
			auto cur_sel = ListBox_GetCurSel(listbox);

			// 一旦リストボックスの項目をすべて消去します。
			ListBox_ResetContent(listbox);

			// ノードを走査します。
			for (const auto& node : model::property.nodes)
			{
				// リストボックスにノードの表示名を追加します。
				ListBox_AddString(listbox, node->display_name.c_str());
			}

			// 選択項目を復元します。
			ListBox_SetCurSel(listbox, std::max(cur_sel, 0));

			// リストボックスの再描画を再開します。
			SetWindowRedraw(listbox, TRUE);

			return TRUE;
		}

		//
		// 外部アプリを追加します。
		//
		BOOL on_insert()
		{
			// 外部アプリ設定ダイアログです。
			extrapp_dialog_t dialog;

			// ダイアログを初期化します。
			dialog.init(idd_extrapp, *this);
			dialog.set_text(idc_display_name, my::format(
				L"{/} {/}", L"外部アプリ", model::property.nodes.size() + 1));
			dialog.set_combobox_index(idc_exit_mode, model::property.c_exit_mode.c_window);

			// ダイアログを表示します。
			if (IDOK == dialog.do_modal())
			{
				// リストボックスを取得します。
				auto listbox = ctrl(idc_extrapps);

				// ノードを作成してコレクションに追加します。
				auto node = std::make_shared<model::property_t::node_t>(
					dialog.get_check(idc_flag_disable),
					dialog.get_text(idc_display_name),
					dialog.get_text(idc_class_name),
					dialog.get_text(idc_window_name),
					dialog.get_text(idc_path),
					dialog.get_combobox_index(idc_exit_mode));
				model::property.nodes.emplace_back(node);

				// 項目を追加します。
				auto index = ListBox_AddString(listbox, node->display_name.c_str());

				// 追加項目を選択します。
				ListBox_SetCurSel(listbox, index);
			}

			// ダイアログを終了します。
			dialog.exit();

			return TRUE;
		}

		//
		// 外部アプリを編集します。
		//
		BOOL on_edit()
		{
			// リストボックスを取得します。
			auto listbox = ctrl(idc_extrapps);

			// 選択項目を取得します。
			auto cur_sel = (size_t)ListBox_GetCurSel(listbox);

			// 選択項目が無効の場合は
			if (cur_sel >= model::property.nodes.size())
			{
				hive.message_box(tr(L"選択項目が無効です"));

				return FALSE;
			}

			// 選択項目のノードを取得します。
			const auto& node = model::property.nodes[cur_sel];

			// 外部アプリ設定ダイアログです。
			extrapp_dialog_t dialog;

			// ダイアログを初期化します。
			dialog.init(idd_extrapp, *this);
			dialog.set_check(idc_flag_disable, node->flag_disable);
			dialog.set_text(idc_display_name, node->display_name);
			dialog.set_text(idc_class_name, node->class_name);
			dialog.set_text(idc_window_name, node->window_name);
			dialog.set_text(idc_path, node->path);
			dialog.set_combobox_index(idc_exit_mode, node->exit_mode);

			// ダイアログを表示します。
			if (IDOK == dialog.do_modal())
			{
//				// ノードに変更を適用します。
				*node = {
					dialog.get_check(idc_flag_disable),
					dialog.get_text(idc_display_name),
					dialog.get_text(idc_class_name),
					dialog.get_text(idc_window_name),
					dialog.get_text(idc_path),
					dialog.get_combobox_index(idc_exit_mode),
				};

				// ノードを変更したので、リストボックスを更新します。
				update_listbox();
			}

			// ダイアログを終了します。
			dialog.exit();

			return TRUE;
		}

		//
		// 外部アプリを削除します。
		//
		BOOL on_erase()
		{
			// リストボックスを取得します。
			auto listbox = ctrl(idc_extrapps);

			// 選択項目を取得します。
			auto cur_sel = (size_t)ListBox_GetCurSel(listbox);

			// 選択項目が無効の場合は
			if (cur_sel >= model::property.nodes.size())
			{
				hive.message_box(tr(L"選択項目が無効です"));

				return FALSE;
			}

			// 選択項目のノードを取得します。
			const auto& node = model::property.nodes[cur_sel];

			// メッセージボックスに表示するテキストを作成します。
			auto message_text = my::format(L"{/}\n\n{/}",
				node->display_name, tr(L"この項目を削除していいですか？"));

			// 本当に削除するかどうかをユーザーに確認します。
			if (IDOK != hive.message_box(message_text, nullptr, MB_OKCANCEL | MB_ICONWARNING))
			{
				// ユーザーが拒否した場合は何もしません。
				return FALSE;
			}

			// コレクションから選択項目を削除します。
			model::property.nodes.erase(model::property.nodes.begin() + cur_sel);

			// リストボックスから選択項目を削除します。
			ListBox_DeleteString(listbox, cur_sel);

			return TRUE;
		}
#if 1
		//
		// WM_INITDIALOGを処理します。
		//
		virtual void on_init_dialog() override
		{
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

			// 外部アプリリストのレイアウトです。
			{
				auto row = std::make_shared<RelativePos>(1, 1, -row_size);

				auto node = root->add_pane(c_axis.c_vert, c_align.c_top, row);
				node->add_pane(c_axis.c_horz, c_align.c_left, full, margin, ctrl(idc_extrapps));
			}

			// ボタンの行のレイアウトです。
			{
				auto node = root->add_pane(c_axis.c_vert, c_align.c_top, row);
				node->add_pane(c_axis.c_horz, c_align.c_left, s_control, margin, ctrl(idc_insert));
				node->add_pane(c_axis.c_horz, c_align.c_left, s_control, margin, ctrl(idc_edit));
				node->add_pane(c_axis.c_horz, c_align.c_left, s_control, margin, ctrl(idc_erase));
			}
		}
#endif
		//
		// WM_COMMANDを処理します。
		//
		virtual void on_command(UINT code, UINT control_id, HWND control) override
		{
			switch (control_id)
			{
			case idc_insert: return (void)on_insert();
			case idc_edit: return (void)on_edit();
			case idc_erase: return (void)on_erase();
			}
		}
	} main_window;
}
