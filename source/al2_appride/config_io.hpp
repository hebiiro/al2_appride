#pragma once

namespace apn::appride
{
	//
	// このクラスはコンフィグの入出力を担当します。
	//
	inline struct config_io_t : config_io_base_t
	{
		//
		// 更新処理を実行します。
		//
		virtual BOOL update() override
		{
			MY_TRACE_FUNC("");

			// ノードを変更したので、リストボックスを更新します。
			view::main_window.update_listbox();

			return __super::update();
		}

		//
		// ノードからコンフィグを読み込みます。
		//
		virtual BOOL read_node(n_json& root) override
		{
			MY_TRACE_FUNC("");

			controller::io.read(root);

			return __super::read_node(root);
		}

		//
		// ノードにコンフィグを書き込みます。
		//
		virtual BOOL write_node(n_json& root) override
		{
			MY_TRACE_FUNC("");

			controller::io.write(root);

			return __super::write_node(root);
		}
	} config_io;
}
